// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseServer.h"
#include "Net/UnrealNetwork.h"
#include "FPSBaseCharacter.h"



// Sets default values
AWeaponBaseServer::AWeaponBaseServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	WeaponMesh->SetOwnerNoSee(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetSimulatePhysics(true);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBaseServer::OnOtherBeginOverlap);

	SetReplicates(true);
}


void AWeaponBaseServer::OnOtherBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	//如果可以转化成功，说明这个枪(other actor)碰的是一个玩家
	AFPSBaseCharacter* FPSCharacter = Cast<AFPSBaseCharacter>(OtherActor);
	if(FPSCharacter)
	{
		if (!FPSCharacter->ExistServerPrimaryWeapon())
		{
			EquipWeapon();
			FPSCharacter->EquipPrimary(this);
		}
	}
}

void AWeaponBaseServer::EquipWeapon()
{
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AWeaponBaseServer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	if(GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(),0))
	{
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleFlash,
			WeaponMesh,
			TEXT("Fire_FX_Slot"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector::OneVector,
			EAttachLocation::KeepRelativeOffset,
			true,
			EPSCPoolMethod::AutoRelease,
			true
		);

		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FireSound,
			GetActorLocation(),
			GetActorRotation()
		);
	}

}

bool AWeaponBaseServer::MultiShootingEffect_Validate()
{
	return true;
}

void AWeaponBaseServer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION(AWeaponBaseServer, ClipCurrentAmmo, COND_None);
}