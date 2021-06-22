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

	//These are temporary singletons hopefully turned into fully fledged parent entities
	//or maybe even just some manager actor with a TMap?
#endif
}

namespace FNiagaraECSSystem
{
#if ECSPROJECTILES_NIAGARA
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
		//get the size of the iterator to grow the arrays
		int ParticleIter = Iter.count();

		auto World = (UWorld*)Iter.world().get_context();

		//get our parent FECSNiagaraGroupProjectileHandle
		auto ParentHandle = *Iter.term<FECSNiagaraGroupProjectileHandle>(2);

		ParentHandle.IteratorOffset += Iter.count();

		//TODO: shrink these eventually? could set a system to run ever x seconds etc

		 ParentHandle.ParticleLocations.SetNum(ParticleIter, false);
		 ParentHandle.PreviousParticleLocations.SetNum(ParticleIter, false);
		
		
		 auto VectorArrayDirect = ParentHandle.ParticleLocations.GetData();
		 auto PrevVectorArrayDirect = ParentHandle.PreviousParticleLocations.GetData();
		int Offset = ParentHandle.IteratorOffset;

		 ParallelFor(Iter.count(), [&](int32 index)
		 {
		 	auto& CurrentTransform = BulletTransform[index];
		
		 	VectorArrayDirect[index + Offset - Iter.count()] = CurrentTransform.CurrentTransform.GetTranslation();
		 	PrevVectorArrayDirect[index + Offset - Iter.count()] = CurrentTransform.PreviousTransform.GetTranslation();
		 });

		if (ParentHandle.Component.IsValid())
		{
			//These are both just UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector
			//Set current positions
			if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
				UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(ParentHandle.Component.Get(), ParentHandle.LocationsParameterName))
			{
				FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
				ArrayDI->FloatData = ParentHandle.ParticleLocations;
				ArrayDI->MarkRenderDataDirty();
			}
			//Set previous positions
			if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
				UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(ParentHandle.Component.Get(), ParentHandle.PreviousLocationsParameterName))
			{
				FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
				ArrayDI->FloatData = ParentHandle.PreviousParticleLocations;
				ArrayDI->MarkRenderDataDirty();
			}
		}
	}

	void UpdateNiagaraHitsArray(flecs::iter& Iter, FECSBulletTransform* BulletTransform,FECSBulletHitNormal* BulletHitNormal)
	{
		//get the size of the iterator to grow the arrays

		auto World = (UWorld*)Iter.world().get_context();
		
		auto Handle = *Iter.world().get<FECSNiagaraGroupHitHandle>();
		Handle.IteratorOffset += Iter.count();
		
		if (Handle.Component.IsValid())
		{
			//TODO: shrink these eventually? could set a system to run ever x seconds etc

			Handle.HitLocations.SetNum(Handle.IteratorOffset, false);
			Handle.HitNormals.SetNum(Handle.IteratorOffset, false);
		
		
			auto VectorArrayDirect = Handle.HitLocations.GetData();
			auto NormalArrayDirect = Handle.HitNormals.GetData();
			int Offset = Handle.IteratorOffset;
			ParallelFor(Iter.count(), [&](int32 index)
			{
				auto& CurrentTransform = BulletTransform[index];
			
				VectorArrayDirect[index + Offset - Iter.count()] = CurrentTransform.CurrentTransform.GetTranslation();
				NormalArrayDirect[index + Offset - Iter.count()] = BulletHitNormal->Normal;
			});
			

			//These are both just UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector
			//Set current positions
			if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
				UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(Handle.Component.Get(), Handle.LocationsParameterName))
			{
				FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
				ArrayDI->FloatData = Handle.HitLocations;
				ArrayDI->MarkRenderDataDirty();
			}
			//Set previous positions
			if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
				UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(Handle.Component.Get(), Handle.HitNormalsParameterName))
			{
				FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
				ArrayDI->FloatData = Handle.HitNormals;
				ArrayDI->MarkRenderDataDirty();
			}
		}
	}


	void PushNiagaraPositionsArray(flecs::entity e, const FECSNiagaraGroupProjectileHandle& Handle)
	{
		if (Handle.Component.IsValid())
		{
			//These are both just UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector
			//Set current positions
			if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
				UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(Handle.Component.Get(), Handle.LocationsParameterName))
			{
				FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
				ArrayDI->FloatData = Handle.ParticleLocations;
				ArrayDI->MarkRenderDataDirty();
			}
			//Set previous positions
			if (UNiagaraDataInterfaceArrayFloat3* ArrayDI =
				UNiagaraFunctionLibrary::GetDataInterface<UNiagaraDataInterfaceArrayFloat3>(Handle.Component.Get(), Handle.PreviousLocationsParameterName))
			{
				FRWScopeLock WriteLock(ArrayDI->ArrayRWGuard, SLT_Write);
				ArrayDI->FloatData = Handle.PreviousParticleLocations;
				ArrayDI->MarkRenderDataDirty();
			}
		}
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

	// World->system<FECSNiagaraGroupProjectileHandle>("Grouped Projectile Positions TArrays to Niagara")
	// 	.kind(flecs::PreStore)
	// 	.each(&FNiagaraECSSystem::PushNiagaraPositionsArray);
	
	// World->system<FECSBulletTransform,FECSBulletHitNormal>("Write Projectile Hits Array to Niagara")
	// 	.kind(flecs::PreStore)
	// 	.iter(&FNiagaraECSSystem::UpdateNiagaraHitsArray);
#endif
}

