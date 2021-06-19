// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Channel.h"
#include "ECSNetworkingChannel.generated.h"

/**
 * 
 */
UCLASS(transient, customConstructor)
class ECSPROJECTILES_API UECSNetworkingChannel : public UChannel
{
	GENERATED_BODY()
public:
	UECSNetworkingChannel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	virtual void ReceivedBunch(FInBunch& Bunch) override;
	virtual void Tick() override;

	virtual bool CanStopTicking() const override { return false; }

	virtual void SendHello();

	bool FirstRun;

};
