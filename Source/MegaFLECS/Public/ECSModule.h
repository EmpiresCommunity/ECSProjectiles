// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"
#include "ECSModule.generated.h"

namespace flecs
{
	class world;
}


/**
 * 
 */
UCLASS()
class MEGAFLECS_API UECSModule : public UObject
{
	GENERATED_BODY()
public:
	UECSModule();

	virtual void InitializeComponents(TSharedPtr<flecs::world> World);

	virtual void InitializeSystems(TSharedPtr<flecs::world> World);

	virtual void FinishInitialize(TSharedPtr<flecs::world> World);

	
};
