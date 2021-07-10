// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectilesNiagaraManager.h"
#include "ECSProjectileModule_Niagara.h"
#include "ECSProjectileBlueprintLib.h"
#include "NiagaraFunctionLibrary.h"


// Sets default values
AECSProjectilesNiagaraManager::AECSProjectilesNiagaraManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AECSProjectilesNiagaraManager::BeginPlay()
{
	Super::BeginPlay();


	UNiagaraComponent* DefaultHitExplosionEffectComponent;

	if(DefaultNiagaraProjectileGroupSystem)
	{
		DefaultHitExplosionEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(DefaultNiagaraProjectileGroupSystem,GetRootComponent(),
		FName(),FVector(),FRotator(),EAttachLocation::SnapToTarget,false);
		FECSNiagaraGroup ProjectileGroup;
		ProjectileGroup.Component = DefaultHitExplosionEffectComponent;
		ProjectileGroup.FirstParameterName = TEXT("ProjectilePositions");
		ProjectileGroup.SecondParameterName = TEXT("PrevProjectilePositions");

		DefaultNiagaraProjectileEntityHandle = UECSProjectileBlueprintLib::SetTempNiagaraManagerEntity(GetWorld(),ProjectileGroup);
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("DefaultNiagaraProjectileGroupSystem invalid for %s"),*GetName())
	}

	if(DefaultHitExplosionEffectSystem)
	{
		FECSNiagaraSystemHandle EffectHandle;

		EffectHandle.System = DefaultHitExplosionEffectSystem;
		DefaultNiagaraHitExplostionEntityHandle = UECSProjectileBlueprintLib::SetTempNiagaraFxHandleEntity(GetWorld(),EffectHandle);
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("DefaultNiagaraProjectileGroupSystem invalid for %s"),*GetName())
	}
	//UE_LOG(LogTemp,Warning,TEXT("Default niagara Entity Managers: %i, %i"),DefaultNiagaraHitExplostionEntityHandle.EntityIDInt(),DefaultNiagaraProjectileEntityHandle.EntityIDInt())

}

// Called every frame
void AECSProjectilesNiagaraManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

