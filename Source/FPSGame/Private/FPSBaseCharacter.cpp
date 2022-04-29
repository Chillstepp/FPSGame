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
	//�ӵ��Ƿ��㹻
	if(ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//�����(���ٵ�ҩ/���߼��(ɳӥ/���ֲ�ǹ/�ѻ�ǹ)/�˺�Ӧ��/��������)
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);

		//�ͻ���(ǹ�岥�Ŷ���/�ֱ۲��Ŷ���/�������/��Ļ����/ʮ������׼UI/������/ǹ������)
		ClientFire();

		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo:%d"), ServerPrimaryWeapon->ClipCurrentAmmo));

	}
	//���ǹΪ������Ҫ����ϵͳ����
	
	//����LOG
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

			//�õ�ǹ�͸���ǹ���ӵ���Ϣ
			ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
			//�ı��ֱ۶���
		}
	}
}

void AFPSBaseCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();
	if (CurrentClientWeapon)
	{
		//ǹе��ǹ������ǹе˨��
		CurrentClientWeapon->PlayShootAnimation();

		//FP�ֱ۶���������̫�棺 �ֱ۶���
		UAnimMontage* ClientArmsFireMontage = CurrentClientWeapon->ClientArmsFireAnimMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1.5f);

		//���ſ�ǹ����
		CurrentClientWeapon->DisplayWeaponEffect();

		//��Ļ����
		FPSPlayerController->PlayerCameraShake(CurrentClientWeapon->CameraShakeClass);

		//ʮ����׼UI��ǹʱ��ɢ
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
		//�ಥ(�����ڷ��������ã�˭����˭�ಥ)
		ServerPrimaryWeapon->MultiShootingEffect();
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

		//�ಥ(�������嶯����̫��)
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
			//ǹе��ǹ������ǹе˨��
			ServerPrimaryWeapon->PlayShootAnimation();
			//ǹе��ǹ��̫�棺�����ֱ۶���
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

//��ʼ������ǹе
void AFPSBaseCharacter::StartWithKindOfWeapon()
{
	//HasAuthority(): �ж���ǰ�Ƿ�������Authority����Ȩ����������
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
				 *������spawn ServerWeaponʱ�Ǻ�����ײ�ģ�����������Ҫ�ֶ�������װ���������߼���
				 *�����ڿͻ��ˣ���������Ҫ������WeaponBaseServer�п���SetReplicates(true)������Ϊ���������ǹ��ʱ����һ�Ѹ��ͻ��˽���ͬ����
				 *�ͻ��˵�ǹ�ǲ�������ײ�ģ�����ڸ��Ƹ��ͻ��˺�����ײ���ʹ����� �ͻ��˵���ײ�߼��Ӷ��õ��ͻ��˵�ǹ��
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