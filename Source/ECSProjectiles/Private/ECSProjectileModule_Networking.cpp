// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_Networking.h"
#include "flecs.h"
#include "ECSProjectilesNetworkingActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "ECSProjectileModule_SimpleSim.h"
#include "ECSNetworkingChannel.h"

DEFINE_LOG_CATEGORY_STATIC(LogECSNetworkSystems, Log, All);

namespace FECSProjectiles
{
	const FName NAME_ECSChannel(TEXT("EcsChannel"));
}

namespace FECSNetworkingSystem
{
	

	struct FNetworkActorHandle
	{
		TWeakObjectPtr<AECSProjectilesNetworkingActor> NetworkActor;
	};

	struct FNetworkChannelHandle
	{
		int32 foo;
	};

	UECSNetworkingChannel* GetOrCreateECSNetworkingChannelForConnection(UNetConnection* Connection)
	{
		for (UChannel* Channel : Connection->Channels)
		{
			if (IsValid(Channel) && Channel->ChName == FECSProjectiles::NAME_ECSChannel)
			{
				return Cast<UECSNetworkingChannel>(Channel);
			}
		}

		if (Connection->GetDriver()->IsKnownChannelName(FECSProjectiles::NAME_ECSChannel))
		{
			return Cast<UECSNetworkingChannel>(Connection->CreateChannelByName(FECSProjectiles::NAME_ECSChannel, EChannelCreateFlags::None));
		}

		return nullptr;
	}

	void SpawnReplicationChannel(flecs::iter & itr, FNetworkChannelHandle* ChannelHandles)
	{
		UWorld* World = (UWorld*)itr.world().get_context();

		if (!IsValid(World))
		{
			return;
		}
		
		int32 count = itr.count();
		for (int32 i = 0; i < count; i++)
		{	
			if (UNetDriver* NetDriver = World->GetNetDriver())
			{
				for (UNetConnection* Connection : NetDriver->ClientConnections)
				{
					if (UECSNetworkingChannel* Chan = GetOrCreateECSNetworkingChannelForConnection(Connection))
					{						
						Chan->SendHello();
					}
				}				
			}
		}
	}

	void TEMP_SendHello(flecs::iter& itr, FNetworkChannelHandle* ChannelHandles)
	{
		UWorld* World = (UWorld*)itr.world().get_context();

		if (!IsValid(World))
		{
			return;
		}

		if (UNetDriver* NetDriver = World->GetNetDriver())
		{
			for (UNetConnection* Connection : NetDriver->ClientConnections)
			{
				if (UECSNetworkingChannel* Chan = GetOrCreateECSNetworkingChannelForConnection(Connection))
				{
					Chan->SendHello();
				}
			}
		}
	}

	template<class Function>
	static void ForEachNetDriver(UEngine* Engine, const UWorld* const World, const Function InFunction)
	{
		if (Engine == nullptr || World == nullptr)
		{
			return;
		}

		FWorldContext* const Context = Engine->GetWorldContextFromWorld(World);
		if (Context != nullptr)
		{
			for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
			{
				InFunction(Driver.NetDriver);
			}
		}
	}



	void AddChannelDefinitionToNetDriver(UNetDriver* NetDriver)
	{
		FChannelDefinition ECSChannelDefinition;
		ECSChannelDefinition.ChannelName = FECSProjectiles::NAME_ECSChannel;
		ECSChannelDefinition.ChannelClass = UECSNetworkingChannel::StaticClass();
		ECSChannelDefinition.StaticChannelIndex = INDEX_NONE;
		ECSChannelDefinition.bTickOnCreate = true;
		ECSChannelDefinition.bServerOpen = true;
		ECSChannelDefinition.bClientOpen = false;

		
		NetDriver->ChannelDefinitions.Add(ECSChannelDefinition);
		NetDriver->ChannelDefinitionMap.Add(FECSProjectiles::NAME_ECSChannel, ECSChannelDefinition);
	}

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
		auto query = e.world().query<const FECSNetworkIdHandle>();

		query.iter([ActorHandle](flecs::iter& iterator, const FECSNetworkIdHandle* NetworkIDHandles)
			{
				int32 count = iterator.count();

				for (int32 i = 0; i < count; i++)
				{
					auto entity = iterator.entity(i);
					
					if (entity.has<FECSBulletTransform>())
					{
						
					}

					if (entity.has<FECSBulletVelocity>())
					{

					}

					//const FECSBulletTransform& TForm = Transforms[i];
					//const FECSBulletVelocity& Vel = Velocities[i];
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
	flecs::component<FECSNetworkingSystem::FNetworkChannelHandle>(*World.Get());
}


void UECSProjectileModule_Networking::InitializeSystems(TSharedPtr<flecs::world> World)
{
	World->system<FECSNetworkingSystem::FNetworkActorHandle>("Spawn Networking Proxy")
		.kind(flecs::OnSet)
		.iter(&FECSNetworkingSystem::SpawnNetworkingProxy);

	World->system<FECSNetworkingSystem::FNetworkChannelHandle>("Spawn Network Channel")
		.kind(flecs::OnSet)
		.iter(&FECSNetworkingSystem::SpawnReplicationChannel);

	World->system<FECSNetworkingSystem::FECSNetworkIdHandle>("Assign Network Transport")
		.kind(flecs::OnSet)
		.each(&FECSNetworkingSystem::AssignNetworkingHandle);
}

void UECSProjectileModule_Networking::FinishInitialize(TSharedPtr<flecs::world> World)
{
	UWorld* WorldPtr = (UWorld*)World->get_context();

	FNetDelegates::OnPendingNetGameConnectionCreated.AddLambda([](UPendingNetGame* NetGame) {
		if (!IsValid(GEngine))
		{
			return;
		}

		const FWorldContext* const WorldContext = GEngine->GetWorldContextFromPendingNetGame(NetGame);
		for (const FNamedNetDriver& Driver : WorldContext->ActiveNetDrivers)
		{
			//Ensure we have the ECS channel definition here
			if (IsValid(Driver.NetDriver) && !Driver.NetDriver->IsKnownChannelName(FECSProjectiles::NAME_ECSChannel))
			{
				FECSNetworkingSystem::AddChannelDefinitionToNetDriver(Driver.NetDriver);
				UE_LOG(LogECSNetworkSystems, Warning, TEXT("Pending Game Registration"));
			}
		}
		
	});

	WorldPtr->OnWorldBeginPlay.AddLambda([World, WorldPtr]() {


		if (IsValid(WorldPtr->NetDriver))
		{
			FECSNetworkingSystem::ForEachNetDriver(GEngine, WorldPtr, &FECSNetworkingSystem::AddChannelDefinitionToNetDriver);

			UE_LOG(LogECSNetworkSystems, Warning, TEXT("Registered Channel Definition %s"), WorldPtr->NetDriver->IsServer() ? TEXT("Server") : TEXT("Client"))
		}
		else
		{
			UE_LOG(LogECSNetworkSystems, Warning, TEXT("Tried to register net channel but null netdriver"));
		}

		//We only create a networking entity/actor if we are in a network situation, and a server one at that
		bool bIsServer = WorldPtr->GetNetMode() == NM_DedicatedServer || WorldPtr->GetNetMode() == NM_ListenServer;
		if (bIsServer)
		{		

			World->set<FECSNetworkingSystem::FNetworkChannelHandle>({});

			World->system< FECSNetworkingSystem::FNetworkChannelHandle>("Send lots of hellos")
				.interval(2.0f)
				.iter(&FECSNetworkingSystem::TEMP_SendHello);

			World->set<FECSNetworkingSystem::FNetworkActorHandle>({});
		}

		
	});

	
}
