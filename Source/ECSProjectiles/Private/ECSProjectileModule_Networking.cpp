// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_Networking.h"
#include "flecs.h"
#include "ECSProjectilesNetworkingActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetDriver.h"
#include "ECSProjectileModule_SimpleSim.h"



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

		for (int32 i = 0; i < itr.count(); i++)
		{
			//Create the network actor
			FActorSpawnParameters params;
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AECSProjectilesNetworkingActor* NetworkingActor = World->SpawnActor<AECSProjectilesNetworkingActor>(params);

			ActorHandles[i].NetworkActor = NetworkingActor;
		}
	}

	struct FECSNetworkIdHandle
	{
		int32 NetworkArrayIndex = INDEX_NONE;
	};

	void AssignNetworkingHandle(flecs::entity e, FECSNetworkIdHandle& NetworkIdHandle)
	{
		const FNetworkActorHandle* ActorHandle = e.world().get<FNetworkActorHandle>();

		if (ActorHandle == nullptr)
		{
			return;
		}

		if (!ActorHandle->NetworkActor.IsValid())
		{
			return;
		}

		AECSProjectilesNetworkingActor* NetworkingActor = ActorHandle->NetworkActor.Get();

		NetworkIdHandle.NetworkArrayIndex = NetworkingActor->ReplicatedEntityArray.Items.Add(FReplicatedProjectileItem());
		NetworkingActor->ReplicatedEntityArray.MarkArrayDirty();

	}


	void PackEntitiesForNetworking(flecs::entity e, const FNetworkActorHandle& ActorHandle)
	{
		auto query = e.world().query<const FECSBulletTransform, const FECSBulletVelocity>();

		query.iter([ActorHandle](flecs::iter& iterator, const FECSBulletTransform* Transforms, const FECSBulletVelocity* Velocities, const FECSNetworkIdHandle* NetworkIDHandles)
			{
				int32 count = iterator.count();

				for (int32 i = 0; i < count; i++)
				{
					auto entity = iterator.entity(i);
					
					const FECSBulletTransform& TForm = Transforms[i];
					const FECSBulletVelocity& Vel = Velocities[i];
					const FECSNetworkIdHandle& NetId = NetworkIDHandles[i];

					AECSProjectilesNetworkingActor* NetworkActor = ActorHandle.NetworkActor.Get();

					


				}
			});
	}
}

void UECSProjectileModule_Networking::InitializeComponents(TSharedPtr<flecs::world> World)
{
	flecs::component<FECSNetworkingSystem::FNetworkActorHandle>(*World.Get());
	flecs::component<FECSNetworkingSystem::FECSNetworkIdHandle>(*World.Get());
}


void UECSProjectileModule_Networking::InitializeSystems(TSharedPtr<flecs::world> World)
{
	World->system<FECSNetworkingSystem::FNetworkActorHandle>("Spawn Networking Proxy")
		.kind(flecs::OnSet)
		.iter(&FECSNetworkingSystem::SpawnNetworkingProxy);

	World->system<FECSNetworkingSystem::FECSNetworkIdHandle>("Assign Network Transport")
		.kind(flecs::OnSet)
		.each(&FECSNetworkingSystem::AssignNetworkingHandle);
}

void UECSProjectileModule_Networking::FinishInitialize(TSharedPtr<flecs::world> World)
{
	UWorld* WorldPtr = (UWorld*)World->get_context();

	WorldPtr->OnActorsInitialized.AddLambda([World](const UWorld::FActorsInitializedParams& Params) {
		//We only create a networking entity/actor if we are in a network situation, and a server one at that
		bool bIsServer = Params.World->GetNetMode() == NM_DedicatedServer || Params.World->GetNetMode() == NM_ListenServer;
		if (!bIsServer)
		{
			return;
		}

		World->set<FECSNetworkingSystem::FNetworkActorHandle>({});
	});

	
}
