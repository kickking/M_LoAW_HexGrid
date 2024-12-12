// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class M_LoAW_HexGridEditorTarget : TargetRules
{
	public M_LoAW_HexGridEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("M_LoAW_HexGrid");
        ExtraModuleNames.Add("M_LoAW_Terrain");
    }
}
