// Copyright 2021 EmpiresTeam.  Licensed under MIT.  See LICENSE.

#pragma once

#include "flecs.h"
#include "MegaFLECSTypes.generated.h"


USTRUCT(BlueprintType)
struct MEGAFLECS_API FECSEntityHandle
{
	GENERATED_BODY()
public:
	FECSEntityHandle()
		: Entity()
	{

	}

	FECSEntityHandle(flecs::entity entity);

	flecs::entity Entity;

	int32 EntityIDInt() const {return Entity.id();}
};

USTRUCT()
struct MEGAFLECS_API FECSScriptStructComponent
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UScriptStruct* ScriptStruct;
};