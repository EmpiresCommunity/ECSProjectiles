// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECSModule.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
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
//regular actor that reports back and is managed by the ECS world
struct FECSActorEntity
{
	TSoftObjectPtr<AActor> Actor;
};
//who is responsible for spawning this entity (not a managed actor)
struct FECSSpawnInstigator
{
	TSoftObjectPtr<AActor> Actor;
};
//custom downward gravity speed for gameplay ballistics
struct FECSBulletGravity
{
	float GravityZ;
};

//WHO LIKES HUNGARIAN NOTATION?? I LIKE HUNGARIAN TYPING!!!
USTRUCT(BlueprintType)
struct FECSGASEffectPayload
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag HitGameplayCue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventToFireOnImpact;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEventData OnHitEventData;
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
