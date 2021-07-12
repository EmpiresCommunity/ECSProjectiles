// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSNetworkingChannel.h"
#include "Net/DataBunch.h"
#include "Engine/NetConnection.h"
#include "flecs.h"
#include "MegaFLECSTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogECSNet, Log, All);

const TCHAR* FECSNetworkMessageInfo::Names[FECSNetworkMessageInfo::MaxNames];

IMPLEMENT_ECS_CHANNEL_MESSAGE(Hello)
IMPLEMENT_ECS_CHANNEL_MESSAGE(CreateEntity)
IMPLEMENT_ECS_CHANNEL_MESSAGE(UpdateComponent)
IMPLEMENT_ECS_CHANNEL_MESSAGE(DestroyEntity)
IMPLEMENT_ECS_CHANNEL_MESSAGE(CreateComponent)
IMPLEMENT_ECS_CHANNEL_MESSAGE(DestroyComponent)

using namespace  FECSNetworkingSystem;

UECSNetworkingChannel::UECSNetworkingChannel(const FObjectInitializer& ObjectInitializer)
	: UChannel(ObjectInitializer)
{
	ChName = FECSProjectiles::NAME_ECSChannel;
}

void UECSNetworkingChannel::Init(UNetConnection* InConnection, int32 InChIndex, EChannelCreateFlags CreateFlags)
{
	Super::Init(InConnection, InChIndex, CreateFlags);
	if (OpenedLocally)
	{
		FOutBunch Bunch(this, false);
		Bunch.bReliable = true;
		Bunch.bOpen = true;
		FECSNetworkMessage<NMECS_Hello>::Pack(Bunch);
		if (!Bunch.IsError())
		{
			SendBunch(&Bunch, true);
		}
	}
}

void UECSNetworkingChannel::ReceivedBunch(FInBunch& Bunch)
{
	uint8 MessageType;
	Bunch << MessageType;

	switch (MessageType)
	{
		case NMECS_Hello:
			UE_LOG(LogECSNet, Warning, TEXT("Got the ECS Hello Packet!"));
			FECSNetworkMessage<NMECS_Hello>::Discard(Bunch);
		break;
		case NMECS_CreateEntity:
			FECSNetworkMessage<NMECS_CreateEntity>::Discard(Bunch);
		break;
		case NMECS_CreateComponent:
			FECSNetworkMessage<NMECS_CreateComponent>::Discard(Bunch);
		break;
		case NMECS_DestroyEntity:
			FECSNetworkMessage<NMECS_DestroyEntity>::Discard(Bunch);
		break;
		case NMECS_UpdateComponent:
			FECSNetworkMessage<NMECS_UpdateComponent>::Discard(Bunch);
		break;
		case NMECS_DestroyComponent:
			FECSNetworkMessage<NMECS_DestroyComponent>::Discard(Bunch);
		break;
		default:
			UE_LOG(LogECSNet, Warning, TEXT("Got an unknown ECS Packet: %d"), MessageType);
		break;
	}
	
}

void UECSNetworkingChannel::Tick()
{
	
}

UECSNetworkingChannel* UECSNetworkingChannel::GetOrCreateECSNetworkingChannelForConnection(UNetConnection* Conn)
{
	for (UChannel* Channel : Conn->Channels)
	{
		if (IsValid(Channel) && Channel->ChName == FECSProjectiles::NAME_ECSChannel)
		{
			return Cast<UECSNetworkingChannel>(Channel);
		}
	}

	if (Conn->GetDriver()->IsKnownChannelName(FECSProjectiles::NAME_ECSChannel))
	{
		return Cast<UECSNetworkingChannel>(Conn->CreateChannelByName(FECSProjectiles::NAME_ECSChannel, EChannelCreateFlags::None));
	}

	return nullptr;
}


bool NetSerializeEntity(flecs::entity e, FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	int32 NumComponents;
	FECSNetworkEntityHandle EHandle;

	if (Ar.IsSaving())
	{
		const FECSNetworkIdHandle* IdHandle = e.get<FECSNetworkIdHandle>();
		if (IdHandle == nullptr || IdHandle->NetworkEntityId == INDEX_NONE)
		{
			bOutSuccess = false;
			return false;
		}

		EHandle = IdHandle->NetworkEntityId;
	}
	TArray<flecs::entity> Components;
	//If we are saving, collect the components to replicate
	if (Ar.IsSaving())
	{
		
		e.each< FECSNetworkComponentIDHandle>([&e, &Components](flecs::entity component) {
			const FECSNetworkComponentIDHandle* id = e.get<FECSNetworkComponentIDHandle>(component);
			if (id != nullptr && id->NetworkedComponentId != INDEX_NONE && e.has<FECSScriptStructComponent>())
			{
				Components.Add(component);
			}
		});

		NumComponents = Components.Num();
	}

	//Write the entity create header
	if (Ar.IsSaving())
	{
		FECSNetworkMessage<NMECS_CreateEntity>::Pack(Ar, EHandle, NumComponents);
	}
	else
	{
		bool bSuccess = FECSNetworkMessage<NMECS_CreateEntity>::Receive(Ar, EHandle, NumComponents);
		//TODO: check if this is false, throw an error and panic
	}

	for (int i = 0; i < NumComponents; i++)
	{
		FECSNetworkComponentHandle ComponetId;
		UScriptStruct* ScriptStruct;
		void* ComponentData = nullptr;

		if (Ar.IsSaving())
		{			
			flecs::entity comp = Components[i];
			ComponetId = e.get<FECSNetworkComponentIDHandle>()->NetworkedComponentId;
			ScriptStruct = comp.get<FECSScriptStructComponent>()->ScriptStruct;
			ComponentData = e.get_mut(comp.object());
		}

		Ar << ComponetId;
		Ar << ScriptStruct;

		//TODO: Figure out how to set the component here, mapping it to a proper component.  
		//Perhaps loop through all the components and find the one with a ScriptStruct that matches what was replicated to us?
		
		if (Ar.IsLoading())
		{
			static flecs::query<FECSScriptStructComponent> StructQuery = e.world().query<FECSScriptStructComponent>();

			flecs::entity FoundComponent;
			StructQuery.each([&FoundComponent, ScriptStruct](flecs::entity e, const FECSScriptStructComponent& StructComp)
				{
					if (StructComp.ScriptStruct == ScriptStruct)
					{
						FoundComponent = e;
					}
				});

			if (!FoundComponent.is_valid())
			{
				//Throw an error
				Ar.SetError();
				bOutSuccess = false;
				return false;
			}

			e.add(FoundComponent);
			e.set<FECSNetworkComponentHandle>(FoundComponent, {ComponetId});			

			ComponentData = e.get_mut(FoundComponent);
		}

		if (ComponentData == nullptr)
		{
			//Panic
			bOutSuccess = false;
			return false;
		}
		
		ScriptStruct->GetCppStructOps()->NetSerialize(Ar, Map, bOutSuccess, ComponentData);

	}

	return true;
}

void UECSNetworkingChannel::SendEntityToClient(flecs::entity entity)
{
	//TODO: make an OutBunch and call NetSerializeEntity, then send it!

}



FArchive& operator<<(FArchive& Ar, FECSNetworkingSystem::FECSNetworkComponetCreationData& ComponentData)
{
	Ar<< ComponentData.NetworkHandle;
	Ar<< ComponentData.ComponentName;
	Ar<< ComponentData.InitialComponentData;

	return Ar;
}
