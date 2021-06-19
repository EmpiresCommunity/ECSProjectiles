// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Engine/NetSerialization.h"
#include "ECSProjectileModule_SimpleSim.h"
#include "ECSProjectilesNetworkingActor.generated.h"

USTRUCT()
struct FReplicatedProjectileItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
public:

	FECSBulletTransform Transform;
	FECSBulletVelocity Velocity;

	//void PreReplicatedRemove(const struct FReplicatedProjectileArray& InArraySerializer);
	//void PostReplicatedAdd(const struct FReplicatedProjectileArray& InArraySerializer);
	//void PostReplicatedChange(const struct FReplicatedProjectileArray& InArraySerializer);

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

};

template<>
struct TStructOpsTypeTraits< FReplicatedProjectileItem > : public TStructOpsTypeTraitsBase2< FReplicatedProjectileItem >
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT()
struct FReplicatedProjectileArray : public FFastArraySerializer 
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TArray<FReplicatedProjectileItem> Items;

	UPROPERTY()
	AECSProjectilesNetworkingActor* OwningNetworkActor;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FReplicatedProjectileItem, FReplicatedProjectileArray>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits< FReplicatedProjectileArray > : public TStructOpsTypeTraitsBase2< FReplicatedProjectileArray >
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


UCLASS()
class ECSPROJECTILES_API AECSProjectilesNetworkingActor : public AInfo
{
	GENERATED_BODY()
	
public:	
	AECSProjectilesNetworkingActor();

	FReplicatedProjectileArray ReplicatedEntityArray;
};
