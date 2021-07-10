// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ECSProjectileModule_Niagara.h"
#include "ECSProjectileModule_SimpleSim.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MegaFLECSTypes.h"
#include "NiagaraComponent.h"
#include "ECSProjectileBlueprintLib.generated.h"


/**
 * 
 */
UCLASS()
class ECSPROJECTILES_API UECSProjectileBlueprintLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ECSBullet", meta = (WorldContext = "WorldContextObject"))

	FECSEntityHandle SpawnECSBullet(UObject* WorldContextObject, FTransform SpawnTransform, float Velocity,
	                                TSubclassOf<AActor> ProjectileActor);
	UFUNCTION(BlueprintCallable, Category = "ECSBullet", meta = (WorldContext = "WorldContextObject"))
	static FECSEntityHandle SpawnECSBulletNiagaraGrouped(UObject* WorldContextObject, FECSGASEffectPayload EffectPayload, FECSEntityHandle NiagaraProjectilesEntityId, FECSEntityHandle
	                                                     NiagaraHitsEntityId, FTransform
	                                                     SpawnTransform, float Velocity = 600.0f, bool
	                                                     bShouldRicochet = false, bool bGroupedHits = false);

	UFUNCTION(BlueprintCallable, Category = "ECSBullet", meta = (WorldContext = "WorldContextObject"))
	static FECSEntityHandle SetTempNiagaraManagerEntity(UObject* WorldContextObject, FECSNiagaraGroup NiagaraGroupHandle);

	UFUNCTION(BlueprintCallable, Category = "ECSBullet", meta = (WorldContext = "WorldContextObject"))
	static FECSEntityHandle SetTempNiagaraFxHandleEntity(UObject* WorldContextObject,
	                                              FECSNiagaraSystemHandle NiagaraSystemHandle);

private:
	static TSharedPtr<flecs::world> GetECSWorld(UWorld* World);
};
