// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ECSModule.h"
#include "ECSProjectileModule_Niagara.generated.h"

#if ECSPROJECTILES_NIAGARA
struct FECSNiagaraHandle
{
	TWeakObjectPtr<class UNiagaraComponent> Component;
	FName LocationParameterName;
	FName PreviousLocationParameterName;
};
//A Niagara system that is intended to render many projectiles at once. Perhaps one per visual projectile type?
//this is intended to be used as a parent/instance of many bullet entities

#endif
USTRUCT(BlueprintType)
struct FECSNiagaraGroupHandle
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<class UNiagaraComponent> Component;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationsParameterName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreviousLocationsParameterName;
	//this can't be the smart way to do things here...
	TArray<FVector> ParticleLocations;
	TArray<FVector> PreviousParticleLocations;

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


};
