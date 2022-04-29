// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBaseCharacter.h"

#include "WeaponBaseClient.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AFPSBaseCharacter::AFPSBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#pragma region Component
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	if (PlayerCamera)
	{
		PlayerCamera->SetupAttachment(RootComponent);
		PlayerCamera->bUsePawnControlRotation = true;
	}

	FPSArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSArmMesh"));
	if(FPSArmsMesh)
	{
		FPSArmsMesh->SetupAttachment(PlayerCamera);
		FPSArmsMesh->SetOnlyOwnerSee(true);

	}
	USkeletalMeshComponent* MyMesh = GetMesh();
	MyMesh->SetOwnerNoSee(true);
	MyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MyMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
#pragma endregion Component

}
#pragma region EngineInitMethod
// Called when the game starts or when spawned
void AFPSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	StartWithKindOfWeapon();

	ClientArmsAnimBP = FPSArmsMesh->GetAnimInstance();

	ServerBodyAnimBP = GetMesh()->GetAnimInstance();

	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());

	if(FPSPlayerController)
	{
		FPSPlayerController->CreatePlayerUI();
	}
}



// Called every frame
void AFPSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFPSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis(TEXT("MoveRight"), this, &AFPSBaseCharacter::MoveRight);
	InputComponent->BindAxis(TEXT("MoveForward"), this, &AFPSBaseCharacter::MoveForward);

	InputComponent->BindAxis(TEXT("Turn"), this, &AFPSBaseCharacter::AddControllerYawInput);
	InputComponent->BindAxis(TEXT("LookUp"), this, &AFPSBaseCharacter::AddControllerPitchInput);

	InputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AFPSBaseCharacter::JumpAction);
	InputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &AFPSBaseCharacter::StopJumpAction);

	InputComponent->BindAction(TEXT("LowSpeedWalk"), EInputEvent::IE_Pressed, this, &AFPSBaseCharacter::LowSpeedWalkAction);
	InputComponent->BindAction(TEXT("LowSpeedWalk"), EInputEvent::IE_Released, this, &AFPSBaseCharacter::NormalSpeedWalkAction);

	InputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Pressed, this, &AFPSBaseCharacter::InputFirePressed);
	InputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Released, this, &AFPSBaseCharacter::InputFireReleased);
}
#pragma endregion EngineInitMethod

#pragma region Fire

void AFPSBaseCharacter::FireWeaponPrimary()
{
	//子弹是否足够
	if(ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//服务端(减少弹药/射线检测(沙鹰/三种步枪/狙击枪)/伤害应用/弹孔生成)
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);

		//客户端(枪体播放动画/手臂播放动画/设计声音/屏幕抖动/十字线瞄准UI/后坐力/枪口闪光)
		ClientFire();

		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo:%d"), ServerPrimaryWeapon->ClipCurrentAmmo));

	}
	//如果枪为连击需要连击系统开发
	
	//测试LOG
	UE_LOG(LogTemp, Warning, TEXT("void AFPSBaseCharacter::FireWeaponPrimary()"));
}

void AFPSBaseCharacter::StopFirePrimary()
{
	
}

#pragma endregion Fire

#pragma region InputEvent
void AFPSBaseCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector(), AxisValue, false);
}

void AFPSBaseCharacter::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector(), AxisValue, false);
}
void AFPSBaseCharacter::JumpAction()
{
	Jump();
}
void AFPSBaseCharacter::StopJumpAction()
{
	StopJumping();
}
void AFPSBaseCharacter::LowSpeedWalkAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 300;
	ServerLowSpeedWalkAction();

}
void AFPSBaseCharacter::NormalSpeedWalkAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
	ServerNormalSpeedWalkAction();

}
void AFPSBaseCharacter::InputFirePressed()
{
	switch (ActiveWeapon)
	{
		case EweaponType::AK47:
			{
				FireWeaponPrimary();
			}
			break;
		default:
			{
				
			}
	}
}
void AFPSBaseCharacter::InputFireReleased()
{
	switch (ActiveWeapon)
	{
		case EweaponType::AK47:
			{
				StopFirePrimary();
			}
			break;
		default:
			{
				
			}
	}
}
#pragma endregion InputEvent

#pragma region NetWorking

void AFPSBaseCharacter::ServerLowSpeedWalkAction_Implementation()
{

	GetCharacterMovement()->MaxWalkSpeed = 300;
}

bool AFPSBaseCharacter::ServerLowSpeedWalkAction_Validate()
{
	return true;
}

void AFPSBaseCharacter::ServerNormalSpeedWalkAction_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 600;
}

bool AFPSBaseCharacter::ServerNormalSpeedWalkAction_Validate()
{
	return true;
}

void AFPSBaseCharacter::ClientEquipFPArmsPrimary_Implementation()
{
	if(ServerPrimaryWeapon)
	{
		if(ClientPrimaryWeapon)
		{
			
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(
				ServerPrimaryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);


			ClientPrimaryWeapon->K2_AttachToComponent(
				FPSArmsMesh,
				TEXT("WeaponSocket"),
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true
			);

			//拿到枪就更新枪的子弹信息
			ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
			//改变手臂动画
		}
	}
}

void AFPSBaseCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();
	if (CurrentClientWeapon)
	{
		//枪械开枪动画：枪械栓动
		CurrentClientWeapon->PlayShootAnimation();

		//FP手臂动画播放蒙太奇： 手臂动作
		UAnimMontage* ClientArmsFireMontage = CurrentClientWeapon->ClientArmsFireAnimMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1.5f);

		//播放开枪声音
		CurrentClientWeapon->DisplayWeaponEffect();

		//屏幕抖动
		FPSPlayerController->PlayerCameraShake(CurrentClientWeapon->CameraShakeClass);

		//十字瞄准UI开枪时扩散
		FPSPlayerController->DoCrosshairRecoil();
	}

}

void AFPSBaseCharacter::ClientUpdateAmmoUI_Implementation(int32 ClipCurrentAmmo, int32 GunCurrentAmmo)
{
	if(FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipCurrentAmmo, GunCurrentAmmo);
	}
}


void AFPSBaseCharacter::ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if(ServerPrimaryWeapon)
	{
		//多播(必须在服务器调用，谁调用谁多播)
		ServerPrimaryWeapon->MultiShootingEffect();
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

		//多播(播放身体动画蒙太奇)
		MultiShooting();
	}

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo:%d"), ServerPrimaryWeapon->ClipCurrentAmmo));

}

bool AFPSBaseCharacter::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSBaseCharacter::MultiShooting_Implementation()
{
	if(ServerBodyAnimBP)
	{
		if(ServerPrimaryWeapon)
		{
			//枪械开枪动画：枪械栓动
			ServerPrimaryWeapon->PlayShootAnimation();
			//枪械开枪蒙太奇：人物手臂动作
			ServerBodyAnimBP->Montage_Play(ServerPrimaryWeapon->ServerTPBodysShootAnimMontage);
		}
	}
}
bool AFPSBaseCharacter::MultiShooting_Validate()
{
	return true;
}
#pragma endregion NetWorking

#pragma  region Weapon
void AFPSBaseCharacter::EquipPrimary(AWeaponBaseServer* WeaponBaseServer)
{
	if(ServerPrimaryWeapon)
	{
		
	}
	else
	{
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		ServerPrimaryWeapon->K2_AttachToComponent(
			GetMesh(), 
			TEXT("Weapon_Rifle"), 
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true
		);
		ClientEquipFPArmsPrimary();
	}
}

bool AFPSBaseCharacter::ExistServerPrimaryWeapon()
{
	if (ServerPrimaryWeapon)
	{
		return true;
	}
	return false;
}

//初始化持有枪械
void AFPSBaseCharacter::StartWithKindOfWeapon()
{
	//HasAuthority(): 判定当前是否运行在Authority（授权）服务器上
	if(HasAuthority())
	{
		PurchaseWeapon(EweaponType::AK47);
	}
}
void AFPSBaseCharacter::PurchaseWeapon(EweaponType WeaponType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (WeaponType)
	{
		case EweaponType::AK47:
			{
				UClass* BlueprintVar = StaticLoadClass(
					AWeaponBaseServer::StaticClass(), 
					nullptr, 
					TEXT("Blueprint'/Game/Blueprint/Weapon/AK47/ServerBP_AK47.ServerBP_AK47_C'")
				);
				AWeaponBaseServer* ServerWeapon= GetWorld()->SpawnActor<AWeaponBaseServer>(
					BlueprintVar,
					GetActorTransform(),
					SpawnInfo
				);

				/*
				 *服务器spawn ServerWeapon时是忽略碰撞的，所以我们需要手动调用下装备武器的逻辑。
				 *而对于客户端，我们首先要开启在WeaponBaseServer中开启SetReplicates(true)，这是为服务端有新枪的时候复制一把给客户端进行同步。
				 *客户端的枪是不忽略碰撞的，因此在复制给客户端和人物撞击就触发了 客户端的碰撞逻辑从而得到客户端的枪。
				 */
				
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
			}
			break;
		default:
			{
				
			}
	}
}
AWeaponBaseClient* AFPSBaseCharacter::GetCurrentClientFPArmsWeaponActor()
{
	switch (ActiveWeapon)
	{
		case EweaponType::AK47:
			{
				return ClientPrimaryWeapon;
			}
	}
	return nullptr;
}
#pragma  endregion Weapon