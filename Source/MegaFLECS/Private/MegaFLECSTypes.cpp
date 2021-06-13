// Copyright 2021 EmpiresTeam.  Licensed under MIT.  See LICENSE.

#include "MegaFLECSTypes.h"
#include "flecs.h"

FECSEntityHandle::FECSEntityHandle(flecs::entity entity)
	: Entity(entity)
{

}

FECSEntityHandle::FECSEntityHandle(FECSEntityHandle& Other)
	: Entity(Other.Entity)
{

}
