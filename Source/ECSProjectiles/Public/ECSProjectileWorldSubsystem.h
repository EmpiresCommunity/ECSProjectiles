// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MegaFLECSTypes.h"
#include "ECSProjectileWorldSubsystem.generated.h"


/**
 * 
 */
UCLASS()
class ECSPROJECTILES_API UECSProjectileWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	AActor* NiagaraManager;

	UPROPERTY(BlueprintReadWrite)
	FECSEntityHandle DefaultNiagaraProjectileManager;
	UPROPERTY(BlueprintReadWrite)
	FECSEntityHandle DefaultNiagaraHitsManager;
	
};
