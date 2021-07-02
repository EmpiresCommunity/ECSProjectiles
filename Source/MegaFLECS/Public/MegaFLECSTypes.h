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

	FECSEntityHandle(FECSEntityHandle& Other);


	flecs::entity Entity;

	int32 EntityIDInt() const {return Entity.id();}
};