// Fill out your copyright notice in the Description page of Project Settings.

#include "NNEActor.h"

ANNEActor::ANNEActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

void ANNEActor::BeginPlay()
{
    Super::BeginPlay();
    SetUpNNE();
}

void ANNEActor::EndPlay(const EEndPlayReason::Type reason)
{
    Super::EndPlay(reason);
    DestroyNNE();
}

void ANNEActor::SetUpNNE()
{
    FString use_runtime_str = FString("NNERuntimeORTCpu");
    TWeakInterfacePtr<INNERuntimeCPU> nne_runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(use_runtime_str);
    if (!nne_runtime.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("cannot find enabled runtime"));
        return;
    }

    m_model = nne_runtime->CreateModelCPU(m_nne_model_data);
    if (!m_model.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("failed to create model"));
        return;
    }

    m_model_instance = m_model->CreateModelInstanceCPU();
    if (!m_model_instance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("failed to create model instance"));
        return;
    }
}

void ANNEActor::DestroyNNE()
{
    if (m_model_helper.IsValid())
    {
        m_model_helper.Reset();
    }

    if (m_model_instance.IsValid())
    {
        m_model_instance.Reset();
    }

    if (m_model.IsValid())
    {
        m_model.Reset();
    }
}

void ANNEActor::RunNNE()
{
    if (!m_model_instance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("invalid model instance"));
        return;
    }

    if (m_input_textures.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("empty input textures"));
        return;
    }

    if (m_model_helper->m_is_running)
    {
        UE_LOG(LogTemp, Error, TEXT("model is already running"));
        return;
    }

    m_model_helper->m_input_texture = m_input_textures[FMath::RandRange(0, m_input_textures.Num() - 1)];
    if (!m_model_helper->m_input_texture.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("invalid input texture"));
        return;
    }

    if (m_model_helper->m_input_texture->GetPixelFormat() != PF_R32_FLOAT)
    {
        UE_LOG(LogTemp, Error, TEXT("unsupported input texture format"));
        return;
    }

    m_model_helper->m_model_instance = m_model_instance;
    m_model_helper->m_is_running = true;

    OnStartRunNNE(m_model_helper->m_input_texture.Get());

    TWeakObjectPtr<ANNEActor> weak_this = this;
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [weak_this, model_helper = m_model_helper]()
        {
            TConstArrayView<UE::NNE::FTensorDesc> InputTensorDescs = model_helper->m_model_instance->GetInputTensorDescs();
            checkf(InputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single input tensor"));
            UE::NNE::FSymbolicTensorShape SymbolicInputTensorShape = InputTensorDescs[0].GetShape();
            checkf(SymbolicInputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable input tensor dimensions"));
            TArray<UE::NNE::FTensorShape> InputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicInputTensorShape) };

            model_helper->m_model_instance->SetInputTensorShapes(InputTensorShapes);

            TConstArrayView<UE::NNE::FTensorDesc> OutputTensorDescs = model_helper->m_model_instance->GetOutputTensorDescs();
            checkf(OutputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single output tensor"));
            UE::NNE::FSymbolicTensorShape SymbolicOutputTensorShape = OutputTensorDescs[0].GetShape();
            checkf(SymbolicOutputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable output tensor dimensions"));

            model_helper->m_input_bindings.SetNumZeroed(1);
            model_helper->m_output_bindings.SetNumZeroed(1);

            // Fill the input tensor with the input texture data
            FTexture2DMipMap& Mip = model_helper->m_input_texture->GetPlatformData()->Mips[0];
            model_helper->m_input_bindings[0].Data = Mip.BulkData.Lock(LOCK_READ_ONLY);
            model_helper->m_input_bindings[0].SizeInBytes = Mip.BulkData.GetBulkDataSize();

            // Allocate memory for the output tensor
            model_helper->m_output_data.SetNumUninitialized(10);
            model_helper->m_output_bindings[0].Data = model_helper->m_output_data.GetData();
            model_helper->m_output_bindings[0].SizeInBytes = model_helper->m_output_data.Num() * sizeof(float);

            // Run the model
            UE::NNE::IModelInstanceCPU::ERunSyncStatus RunSyncStatus = model_helper->m_model_instance->RunSync(model_helper->m_input_bindings, model_helper->m_output_bindings);
            if (RunSyncStatus != UE::NNE::IModelInstanceCPU::ERunSyncStatus::Ok)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to run the model"));
            }

            Mip.BulkData.Unlock();

            AsyncTask(ENamedThreads::GameThread, [weak_this, model_helper]()
                {
                    model_helper->m_is_running = false;

                    if (weak_this.IsValid())
                    {
                        weak_this->OnStopRunNNE(model_helper->m_output_data);
                    }
                });
        });
}
