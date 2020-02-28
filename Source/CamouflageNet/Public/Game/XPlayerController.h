// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CAMOUFLAGENET_API AXPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AXPlayerController();
	virtual void BeginPlay() override;
};
