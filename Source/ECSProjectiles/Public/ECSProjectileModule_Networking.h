// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECSModule.h"
#include "ECSProjectileModule_Networking.generated.h"

/**
 * 
 */
UCLASS()
class ECSPROJECTILES_API UECSProjectileModule_Networking : public UECSModule
{
	GENERATED_BODY()
public:

	virtual void InitializeComponents(TSharedPtr<flecs::world> World);

	virtual void InitializeSystems(TSharedPtr<flecs::world> World);

	virtual void FinishInitialize(TSharedPtr<flecs::world> World);
	
};
