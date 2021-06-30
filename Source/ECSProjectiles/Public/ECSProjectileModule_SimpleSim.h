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
struct FECSDebugTag{};

struct FECSBulletVelocity
{
	FVector Velocity;
};

struct FECSBulletRicochet
{
	float Angle;
	float MinSpeed;
};
struct FECSRayCast
{
	FHitResult HitResult;
};
struct FECSBulletHit
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
