// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectilesNetworkingActor.h"

AECSProjectilesNetworkingActor::AECSProjectilesNetworkingActor()
{
 	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
}
