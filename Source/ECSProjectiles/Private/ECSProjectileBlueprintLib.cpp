// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileBlueprintLib.h"

#include "ECSNetworkingChannel.h"
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


FECSEntityHandle UECSProjectileBlueprintLib::SpawnECSBulletNiagaraGrouped(UObject* WorldContextObject,FECSGASEffectPayload EffectPayload, FECSEntityHandle NiagaraProjectilesEntityId, FECSEntityHandle NiagaraHitsEntityId, FTransform
                                                                          SpawnTransform, float Velocity, bool bShouldRicochet,bool bGroupedHits)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	TSharedPtr<flecs::world> ECSWorld = GetECSWorld(World);

	//ECSWorld->defer_begin();
	flecs::entity e = ECSWorld->entity()
		.set<FECSBulletTransform>({SpawnTransform, SpawnTransform})
		.set<FECSNetworkingSystem::FECSNetworkComponentIDHandle,FECSBulletTransform>({0,sizeof(FECSBulletTransform)})
		.set<FECSBulletVelocity>({ SpawnTransform.GetRotation().GetForwardVector() * Velocity })
		.set<FECSNetworkingSystem::FECSNetworkComponentIDHandle,FECSBulletVelocity>({0,sizeof(FECSBulletVelocity)})

		.set<FECSGASEffectPayload>(EffectPayload)
		.set<FECSNetworkingSystem::FECSNetworkComponentIDHandle,FECSGASEffectPayload>({0,sizeof(FECSGASEffectPayload)})
		.set<FECSBulletGravity>({World->GetGravityZ()})
		.add<FECSRayCast>()
			//this is an entity pair!
			.add<FECSRNiagaraProjectileManager>(NiagaraProjectilesEntityId.Entity);

	if(bShouldRicochet)
	{
		e.set<FECSBulletRicochet>({30.0f,Velocity/2.0f});
	}

	if(bGroupedHits)
	{
			e.add<FECSRNiagaraHitsManager>(NiagaraHitsEntityId.Entity);
	}
	else
	{
		e.add<FECSRNiagaraHitFX>(NiagaraHitsEntityId.Entity);
	}
	e.set<FECSNetworkingSystem::FECSNetworkIdHandle>({INDEX_NONE});
	//ECSWorld->defer_end();

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
FECSEntityHandle UECSProjectileBlueprintLib::SetTempNiagaraFxHandleEntity(UObject* WorldContextObject, FECSNiagaraSystemHandle NiagaraSystemHandle)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	
	//make a NiagaraGroupHandle entity
	//future: make Niagara handles generic? two different entities? depends on how crazy we get here
	
	return GetECSWorld(World)->entity()
		.set<FECSNiagaraSystemHandle>(NiagaraSystemHandle);
	
}
TSharedPtr<flecs::world> UECSProjectileBlueprintLib::GetECSWorld(UWorld* World)
{
	if (UECSWorldSubsystem* WorldSubsystem = World->GetSubsystem<UECSWorldSubsystem>())
	{
		return WorldSubsystem->GetEcsWorld();
	}

	return nullptr;
}
