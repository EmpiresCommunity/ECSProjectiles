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

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UNiagaraSystem* DefaultHitExplosionEffectSystem;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	UNiagaraSystem* DefaultNiagaraProjectileGroupSystem;
	
	UPROPERTY(BlueprintReadOnly)
	UNiagaraComponent* DefaultNiagaraProjectileGroupComponent;


	
	UPROPERTY(BlueprintReadWrite)
	FECSEntityHandle DefaultNiagaraProjectileEntityHandle;

	UPROPERTY(BlueprintReadWrite)
	FECSEntityHandle DefaultNiagaraHitExplostionEntityHandle;


protected:
	virtual void BeginPlay() override;
	

public:
	virtual void Tick(float DeltaTime) override;
};
