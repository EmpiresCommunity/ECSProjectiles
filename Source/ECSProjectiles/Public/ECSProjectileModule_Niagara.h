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
#endif 
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
