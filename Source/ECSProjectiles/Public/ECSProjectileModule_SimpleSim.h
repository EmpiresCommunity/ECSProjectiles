// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECSModule.h"
#include "ECSProjectileModule_SimpleSim.generated.h"

struct FECSBulletTransform
{
	FTransform CurrentTransform;
	FTransform PreviousTransform;
};

struct FECSBulletVelocity
{
	FVector Velocity;
};

struct FECSActorEntity
{
	TSoftObjectPtr<AActor> Actor;
};

struct FECSWorldReference
{
	TSoftObjectPtr<UWorld> World;
};

/**
 * 
 */
UCLASS()
class ECSPROJECTILES_API UECSProjectileModule_SimpleSim : public UECSModule
{
	GENERATED_BODY()
public:
	

	virtual void InitializeComponents(TSharedPtr<flecs::world> World);

	virtual void InitializeSystems(TSharedPtr<flecs::world> World);

};
