// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBaseCharacter.h"
#include "WeaponBaseServer.h"


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
		EquipWeapon();
		//玩家逻辑
		FPSCharacter->EquipPrimary(this);
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
