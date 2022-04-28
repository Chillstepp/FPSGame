// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiFPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API AMultiFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void PlayerCameraShake(TSubclassOf<UCameraShakeBase>CameraShake);
};