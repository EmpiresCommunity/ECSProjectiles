// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ECSProjectileModule_Niagara.h"
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
	static FECSEntityHandle SpawnECSBullet(UObject* WorldContextObject, FTransform SpawnTransform, float Velocity, TSubclassOf<AActor> ProjectileActor);

	UFUNCTION(BlueprintCallable, Category = "ECSBullet", meta = (WorldContext = "WorldContextObject"))
	static FECSEntityHandle SpawnECSBulletNiagaraGrouped(UObject* WorldContextObject,FString NiagaraBulletManagerName, FTransform SpawnTransform, float Velocity);


	UFUNCTION(BlueprintCallable, Category = "ECSBullet", meta = (WorldContext = "WorldContextObject"))
	static void SetTempNiagaraSingleton(UObject* WorldContextObject, FECSNiagaraGroupHandle ComponentRef);

private:
	static TSharedPtr<flecs::world> GetECSWorld(UWorld* World);
};
