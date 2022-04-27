// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Camera/CameraComponent.h>
#include "WeaponBaseServer.h"
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
	//这里的meta是告诉虽然我是private成员，但是可以被访问到(蓝图拿到)
	//那为什么这里非要放在private中呢？因为我们不希望其他代码可以看到这个的存在
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* PlayerCamera;
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* FPSArmsMesh;
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
#pragma endregion InputEvent
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
#pragma region NetWorking
	//服务器上执行，可靠传输(不会丢包/错码)，比如生成弹孔这种不重要的我们可以不用可靠传输
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

#pragma endregion NetWorking



#pragma  region Weapon
public:
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);
private:
	UPROPERTY(meta=(AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

#pragma  endregion Weapon
};
