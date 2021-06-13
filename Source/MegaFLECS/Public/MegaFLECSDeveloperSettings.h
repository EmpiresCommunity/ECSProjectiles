// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MegaFLECSDeveloperSettings.generated.h"

class UECSModule;

/**
 * 
 */
UCLASS(config = Game, defaultconfig)
class MEGAFLECS_API UMegaFLECSDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UMegaFLECSDeveloperSettings(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditDefaultsOnly, config)
	TArray<TSubclassOf<UECSModule>> ECSModules;

};
