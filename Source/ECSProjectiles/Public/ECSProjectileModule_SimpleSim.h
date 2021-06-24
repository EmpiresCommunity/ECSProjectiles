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
struct FECSRayCast
{
	FHitResult HitResult;
};
struct FECSActorEntity
{
	TSoftObjectPtr<AActor> Actor;
};

//custom downward gravity speed for gameplay ballistics
struct FECSBulletGravity
{
	float GravityZ;
};
//an "event" that is just a component representing the hit normal.
//I guess there's no reason we can't just stuff the entire hitresult in there?
//could make our own ECShitresult without the chaff if we want to stay ~Data Oriented~ and keep it small
struct FECSBulletHitNormal
{
	FVector_NetQuantizeNormal Normal;
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
