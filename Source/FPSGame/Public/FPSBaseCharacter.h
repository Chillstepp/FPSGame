// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Camera/CameraComponent.h>
#include "WeaponBaseServer.h"
#include "MultiFPSPlayerController.h"
#include "FPSBaseCharacter.generated.h"

class AWeaponBaseClient;
class AWeaponBaseServer;

UCLASS()
class FPSGAME_API AFPSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSBaseCharacter();
private:
#pragma region Component
	//�����meta�Ǹ�����Ȼ����private��Ա�����ǿ��Ա����ʵ�(��ͼ�õ�)
	//��Ϊʲô�����Ҫ����private���أ���Ϊ���ǲ�ϣ������������Կ�������Ĵ���
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* PlayerCamera;
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* FPSArmsMesh;

	UAnimInstance* ClientArmsAnimBP;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AMultiFPSPlayerController* FPSPlayerController;
#pragma endregion Component

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region InputEvent
	void MoveRight(float AxisValue);
	void MoveForward(float AxisValue);

	void JumpAction();
	void StopJumpAction();

	void LowSpeedWalkAction();
	void NormalSpeedWalkAction();

	void InputFirePressed();
	void InputFireReleased();
#pragma endregion InputEvent

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#pragma region Fire
public:
	void FireWeaponPrimary();
	void StopFirePrimary();
#pragma endregion Fire

#pragma region NetWorking
public:
	//��������ִ�У��ɿ�����(���ᶪ��/����)���������ɵ������ֲ���Ҫ�����ǿ��Բ��ÿɿ�����
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalkAction();
	void ServerLowSpeedWalkAction_Implementation();
	bool ServerLowSpeedWalkAction_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalkAction();
	void ServerNormalSpeedWalkAction_Implementation();
	bool ServerNormalSpeedWalkAction_Validate();

	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPrimary();
	void ClientEquipFPArmsPrimary_Implementation();

	UFUNCTION(Client, Reliable)
	void ClientFire();
	void ClientFire_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

#pragma endregion NetWorking

#pragma  region Weapon
public:
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);
	bool ExistServerPrimaryWeapon();
private:
	UPROPERTY(EditAnywhere)
	EweaponType ActiveWeapon;

	UPROPERTY(meta=(AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	void StartWithKindOfWeapon();

	void PurchaseWeapon(EweaponType WeaponType);

	AWeaponBaseClient* GetCurrentClientFPArmsWeaponActor();

#pragma  endregion Weapon
};
