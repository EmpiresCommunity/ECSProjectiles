// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileBlueprintLib.h"

#include "ECSProjectileModule_Niagara.h"
#include "ECSWorldSubsystem.h"
#include "ECSProjectileModule_SimpleSim.h"

FECSEntityHandle UECSProjectileBlueprintLib::SpawnECSBullet(UObject* WorldContextObject, FTransform SpawnTransform, float Velocity, TSubclassOf<AActor> ProjectileActor)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* actor = World->SpawnActor<AActor>(ProjectileActor, SpawnTransform, SpawnParams);

	TSharedPtr<flecs::world> ECSWorld = GetECSWorld(World);

	flecs::entity e = ECSWorld->entity()
		.set<FECSActorEntity>({ actor })
		.set<FECSBulletTransform>({SpawnTransform, SpawnTransform})
		.set<FECSBulletVelocity>({ SpawnTransform.GetRotation().GetForwardVector() * Velocity });

	return { e };
}


FECSEntityHandle UECSProjectileBlueprintLib::SpawnECSBulletNiagaraGrouped(UObject* WorldContextObject,FECSEntityHandle NiagaraEntityId, FTransform SpawnTransform, float Velocity,bool bShouldRicochet)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	TSharedPtr<flecs::world> ECSWorld = GetECSWorld(World);

	
	flecs::entity e = ECSWorld->entity()
		.set<FECSBulletTransform>({SpawnTransform, SpawnTransform})
		.set<FECSBulletVelocity>({ SpawnTransform.GetRotation().GetForwardVector() * Velocity })
		.set<FECSBulletGravity>({World->GetGravityZ()})
		.add<FECSRayCast>()
			//this is an entity pair!
			.add<FECSNiagaraProjectileRelationComponent>(NiagaraEntityId.Entity);

	if(bShouldRicochet) e.set<FECSBulletRicochet>({30.0f,100.0f});


	
	
	return { e };
}

FECSEntityHandle UECSProjectileBlueprintLib::SetTempNiagaraManagerEntity(UObject* WorldContextObject, FECSNiagaraGroupManager NiagaraComponentHandle)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	
	//make a NiagaraGroupHandle entity
	//future: make Niagara handles generic? two different entities? depends on how crazy we get here
	
	return GetECSWorld(World)->entity()
		.set<FECSNiagaraGroupManager>(NiagaraComponentHandle);
	
}

TSharedPtr<flecs::world> UECSProjectileBlueprintLib::GetECSWorld(UWorld* World)
{
	if (UECSWorldSubsystem* WorldSubsystem = World->GetSubsystem<UECSWorldSubsystem>())
	{
		return WorldSubsystem->GetEcsWorld();
	}

	return nullptr;
}
