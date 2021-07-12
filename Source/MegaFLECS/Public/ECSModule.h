// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"
#include "flecs.h"
#include "MegaFLECSTypes.h"
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

	
	

	template<typename T>
	typename TEnableIf<TProvidesStaticStruct<T>::Value, flecs::entity >::Type
	RegisterComponent(TSharedPtr<flecs::world> World, const char* Name = nullptr)
	{
		flecs::entity comp = flecs::component<T>(*World.Get(), Name);
		comp.set< FECSScriptStructComponent>({ FECSScriptStructComponent::StaticStruct() });

		return comp;
	}

	template<typename T>
	typename TEnableIf<!TProvidesStaticStruct<T>::Value, flecs::entity >::Type
	RegisterComponent(TSharedPtr<flecs::world> World, const char* Name = nullptr)
	{
		return flecs::component<T>(*World.Get(), Name);
	}

	
};
