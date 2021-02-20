// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSChargeProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AFPSChargeProjectile::AFPSChargeProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSChargeProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}


void AFPSChargeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionTemplate, GetActorLocation());

		FCollisionObjectQueryParams QueryParams;
		QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

		FCollisionShape CollShape;
		CollShape.SetSphere(500.0f);

		TArray<FOverlapResult> OutOverlaps;
		GetWorld()->OverlapMultiByObjectType(OutOverlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

		for (FOverlapResult Result : OutOverlaps)
		{
			UPrimitiveComponent* Overlap = Result.GetComponent();
			if (Overlap && Overlap->IsSimulatingPhysics())
			{
				UMaterialInstanceDynamic* MatInst = Overlap->CreateAndSetMaterialInstanceDynamic(0);
				if (MatInst)
				{
					Overlap->DestroyComponent();
					//MatInst->SetVectorParameterValue("Color", TargetColor);
				}
			}
		}
		Destroy();
	}
}