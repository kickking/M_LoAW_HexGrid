// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class M_LoAW_Terrain : ModuleRules
{
	public M_LoAW_Terrain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "FastNoiseGenerator", "FastNoise" });

		PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent", "LoAW_GridDataCreator" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
