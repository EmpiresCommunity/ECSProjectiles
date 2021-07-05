// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_Networking.h"
#include "flecs.h"
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
	struct FNetworkChannelHandle
	{
		int32 foo;
	};
	
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
					UECSNetworkingChannel::GetOrCreateECSNetworkingChannelForConnection(Connection);
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

	template<class Function>
	void ForEachConnection(const UWorld* const World, const Function InFunction)
	{
		if (GEngine == nullptr || World == nullptr)
		{
			return;
		}

		FWorldContext* const Context = GEngine->GetWorldContextFromWorld(World);
		if (Context != nullptr)
		{
			for (FNamedNetDriver& Driver : Context->ActiveNetDrivers)
			{
				for (UNetConnection* Connection : Driver.NetDriver->ClientConnections)
				{
					InFunction(Connection);
				}				
			}
		}
	}	

	void AssignNetworkingHandle(flecs::entity e, FECSNetworkIdHandle& NetworkIdHandle)
	{		
	}

	void ProcessNetworkedEntities(flecs::iter& itr, FECSNetworkIdHandle* NetworkIdHandles)
	{
		UWorld* World = (UWorld*)itr.world().get_context();

		int32 count = itr.count();
		for (int32 i = 0; i < count; i++)
		{
			FECSNetworkIdHandle& NetIdHandle = NetworkIdHandles[i];
			flecs::entity e = itr.entity(i);


			//If the network ID is INDEX_NONE, that means it has not be assigned.  
			//Assign an id and send it to all clients
			if (NetIdHandle.NetworkEntityId == INDEX_NONE)
			{
				static int32 NetId = 0;
				NetIdHandle.NetworkEntityId = NetId++;

				TArray< FECSNetworkComponetCreationData> ComponentCreateInfos;
			
				//Assign component IDs to this entity's networked components
				int32 ComponentId = 0;
				
				e.each<FECSNetworkComponentIDHandle>([&ComponentId, &e, &ComponentCreateInfos](flecs::entity component) {
			
					FECSNetworkComponentIDHandle* id = e.get_mut<FECSNetworkComponentIDHandle>(component);
					if (id != nullptr && id->NetworkedComponentId == INDEX_NONE)
					{
						id->NetworkedComponentId = ComponentId++;
					}

					FECSNetworkComponetCreationData CreateInfo = ComponentCreateInfos.AddDefaulted_GetRef();
					CreateInfo.NetworkHandle = id->NetworkedComponentId;
					CreateInfo.ComponentName = FString(component.name().c_str());

					//TODO: Pack the component data into the ComponentCreateInfo's data struct
				

				});

				//Send the entity data to all clients
				ForEachConnection(World, [&NetIdHandle, &ComponentCreateInfos](UNetConnection* Connection)
				{
					UECSNetworkingChannel* NetChannel =	UECSNetworkingChannel::GetOrCreateECSNetworkingChannelForConnection(Connection);

					//Send this entity
					NetChannel->SendNewEntity(NetIdHandle.NetworkEntityId, ComponentCreateInfos);
				});
			}
			else
			{
				//Get all the networked components and send them to the client
			}
		}
	}
		
}

void UECSProjectileModule_Networking::InitializeComponents(TSharedPtr<flecs::world> World)
{
	flecs::component<FECSNetworkingSystem::FECSNetworkIdHandle>(*World.Get());
	flecs::component<FECSNetworkingSystem::FNetworkChannelHandle>(*World.Get());
}


void UECSProjectileModule_Networking::InitializeSystems(TSharedPtr<flecs::world> World)
{
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
		}

		
	});

	
}
