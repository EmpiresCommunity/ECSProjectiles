// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MegaFLECSTypes.h"
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

private:
	static TSharedPtr<flecs::world> GetECSWorld(UWorld* World);
};
