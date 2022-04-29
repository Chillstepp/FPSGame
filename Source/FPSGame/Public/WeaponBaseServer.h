// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "WeaponBaseClient.h"
#include "Kismet/GameplayStatics.h"
#include "WeaponBaseServer.generated.h"


UENUM()
enum class EweaponType : uint8
{
	AK47 UMETA(DisplayName = "AK47"),
	DesertEagle UMETA(Display = "DesertEagle")
};

UCLASS()
class FPSGAME_API AWeaponBaseServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBaseServer();

	//武器类型
	UPROPERTY(EditAnywhere)
	EweaponType KindOfWeapon;

	//骨骼网格体组件: 武器
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	//碰撞体
	UPROPERTY(EditAnywhere)
	USphereComponent* SphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBaseClient>ClientWeaponBaseBPClass;

	UFUNCTION()
	void OnOtherBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EquipWeapon();

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validate();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
