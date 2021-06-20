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
	flecs::component<FECSNiagaraGroupHandle>(*World.Get());

	//This is temporary (yeah, I know...)
	World.Get()->add<FECSNiagaraGroupHandle>();

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
		 //TODO: shrink these eventually? could set a system to run ever x seconds etc
		auto World = (UWorld*)Iter.world().get_context();
		
		auto Handle = *Iter.world().get<FECSNiagaraGroupHandle>();
		 Handle.ParticleLocations.SetNum(ParticleIter, false);
		 Handle.PreviousParticleLocations.SetNum(ParticleIter, false);
		
		
		 auto VectorArrayDirect = Handle.ParticleLocations.GetData();
		 auto PrevVectorArrayDirect = Handle.PreviousParticleLocations.GetData();
		 ParallelFor(Iter.count(), [&](int32 index)
		 {
		 	auto& CurrentTransform = BulletTransform[index];
		
		 	VectorArrayDirect[index] = CurrentTransform.CurrentTransform.GetTranslation();
		 	PrevVectorArrayDirect[index] = CurrentTransform.PreviousTransform.GetTranslation();
		 });
		
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
	
	World->system<FECSBulletTransform>("Write Projectile Positions Array to Niagara")
		.kind(flecs::PreStore)
		.iter(&FNiagaraECSSystem::UpdateNiagaraPositionsArray);
#endif
}

