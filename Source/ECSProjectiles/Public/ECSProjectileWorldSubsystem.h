// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ECSProjectileWorldSubsystem.generated.h"
class AECSProjectilesNiagaraManager;
/**
 * 
 */
UCLASS()
class ECSPROJECTILES_API UECSProjectileWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite)
	AECSProjectilesNiagaraManager* ProjectilesNiagaraManagerActor;
	
};
