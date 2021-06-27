// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_SimpleSim.h"
#include "flecs.h"
#include "ECSProjectileDeveloperSettings.h"
#include "Async/ParallelFor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


void UECSProjectileModule_SimpleSim::InitializeComponents(TSharedPtr<flecs::world> World)
{
	flecs::component<FECSBulletTransform>(*World.Get());
	flecs::component<FECSBulletVelocity>(*World.Get());
	flecs::component<FECSActorEntity>(*World.Get());
	flecs::component<FECSRayCast>(*World.Get());
	flecs::component<FECSBulletGravity>(*World.Get());
	flecs::component<FECSBulletRicochet>(*World.Get());
	flecs::component<FECSBulletHit>(*World.Get());

}


namespace FProjectileSimpleSim
{
	void TransformECSPositionToActor(flecs::entity e, FECSBulletTransform& tform, FECSActorEntity& Actor)
	{
		if (Actor.Actor.IsValid())
		{
			Actor.Actor->SetActorTransform(tform.CurrentTransform);
		}
	}

	void TransformActorPostionToECS(flecs::entity e, FECSActorEntity& Actor, FECSBulletTransform& TForm)
	{
		if (Actor.Actor.IsValid())
		{
			TForm.CurrentTransform = Actor.Actor->GetActorTransform();
		}
	}

	void UpdateProjectilePositions(flecs::entity e, FECSBulletTransform& tform, FECSBulletVelocity& velocity)
	{
		UWorld* World = (UWorld*)e.world().get_context();
		const float DeltaTime = World->DeltaTimeSeconds;

		if (!IsValid(World))
		{
			return;
		}
				
		tform.PreviousTransform = tform.CurrentTransform;

		velocity.Velocity += FVector(0, 0, World->GetGravityZ() * DeltaTime);

		FVector DesiredDestination = tform.CurrentTransform.GetLocation() +  velocity.Velocity * DeltaTime;

		

		FCollisionQueryParams QueryParams;

		//Ignore any actor we are moving
		if (e.has<FECSActorEntity>())
		{
			if (AActor* TrackedActor = e.get<FECSActorEntity>()->Actor.Get())
			{
				QueryParams.AddIgnoredActor(TrackedActor);
			}
			
		}

		//Trace to see if we get stopped in this step
		FHitResult HitResult;
		if (World->LineTraceSingleByChannel(HitResult, tform.CurrentTransform.GetLocation(), DesiredDestination,
			GetDefault<UECSProjectileDeveloperSettings>()->ProjectileCollision, QueryParams))
		{
			//TODO: If we've been stopped, we need to raise an event.
			//Meanwhile, just stop us here
			DesiredDestination = HitResult.ImpactPoint;
			e.set<FECSBulletHit>({HitResult});
			e.remove<FECSBulletVelocity>();
			
			
		}


		//Set the final position

		tform.CurrentTransform = FTransform((DesiredDestination - tform.PreviousTransform.GetLocation()).GetSafeNormal().ToOrientationQuat(), DesiredDestination, tform.CurrentTransform.GetScale3D());
		

	}

	void ProjectileVelocityAffectsPosition(flecs::entity e, FECSBulletTransform& Transform, FECSBulletVelocity& Velocity)
	{
		UWorld* World = (UWorld*)e.world().get_context();
		const float DeltaTime = World->DeltaTimeSeconds;
		
		Transform.PreviousTransform = Transform.CurrentTransform;
		//gonna make another system for gravity
		FVector DesiredDestination = Transform.CurrentTransform.GetLocation() +  Velocity.Velocity * DeltaTime;
		//Set the final position
		Transform.CurrentTransform = FTransform((DesiredDestination - Transform.PreviousTransform.GetLocation()).GetSafeNormal().ToOrientationQuat(), DesiredDestination, Transform.CurrentTransform.GetScale3D());
	}
	void ProjectileGravityAffectsVelocity(flecs::entity e, FECSBulletVelocity& velocity, FECSBulletGravity& gravity)
	{
		UWorld* World = (UWorld*)e.world().get_context();
		const float DeltaTime = World->DeltaTimeSeconds;
		
		velocity.Velocity  +=  FVector(0,0,gravity.GravityZ) * DeltaTime;
	}
	
	void BulletAsyncRayCast(flecs::iter Iter,FECSBulletTransform* Transform, FECSBulletVelocity* Velocity,FECSRayCast* Raycast)
	{
		ParallelForWithPreWork(Iter.count(),
			[&](int32 index)
			{
               
				auto& Position = Transform[index];
				auto& RayCastHit = Raycast[index];

				auto World = (UWorld*)Iter.world().get_context();

				FCollisionQueryParams QueryParams;
				QueryParams.bReturnPhysicalMaterial = true;
				World->LineTraceSingleByChannel(RayCastHit.HitResult, Position.PreviousTransform.GetTranslation(), Position.CurrentTransform.GetTranslation(),
					GetDefault<UECSProjectileDeveloperSettings>()->ProjectileCollision, QueryParams);

			},
			[&]()
			{
			});
	}
	
	void QueryAsyncRayCasts(flecs::iter Iter, FECSBulletTransform* Transform,FECSRayCast*Raycast)
	{
		for (auto i : Iter)
     		{
     			if(Raycast[i].HitResult.bBlockingHit)
     			{
  
     				//kinda wish we just split position and rotation into their own comps (grumble grumble...)
     				//BulletTransform.CurrentTransform = FTransform(Transform->CurrentTransform.GetRotation(),HitResult.ImpactPoint,Transform->CurrentTransform.GetScale3D());
     				Iter.entity(i).set<FECSBulletHit>({Raycast[i].HitResult});

     			}
     		}

	}

	//less goofy way of doing this? hmm... 	
	void HandleRicochetHits(flecs::iter Iter, FECSBulletTransform* Transform,FECSBulletVelocity* Velocity,FECSBulletHit*Hit,FECSBulletRicochet*Ricochet)
	{
		for (auto i : Iter)
		{

			Iter.entity(i).remove<FECSBulletHit>();


				FHitResult HitResult = Hit[i].HitResult;

				
				auto& BulletTransform = Transform[i];
				//we should prevent too direct angle reflections etc
				auto& BulletRicochet = Ricochet[i];
				auto& BulletVecocity = Velocity[i];

				BulletVecocity.Velocity = FMath::GetReflectionVector(BulletVecocity.Velocity,HitResult.ImpactNormal);
				if (HitResult.PhysMaterial.IsValid())
				{
					BulletVecocity.Velocity *= HitResult.PhysMaterial.Get()->Restitution;
				}
				if(BulletVecocity.Velocity.Size() < BulletRicochet.MinSpeed)
				{
					Iter.entity(i).remove<FECSBulletRicochet>();
				}
				


				auto ReflectionVector = FMath::GetReflectionVector(HitResult.TraceEnd - HitResult.Location,HitResult.ImpactNormal);
				auto ReflectionPosition = ReflectionVector + HitResult.Location;				
				BulletTransform.CurrentTransform.SetTranslation(ReflectionPosition);
				BulletTransform.PreviousTransform.SetTranslation(ReflectionPosition-BulletVecocity.Velocity);
			
		}
	}
	void StopHitBullets(flecs::entity e, FECSBulletTransform& Transform , FECSRayCast& Raycast,FECSBulletVelocity& Velocity, FECSBulletHit& BulletHit)
	{
		Transform.CurrentTransform.SetTranslation(BulletHit.HitResult.ImpactPoint);
		e.remove<FECSBulletVelocity>();

	}
	
}


void UECSProjectileModule_SimpleSim::InitializeSystems(TSharedPtr<flecs::world> World)
{
	World->system<FECSBulletTransform, FECSActorEntity>("Transform ECS Position To Actor")
		.kind(flecs::PreStore)
		.each(&FProjectileSimpleSim::TransformECSPositionToActor);
	World->system<FECSActorEntity, FECSBulletTransform>("Transform Actor Position to ECS")
		.kind(flecs::PreUpdate)
		.each(&FProjectileSimpleSim::TransformActorPostionToECS);


	if(false)
	{
		World->system<FECSBulletTransform, FECSBulletVelocity>("Update Positions")
			.each(&FProjectileSimpleSim::UpdateProjectilePositions);
	}
	else
	{
		World->system<FECSBulletVelocity, FECSBulletGravity>("Projectile Gravity affects Velocity")
			.each(&FProjectileSimpleSim::ProjectileGravityAffectsVelocity);

		World->system<FECSBulletTransform, FECSBulletVelocity>("Projectile Velocity affects Position")
			.each(&FProjectileSimpleSim::ProjectileVelocityAffectsPosition);

		World->system<FECSBulletTransform, FECSBulletVelocity,FECSRayCast>("Bullet Collision Raycasts")
			.iter(&FProjectileSimpleSim::BulletAsyncRayCast);
		World->system<FECSBulletTransform, FECSRayCast>("Query Async Raycasts for hits")
			.iter(&FProjectileSimpleSim::QueryAsyncRayCasts);
		World->system<FECSBulletTransform,FECSBulletVelocity,FECSBulletHit,FECSBulletRicochet>("Handle ricochet hits")
			.iter(&FProjectileSimpleSim::HandleRicochetHits);

		World->system<FECSBulletTransform, FECSRayCast,FECSBulletVelocity,FECSBulletHit>("stop hit bullets")
			.term<FECSBulletRicochet>().oper(flecs::Not)
			.each(&FProjectileSimpleSim::StopHitBullets);


	}


	


	
}
