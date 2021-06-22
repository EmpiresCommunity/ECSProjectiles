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
	flecs::component<FECSNiagaraGroupProjectileHandle>(*World.Get());
	flecs::component<FECSNiagaraGroupHitHandle>(*World.Get());
#endif
}

namespace FNiagaraECSSystem
{
#if ECSPROJECTILES_NIAGARA
	//Just UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector but copied here.
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
		auto World = (UWorld*)Iter.world().get_context();
		//get our parent FECSNiagaraGroupProjectileHandle
		auto ParentHandle = Iter.term<FECSNiagaraGroupProjectileHandle>(2);
		
		ParentHandle->IteratorOffset += Iter.count();

		//TODO: shrink these eventually? could set a system to run ever x seconds etc
		 ParentHandle->ParticleLocations.SetNum(ParentHandle->IteratorOffset, false);
		 ParentHandle->PreviousParticleLocations.SetNum(ParentHandle->IteratorOffset, false);
		
		 auto VectorArrayDirect = ParentHandle->ParticleLocations.GetData();
		 auto PrevVectorArrayDirect = ParentHandle->PreviousParticleLocations.GetData();
		//int Offset = ParentHandle->IteratorOffset;

		 ParallelFor(Iter.count(), [&](int32 index)
		 {
		 	auto& CurrentTransform = BulletTransform[index];
		
		 	VectorArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentTransform.CurrentTransform.GetTranslation();
		 	PrevVectorArrayDirect[index + ParentHandle->IteratorOffset - Iter.count()] = CurrentTransform.PreviousTransform.GetTranslation();
		 });

	}

	void UpdateNiagaraHitsArray(flecs::iter& Iter, FECSBulletTransform* BulletTransform,FECSBulletHitNormal* BulletHitNormal)
	{
	
	}


	void PushNiagaraPositionsArray(flecs::entity e, FECSNiagaraGroupProjectileHandle& Handle)
	{
		UE_LOG(LogTemp,Warning,TEXT("PushNiagaraPositionsArray count: %i:"),Handle.IteratorOffset)

		if (Handle.Component.IsValid())
		{
			//Set current positions
			SetNiagaraVectorArray(Handle.Component.Get(),Handle.LocationsParameterName,Handle.ParticleLocations);
			SetNiagaraVectorArray(Handle.Component.Get(),Handle.PreviousLocationsParameterName,Handle.PreviousParticleLocations);
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
		.term<FECSNiagaraGroupProjectileHandle>()
			.superset(flecs::Childof)
			//.oper(flecs::And)
		.iter(&FNiagaraECSSystem::UpdateNiagaraPositionsArray);

	World->system<FECSNiagaraGroupProjectileHandle>("Grouped Projectile Positions TArrays to Niagara")
		.kind(flecs::PreStore)
		.each(&FNiagaraECSSystem::PushNiagaraPositionsArray);
	
	World->system<FECSBulletTransform,FECSBulletHitNormal>("Write Projectile Hits Array to Niagara")
		.kind(flecs::PreStore)
		.iter(&FNiagaraECSSystem::UpdateNiagaraHitsArray);
#endif
}

