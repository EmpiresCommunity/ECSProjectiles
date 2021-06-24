// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_Niagara.h"
#include "flecs.h"
#include "ECSProjectileModule_SimpleSim.h"
#include "MegaFLECSTypes.h"
#if ECSPROJECTILES_NIAGARA
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFloat.h"
#include "NiagaraFunctionLibrary.h"
#include "Async/ParallelFor.h"

#endif

void UECSProjectileModule_Niagara::InitializeComponents(TSharedPtr<flecs::world> World)
{
#if ECSPROJECTILES_NIAGARA
	flecs::component<FECSNiagaraHandle>(*World.Get());
	flecs::component<FECSNiagaraProjectileRelationComponent>(*World.Get());
	flecs::component<FECSNiagaraHitsRelationComponent>(*World.Get());
	flecs::component<FECSNiagaraGroupManager>(*World.Get());

#endif
}

namespace FNiagaraECSSystem
{
#if ECSPROJECTILES_NIAGARA
	//Just UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector but copied here.
	//not a system? should put somewhere else I suppose
	void SetNiagaraVectorArray(UNiagaraComponent* NiagaraSystem, FName ParameterName,TArray<FVector> VectorArray)
	{
		if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
			UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(NiagaraSystem, ParameterName))
		{
			FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
			ArrayDI->FloatData = VectorArray;
			ArrayDI->MarkRenderDataDirty();
		}
	}
	
	void UpdateNiagaraPositions(flecs::entity e, const FECSNiagaraHandle& Handle, const FECSBulletTransform& Transform)
	{
		if (Handle.Component.IsValid())
		{
			Handle.Component->SetVectorParameter(Handle.LocationParameterName, Transform.CurrentTransform.GetLocation());
			Handle.Component->SetVectorParameter(Handle.PreviousLocationParameterName, Transform.PreviousTransform.GetLocation());
		}
	}


	void UpdateNiagaraPositionsArray(flecs::iter& Iter, FECSBulletTransform* BulletTransform)
	{
		//get our parent FECSNiagaraGroupProjectileHandle
		auto ParentHandleObject = Iter.term_id(2).object();
		//
		auto ParentHandle = const_cast<FECSNiagaraGroupManager*>(ParentHandleObject.get<FECSNiagaraGroupManager>());
		ParentHandle->IteratorOffset += Iter.count();

		//TODO: shrink these eventually? could set a system to run ever x seconds etc
		 ParentHandle->ParticleLocations.SetNum(ParentHandle->IteratorOffset, false);
		 ParentHandle->PreviousParticleLocations.SetNum(ParentHandle->IteratorOffset, false);
		
		 auto PositionArrayDirect = ParentHandle->ParticleLocations.GetData();
		 auto PrevPositionArrayDirect = ParentHandle->PreviousParticleLocations.GetData();
		//int Offset = ParentHandle->IteratorOffset;

		 ParallelFor(Iter.count(), [&](int32 index)
		 {
		 	auto& CurrentTransform = BulletTransform[index];
		
		 	PositionArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentTransform.CurrentTransform.GetTranslation();
		 	PrevPositionArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentTransform.PreviousTransform.GetTranslation();
		 });

	}

	void PushNiagaraPositionsArray(flecs::entity e, FECSNiagaraGroupManager& Handle)
	{
		if (Handle.Component.IsValid())
		{
			//Set current positions
			SetNiagaraVectorArray(Handle.Component.Get(),Handle.FirstParameterName,Handle.ParticleLocations);
			SetNiagaraVectorArray(Handle.Component.Get(),Handle.SecondParameterName,Handle.PreviousParticleLocations);
		}
		Handle.IteratorOffset = 0;
	}


#endif

}

void UECSProjectileModule_Niagara::InitializeSystems(TSharedPtr<flecs::world> World)
{
#if ECSPROJECTILES_NIAGARA
	World->system<FECSNiagaraHandle, FECSBulletTransform>("Write Individual Projectile Positions to Niagara")
		.kind(flecs::PreStore)
		.each(&FNiagaraECSSystem::UpdateNiagaraPositions);

	World->system<FECSBulletTransform>("Write Grouped Projectile Positions TArrays")
		.kind(flecs::PreStore)
		.term<FECSNiagaraProjectileRelationComponent>(flecs::Wildcard)
		.iter(&FNiagaraECSSystem::UpdateNiagaraPositionsArray);

	World->system<FECSNiagaraGroupManager>("Grouped Particle TArrays to Niagara")
		.kind(flecs::PreStore)
		.each(&FNiagaraECSSystem::PushNiagaraPositionsArray);
#endif
}

