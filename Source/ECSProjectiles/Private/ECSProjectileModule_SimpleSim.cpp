// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_SimpleSim.h"
#include "flecs.h"
#include "ECSProjectileDeveloperSettings.h"
#include "Async/ParallelFor.h"


void UECSProjectileModule_SimpleSim::InitializeComponents(TSharedPtr<flecs::world> World)
{
	flecs::component<FECSBulletTransform>(*World.Get());
	flecs::component<FECSBulletVelocity>(*World.Get());
	flecs::component<FECSActorEntity>(*World.Get());
	flecs::component<FECSRayCast>(*World.Get());

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
		const float DeltaTime = e.delta_time();

		UWorld* World = (UWorld*)e.world().get_context();
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
			e.set<FECSBulletHitNormal>({HitResult.ImpactNormal});
			e.remove<FECSBulletVelocity>();
			
			
		}


		//Set the final position

		tform.CurrentTransform = FTransform((DesiredDestination - tform.PreviousTransform.GetLocation()).GetSafeNormal().ToOrientationQuat(), DesiredDestination, tform.CurrentTransform.GetScale3D());
		

	}


	void ProjectileVelocityAffectsPosition(flecs::entity e, FECSBulletTransform& tform, FECSBulletVelocity& velocity)
	{
		const float DeltaTime = e.delta_time();

		UWorld* World = (UWorld*)e.world().get_context();
		if (!IsValid(World))
		{
			return;
		}
		tform.PreviousTransform = tform.CurrentTransform;
		//gonna make another system for gravity
		FVector DesiredDestination = tform.CurrentTransform.GetLocation() +  velocity.Velocity * DeltaTime;
		//Set the final position
		tform.CurrentTransform = FTransform((DesiredDestination - tform.PreviousTransform.GetLocation()).GetSafeNormal().ToOrientationQuat(), DesiredDestination, tform.CurrentTransform.GetScale3D());
	}
	
	void ECSBulletRayCast(flecs::iter Iter,FECSBulletTransform* P, FECSBulletVelocity* V,FECSRayCast* Isb)
	{
		ParallelForWithPreWork(Iter.count(),
			[&](int32 index)
			{
               
				auto& Position = P[index];
				auto& RayCastHit = Isb[index];

				auto World = (UWorld*)Iter.world().get_context();


				auto HitResult = FHitResult();
				World->LineTraceSingleByChannel(HitResult, Position.PreviousTransform.GetTranslation(), Position.CurrentTransform.GetTranslation(),
					GetDefault<UECSProjectileDeveloperSettings>()->ProjectileCollision, FCollisionQueryParams());
				RayCastHit.HitResult = HitResult;

			},
			[&]()
			{
			});
	}
	
	void QueryAsyncRayCasts(flecs::iter Iter, FECSBulletTransform* P,FECSRayCast*Rc)
	{
		for (auto i : Iter)
		{
			if(Rc[i].HitResult.bBlockingHit)
			{
				FHitResult HitResult = Rc[i].HitResult;
				auto BulletTransform = P[i];
				//kinda wish we just split position and rotation into their own comps (grumble grumble...)
				//move the entity back to here
				BulletTransform.CurrentTransform = FTransform(P->CurrentTransform.GetRotation(),HitResult.ImpactPoint,P->CurrentTransform.GetScale3D());
				Iter.entity(i).remove<FECSBulletVelocity>();
				//stop raycasting as we are stopped (I guess?)
				//really need a cool way to represent events (maybe not even in entities or components)
				Iter.entity(i).remove<FECSRayCast>();
			}
		}
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
		World->system<FECSBulletTransform, FECSBulletVelocity>("Projectile Velocity affects Position")
			.each(&FProjectileSimpleSim::ProjectileVelocityAffectsPosition);
		
		World->system<FECSBulletTransform, FECSBulletVelocity,FECSRayCast>("Bullet Collision Raycasts")
			.iter(&FProjectileSimpleSim::ECSBulletRayCast);
		World->system<FECSBulletTransform, FECSRayCast>("Query Async Raycasts for hits")
			.iter(&FProjectileSimpleSim::QueryAsyncRayCasts);
	}


	


	
}
