// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECSModule.h"
#include "NiagaraSystem.h"

#include "ECSProjectileModule_Niagara.generated.h"

#if ECSPROJECTILES_NIAGARA
struct FECSNiagaraComponentHandle
{
	TWeakObjectPtr<class UNiagaraComponent> Component;
	FName LocationParameterName;
	FName PreviousLocationParameterName;
};


struct FECSRNiagaraProjectileManager{};
struct FECSRNiagaraHitsManager{};
struct FECSRNiagaraHitFX{};


//A Niagara system that is intended to render many projectiles at once. Perhaps one per visual projectile type?
//this is intended to be used as a parent/instance of many bullet entities
#endif

USTRUCT(BlueprintType)

struct FECSNiagaraSystemHandle
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class UNiagaraSystem> System;
};
USTRUCT(BlueprintType)
struct FECSNiagaraGroupManager
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class UNiagaraComponent> Component;
	//generic parameter names. Dare we split these off into components?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FirstParameterName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SecondParameterName;
	//this can't be the smart way to do things here...
	TArray<FVector> FirstArray;
	TArray<FVector> SecondArray;

	//This is required because an iterator is called for every ECS archetype.
	//This gets incremented with the length of the iterator.
	int32 IteratorOffset = 0;
};
/**
 * 
 */
UCLASS()
class ECSPROJECTILES_API UECSProjectileModule_Niagara : public UECSModule
{
	GENERATED_BODY()
public:

	virtual void InitializeComponents(TSharedPtr<flecs::world> World);

	virtual void InitializeSystems(TSharedPtr<flecs::world> World);

	virtual void FinishInitialize(TSharedPtr<flecs::world> World);

};
