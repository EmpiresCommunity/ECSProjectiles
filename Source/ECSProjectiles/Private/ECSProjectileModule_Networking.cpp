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


	struct FECSNetworkIdHandle
	{
		int32 NetworkArrayIndex = INDEX_NONE;
	};

	void AssignNetworkingHandle(flecs::entity e, FECSNetworkIdHandle& NetworkIdHandle)
	{		
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
