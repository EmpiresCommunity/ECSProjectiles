// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "MegaFLECSTypes.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
#include "ECSProjectilesNiagaraManager.generated.h"



/**This class is intended act as a singleton manager that holds references to Niagara systems the ECS projectiles need,
grouped projectiles and explosions.
**/
UCLASS()
class ECSPROJECTILES_API AECSProjectilesNiagaraManager : public AActor
{
	GENERATED_BODY()

public:
	AECSProjectilesNiagaraManager();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UNiagaraSystem* DefaultHitExplosionEffectSystem;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UNiagaraSystem* DefaultNiagaraProjectileGroupSystem;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FECSEntityHandle DefaultNiagaraProjectileEntityHandle;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FECSEntityHandle DefaultNiagaraHitExplostionEntityHandle;


protected:
	virtual void BeginPlay() override;
	

public:
	virtual void Tick(float DeltaTime) override;
};
