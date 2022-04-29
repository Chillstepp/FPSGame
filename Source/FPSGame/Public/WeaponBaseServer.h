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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	//枪体现在剩余子弹
	UPROPERTY(EditAnywhere)
	int32 GunCurrentAmmo;

	//弹夹现在剩余子弹,Replicated如果服务器改变，那么客户端也要改变
	UPROPERTY(EditAnywhere, Replicated)
	int32 ClipCurrentAmmo;

	//弹夹容量
	UPROPERTY(EditAnywhere)
	int32 MaxClipAmmo;

	//开枪动画蒙太奇：人物手臂动作
	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysShootAnimMontage;

	//开枪栓动：枪械动画， 蓝图中实现
	UFUNCTION(BlueprintImplementableEvent, Category = "FPGunAnimation")
	void PlayShootAnimation();

	//多播：射击效果
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validate();

	//射击距离
	UPROPERTY(EditAnywhere)
	float BulletDistance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

