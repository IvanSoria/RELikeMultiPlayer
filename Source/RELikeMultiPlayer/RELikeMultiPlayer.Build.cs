// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RELikeMultiPlayer : ModuleRules
{
	public RELikeMultiPlayer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "OnlineSubsystemSteam", "OnlineSubsystem", "UMG", "Slate", "SlateCore" });

		PublicIncludePaths.AddRange(new string[] { "RELikeMultiPlayer/Core", "RELikeMultiPlayer/Player", "RELikeMultiPlayer/Components", "RELikeMultiPlayer/Items","RELikeMultiPlayer/AI", "RELikeMultiPlayer/UI"});
	}
}
