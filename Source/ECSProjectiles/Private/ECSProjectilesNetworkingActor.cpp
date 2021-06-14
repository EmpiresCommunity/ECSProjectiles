// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectilesNetworkingActor.h"

AECSProjectilesNetworkingActor::AECSProjectilesNetworkingActor()
{
 	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;

    ReplicatedEntityArray.OwningNetworkActor = this;
}

bool FReplicatedProjectileItem::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    return true;
}
