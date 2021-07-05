// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSNetworkingChannel.h"
#include "Net/DataBunch.h"
#include "Engine/NetConnection.h"

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

void UECSNetworkingChannel::SendNewEntity(FECSNetworkingSystem::FECSNetworkEntityHandle EntityHandle, TArray<FECSNetworkingSystem::FECSNetworkComponetCreationData>& ComponentCreationInfos)
{
	FOutBunch Bunch(this, false);
	Bunch.bReliable = true;

	FECSNetworkMessage<NMECS_CreateEntity>::Pack(Bunch, EntityHandle, ComponentCreationInfos);

	if (!Bunch.IsError())
	{
		SendBunch(&Bunch, true);
	}
}

FArchive& operator<<(FArchive& Ar, FECSNetworkingSystem::FECSNetworkComponetCreationData& ComponentData)
{
	Ar<< ComponentData.NetworkHandle;
	Ar<< ComponentData.ComponentName;
	Ar<< ComponentData.InitialComponentData;

	return Ar;
}
