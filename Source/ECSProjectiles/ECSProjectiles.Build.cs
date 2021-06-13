// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ECSProjectiles : ModuleRules
{
	public ECSProjectiles(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Public/ECSPRojectiles.h";

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",		
				"DeveloperSettings",				
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"MegaFLECS",
			});

		//If we are using Niagara, enable it here
		if(true)
        {
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"Niagara",
			});
			PublicDefinitions.Add("ECSPROJECTILES_NIAGARA=1");
		}
        else
        {
			PublicDefinitions.Add("ECSPROJECTILES_NIAGARA=0");
		}
	}
}
