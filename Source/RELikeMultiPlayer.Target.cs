// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RELikeMultiPlayerTarget : TargetRules
{
	public RELikeMultiPlayerTarget(TargetInfo Target) : base(Target)
	{
        // Update to the latest build settings
        DefaultBuildSettings = BuildSettingsVersion.V5;

        // Set the include order to the latest version
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        // Add the module names
        ExtraModuleNames.Add("RELikeMultiPlayer");

        // Configure for unique build environment if needed
        // This is necessary if you're making changes that conflict with the shared build environment
        //BuildEnvironment = TargetBuildEnvironment.Unique;

        // Optionally, override build environment settings if necessary
        // Uncomment the following line if you need to enforce specific settings
        // bOverrideBuildEnvironment = true;
	}
}
