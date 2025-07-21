// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RELikeMultiPlayerEditorTarget : TargetRules
{
	public RELikeMultiPlayerEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.Add("RELikeMultiPlayer");

        //Additional configuration if necessary
        //BuildEnvironment = TargetBuildEnvironment.Unique;
        //bOverrideBuildEnvironment = true; // Uncomment if needed
	}
}
