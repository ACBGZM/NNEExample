// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NNE.h"
#include "NNEModelData.h"
#include "NNERuntimeCPU.h"
#include "Engine/Texture2D.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NNEActor.generated.h"

//class UTexture2D;
//class UNNEModelData;
//
//namespace UE::NNE
//{
//    class IModelCPU;
//    class IModelInstanceCPU;
//    struct FTensorBindingCPU;
//}

struct FNNEModelHelper
{
    TSharedPtr<UE::NNE::IModelInstanceCPU> m_model_instance;
    TWeakObjectPtr<UTexture2D> m_input_texture;
    TArray<float> m_output_data;

    TArray<UE::NNE::FTensorBindingCPU> m_input_bindings;
    TArray<UE::NNE::FTensorBindingCPU> m_output_bindings;

    bool m_is_running{ false };
};


UCLASS()
class NNEDEMO_API ANNEActor : public AActor
{
    GENERATED_BODY()
    
public:
    ANNEActor();

    UFUNCTION(BlueprintCallable, Category = "NNEActor")
    void RunNNE();

    UFUNCTION(BlueprintImplementableEvent, Category = "NNEActor")
    void OnStartRunNNE(const UTexture2D* input_texture);

    UFUNCTION(BlueprintImplementableEvent, Category = "NNEActor")
    void OnStopRunNNE(const TArray<float>& output_data);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type reason) override;

private:
    void SetUpNNE();
    void DestroyNNE();


public:
    UPROPERTY(EditAnywhere, meta = (DisplayName = "NNE Model Data"))
    TObjectPtr<UNNEModelData> m_nne_model_data;

    UPROPERTY(EditAnywhere)
    TArray<TObjectPtr<UTexture2D>> m_input_textures;

private:
    TSharedPtr<FNNEModelHelper> m_model_helper{ MakeShared<FNNEModelHelper>() };

    TSharedPtr<UE::NNE::IModelCPU> m_model;
    TSharedPtr<UE::NNE::IModelInstanceCPU> m_model_instance;
};
