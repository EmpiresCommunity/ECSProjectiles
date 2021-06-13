// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_Networking.h"
#include "flecs.h"
#include "ECSProjectilesNetworkingActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetDriver.h"



namespace FECSNetworkingSystem
{
	struct FNetworkActorHandle
	{
		TWeakObjectPtr<AECSProjectilesNetworkingActor> NetworkActor;
	};

	void SpawnNetworkingProxy(flecs::iter& itr, FNetworkActorHandle* ActorHandles)
	{		
		UWorld* World = (UWorld*)itr.world().get_context();

		if (!IsValid(World))
		{
			return;
		}
		
		//We only create a networking actor if we are in a network situation, and a server one at that
		bool bIsServer = World->GetNetMode() == NM_DedicatedServer || World->GetNetMode() == NM_ListenServer;
		if (!bIsServer)
		{
			return;
		}

		for (int32 i = 0; i < itr.count(); i++)
		{
			//Create the network actor
			FActorSpawnParameters params;
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AECSProjectilesNetworkingActor* NetworkingActor = World->SpawnActor<AECSProjectilesNetworkingActor>(params);

			ActorHandles[i].NetworkActor = NetworkingActor;
		}
	}


}

void UECSProjectileModule_Networking::InitializeComponents(TSharedPtr<flecs::world> World)
{
	flecs::component<FECSNetworkingSystem::FNetworkActorHandle>(*World.Get());
}


void UECSProjectileModule_Networking::InitializeSystems(TSharedPtr<flecs::world> World)
{
	World->system<FECSNetworkingSystem::FNetworkActorHandle>("Spawn Networking Proxy")
		.kind(flecs::OnSet)
		.iter(&FECSNetworkingSystem::SpawnNetworkingProxy);
}

void UECSProjectileModule_Networking::FinishInitialize(TSharedPtr<flecs::world> World)
{
	UWorld* WorldPtr = (UWorld*)World->get_context();

	WorldPtr->OnActorsInitialized.AddLambda([World](const UWorld::FActorsInitializedParams& Params) {
		World->set<FECSNetworkingSystem::FNetworkActorHandle>({});
	});

	
}
