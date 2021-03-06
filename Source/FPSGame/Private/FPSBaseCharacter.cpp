// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponBaseClient.h"
#include "Components/DecalComponent.h"
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
	//????????????
	if(ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//??????(????????/????????(????/????????/??????)/????????/????????)
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);

		//??????(????????????/????????????/????????/????????/??????????UI/??????/????????)
		ClientFire();

		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo:%d"), ServerPrimaryWeapon->ClipCurrentAmmo));

	}
	//????????????????????????????
	
	//????LOG
	UE_LOG(LogTemp, Warning, TEXT("void AFPSBaseCharacter::FireWeaponPrimary()"));
}

void AFPSBaseCharacter::StopFirePrimary()
{
	
}

//
void AFPSBaseCharacter::RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	//????????
	FVector EndLocation;
	//????????????
	const FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
	//??????????
	TArray<AActor*>IgnoreArray;
	IgnoreArray.Add(this);
	//??????????????????
	FHitResult HitResult;

	
	if(ServerPrimaryWeapon)
	{
		//??????????????Endlocation????
		if (IsMoving)
		{

		}
		else
		{
			EndLocation = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
		}
	}
	bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(), 
		CameraLocation, 
		EndLocation, 
		ETraceTypeQuery::TraceTypeQuery1, 
		false,
		IgnoreArray,
		EDrawDebugTrace::None,//????debug??
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		3.0f  //??EDrawDebugTrace::????????????????????????????????????????Persistent??????????
	);

	if(HitSuccess)
	{
		//LOG:????????????????
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("HitActorName:%s"), *HitResult.Actor->GetName()));

		AFPSBaseCharacter* CanCastToFPSCharacter = Cast<AFPSBaseCharacter>(HitResult.Actor);
		if(CanCastToFPSCharacter)
		{
			//1.??????????????????
		}
		else
		{
			//2.??????????????????

			/*
			 * MakeRotFromX(FVector X)??????
			 * ????????X??????????1??0??0??????????????????????????????X????????????????????????????????X??????????X????Rotator??
			 */
			//HitResult.Normal??????
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			MultiSpawnBulletDecal(HitResult.Location, XRotator);
		}
		

	}
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

			//????????????????????????
			ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
			//????????????
		}
	}
}

void AFPSBaseCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();
	if (CurrentClientWeapon)
	{
		//??????????????????????
		CurrentClientWeapon->PlayShootAnimation();

		//FP???????????????????? ????????
		UAnimMontage* ClientArmsFireMontage = CurrentClientWeapon->ClientArmsFireAnimMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1.5f);

		//????????????
		CurrentClientWeapon->DisplayWeaponEffect();

		//????????
		FPSPlayerController->PlayerCameraShake(CurrentClientWeapon->CameraShakeClass);

		//????????UI??????????
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
		//????(??????????????????????????????)
		ServerPrimaryWeapon->MultiShootingEffect();
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

		//????(??????????????????)
		MultiShooting();
	}
	RifleLineTrace(CameraLocation, CameraRotation, IsMoving);
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo:%d"), ServerPrimaryWeapon->ClipCurrentAmmo));

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
			//??????????????????????
			ServerPrimaryWeapon->PlayShootAnimation();
			//????????????????????????????
			ServerBodyAnimBP->Montage_Play(ServerPrimaryWeapon->ServerTPBodysShootAnimMontage);
		}
	}
}
bool AFPSBaseCharacter::MultiShooting_Validate()
{
	return true;
}

void AFPSBaseCharacter::MultiSpawnBulletDecal_Implementation(FVector Location, FRotator Rotation)
{
	if(ServerPrimaryWeapon)
	{
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
			GetWorld(),
			ServerPrimaryWeapon->BulletDecalMaterial,
			FVector(8.0f, 8.0f, 8.0f),
			Location,
			Rotation,
			10.0f
		);
		if(Decal)
		{
			Decal->SetFadeScreenSize(0.001f);
		}
	}
	
}
bool AFPSBaseCharacter::MultiSpawnBulletDecal_Validate(FVector Location, FRotator Rotation)
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

//??????????????
void AFPSBaseCharacter::StartWithKindOfWeapon()
{
	//HasAuthority(): ??????????????????Authority????????????????
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
				 *??????spawn ServerWeapon??????????????????????????????????????????????????????
				 *??????????????????????????????WeaponBaseServer??????SetReplicates(true)????????????????????????????????????????????????????
				 *?????????????????????????????????????????????????????????????? ????????????????????????????????????
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