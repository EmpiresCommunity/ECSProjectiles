// Fill out your copyright notice in the Description page of Project Settings.


#include "ECSProjectileModule_SimpleSim.h"
#include "flecs.h"
#include "ECSProjectileDeveloperSettings.h"



void UECSProjectileModule_SimpleSim::InitializeComponents(TSharedPtr<flecs::world> World)
{
	flecs::component<FECSBulletTransform>(*World.Get());
	flecs::component<FECSBulletVelocity>(*World.Get());
	flecs::component<FECSActorEntity>(*World.Get());
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
			e.remove<FECSBulletVelocity>();
			
			
		}


		//Set the final position

		tform.CurrentTransform = FTransform((DesiredDestination - tform.PreviousTransform.GetLocation()).GetSafeNormal().ToOrientationQuat(), DesiredDestination, tform.CurrentTransform.GetScale3D());
		

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
	World->system<FECSBulletTransform, FECSBulletVelocity>("Update Positions")
		.each(&FProjectileSimpleSim::UpdateProjectilePositions);
}
