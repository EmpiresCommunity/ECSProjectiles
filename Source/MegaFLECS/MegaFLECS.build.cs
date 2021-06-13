// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MegaFLECS : ModuleRules
{
    public MegaFLECS(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Public/MegaFLECS.h";
		CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
             "CoreUObject",
             "Engine",
             "DeveloperSettings",
        });

        PublicIncludePaths.AddRange(new string[] { "FlecsLibrary/Public" });
        PrivateIncludePaths.AddRange(new string[] { "FlecsLibrary/Private" });

        AppendStringToPublicDefinition("flecs_EXPORTS", "0");     
    }

}