// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ECSWorldSubsystem.generated.h"

namespace flecs
{
	class world;
}

/**
 * 
 */
UCLASS()
class MEGAFLECS_API UECSWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UECSWorldSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TSharedPtr<flecs::world> GetEcsWorld() const { return ECSWorld; }

protected:	
	FTickerDelegate OnTickDelegate;
	FDelegateHandle OnTickHandle;

	TSharedPtr<flecs::world> ECSWorld;

private:
	bool Tick(float DeltaTime);
};
