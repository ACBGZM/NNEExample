// Copyright Epic Games, Inc. All Rights Reserved.

#include "NNEDemoGameMode.h"
#include "NNEDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANNEDemoGameMode::ANNEDemoGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
