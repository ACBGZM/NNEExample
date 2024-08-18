// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NNEDemo : ModuleRules
{
	public NNEDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "NNE" });
	}
}
