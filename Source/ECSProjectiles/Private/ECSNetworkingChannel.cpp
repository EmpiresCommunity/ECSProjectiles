// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSNetworkingChannel.h"
#include "Net/DataBunch.h"

DEFINE_LOG_CATEGORY_STATIC(LogECSNet, Log, All);

UECSNetworkingChannel::UECSNetworkingChannel(const FObjectInitializer& ObjectInitializer)
	: UChannel(ObjectInitializer)
{
	ChName = FECSProjectiles::NAME_ECSChannel;
	FirstRun= true;
}

void UECSNetworkingChannel::ReceivedBunch(FInBunch& Bunch)
{
	uint8 MessageType;

	Bunch << MessageType;

	if (MessageType == 2)
	{
		UE_LOG(LogECSNet, Warning, TEXT("Got the ECS Hello Packet!"));
	}
}

void UECSNetworkingChannel::Tick()
{
	
}

void UECSNetworkingChannel::SendHello()
{
	FOutBunch Bunch(this, false);

	Bunch.bReliable = true;
	if (FirstRun)
	{
		Bunch.bOpen = true;
		FirstRun = false;
	}

	

	uint8 MessageType = 2;
	Bunch << MessageType;

	if (!Bunch.IsError())
	{
		SendBunch(&Bunch, true);
	}
}
