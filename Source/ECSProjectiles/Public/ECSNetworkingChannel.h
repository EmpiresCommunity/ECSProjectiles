// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Channel.h"
#include "Net/DataBunch.h"
#include "ECSNetworkingChannel.generated.h"

template<uint8 MessageType> class FECSNetworkMessage
{
};

class ECSPROJECTILES_API FECSNetworkMessageInfo
{
public:
	static inline const TCHAR* GetName(uint8 MessageIndex)
	{
		CheckInitialized();
		return Names[MessageIndex];
	}

	static inline bool IsRegistered(uint8 MessageIndex)
	{
		CheckInitialized();
		return Names[MessageIndex][0] != 0;
	}

	template<uint8 MessageType> friend class FECSNetworkMessage;
private:
	static constexpr int MaxNames = TNumericLimits<uint8>::Max() + 1;
	static const TCHAR* Names[MaxNames];

	static void CheckInitialized()
	{
		static bool bInitialized = false;
		if (!bInitialized)
		{
			for (int32 i = 0; i < UE_ARRAY_COUNT(Names); i++)
			{
				Names[i] = TEXT("");
			}
			bInitialized = true;
		}
	}
	static void SetName(uint8 MessageType, const TCHAR* InName)
	{
		CheckInitialized();
		Names[MessageType] = InName;
	}

	template<typename... ParamTypes>
	static void ReceiveParams(FInBunch& Bunch, ParamTypes&... Params) {}

	template<typename FirstParamType, typename... ParamTypes>
	static void ReceiveParams(FInBunch& Bunch, FirstParamType& FirstParam, ParamTypes&... Params)
	{
		Bunch << FirstParam;
		ReceiveParams(Bunch, Params...);
	}

	template<typename... ParamTypes>
	static void SendParams(FOutBunch& Bunch, ParamTypes&... Params) {}

	template<typename FirstParamType, typename... ParamTypes>
	static void SendParams(FOutBunch& Bunch, FirstParamType& FirstParam, ParamTypes&... Params)
	{
		Bunch << FirstParam;
		SendParams(Bunch, Params...);
	}
};



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

	static UECSNetworkingChannel* GetOrCreateECSNetworkingChannelForConnection(UNetConnection* Conn);

};

#define DEFINE_ECS_CHANNEL_MESSAGE(Name, Index, ...) \
enum { NMECS_##Name = Index }; \
template<> class FECSNetworkMessage<Index> \
{ \
public: \
	static uint8 Initialize() \
	{ \
		FECSNetworkMessageInfo::SetName(Index, TEXT(#Name)); \
		return 0; \
	} \
	/** sends a message of this type on the specified connection's control channel \
		* @note: const not used only because of the FArchive interface; the parameters are not modified \
		*/ \
	template<typename... ParamTypes> \
	static void Send(UNetConnection* Conn, ParamTypes&... Params) \
	{ \
		static_assert(Index < FECSNetworkMessageInfo::MaxNames, "ECS Channel Message Header Must be a Byte"); \
		UECSNetworkingChannel* Channel = UECSNetworkingChannel::GetOrCreateECSNetworkingChannelForConnection(Conn); \
		if (IsValid(Channel) && !Channel->Closing) \
		{ \
			FOutBunch Bunch(Channel, false); \
			Pack(Bunch, Params...); \
			Channel->SendBunch(&Bunch, true); \
		} \
	} \
	template<typename... ParamTypes> \
	static void Pack(FOutBunch& Bunch, ParamTypes&... Params) \
	{ \
		uint8 MessageType = Index; \
		Bunch << MessageType; \
		FECSNetworkMessageInfo::SendParams(Bunch, Params...); \
	} \
	/** receives a message of this type from the passed in bunch */ \
	template<typename... ParamTypes> \
	UE_NODISCARD static bool Receive(FInBunch& Bunch, ParamTypes&... Params) \
	{ \
		FECSNetworkMessageInfo::ReceiveParams(Bunch, Params...); \
		return !Bunch.IsError(); \
	} \
	/** throws away a message of this type from the passed in bunch */ \
	static void Discard(FInBunch& Bunch) \
	{ \
		TTuple<__VA_ARGS__> Params; \
		VisitTupleElements([&Bunch](auto& Param) \
		{ \
			Bunch << Param; \
		}, \
		Params); \
	} \
};

#define IMPLEMENT_ECS_CHANNEL_MESSAGE(Name) static uint8 Dummy##_FECSNetworkMessage_##Name = FECSNetworkMessage<NMECS_##Name>::Initialize();

using FECSNetworkEntityHandle = uint64;
using FECSNetworkComponentHandle = uint32;

DEFINE_ECS_CHANNEL_MESSAGE(Hello,			0);
DEFINE_ECS_CHANNEL_MESSAGE(CreateEntity,	1,	FECSNetworkEntityHandle, TArray<FString>, TArray<uint8>);						//Creates a new entity with a given network entity handle, with optional component arrays and data
DEFINE_ECS_CHANNEL_MESSAGE(UpdateComponent, 2,	FECSNetworkEntityHandle, FECSNetworkComponentHandle, TArray<uint8>);			//Update a component with raw bit data for that component
DEFINE_ECS_CHANNEL_MESSAGE(DestroyEntity,	3,	FECSNetworkEntityHandle);														//Destroyes an entity
DEFINE_ECS_CHANNEL_MESSAGE(CreateComponent, 4,	FECSNetworkEntityHandle, FECSNetworkComponentHandle, FString, TArray<uint8>)	//Creates a new component on an entity
DEFINE_ECS_CHANNEL_MESSAGE(DestroyComponent, 5, FECSNetworkEntityHandle, FECSNetworkComponentHandle)							//Destroys a component on an entity