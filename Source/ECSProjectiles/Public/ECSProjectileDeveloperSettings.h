// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ECSProjectilesNiagaraManager.h"
#include "Engine/DeveloperSettings.h"
#include "ECSProjectileDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig)
class ECSPROJECTILES_API UECSProjectileDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, config)
	TEnumAsByte<ECollisionChannel> ProjectileCollision;

	UPROPERTY(EditDefaultsOnly, config)
	TSubclassOf<class AECSProjectilesNiagaraManager> NiagaraManagerActor;
};
