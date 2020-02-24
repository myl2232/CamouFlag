// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "TGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class CAMOUFLAGENET_API UTGameViewportClient : public UGameViewportClient
{
	GENERATED_UCLASS_BODY()

public:
	~UTGameViewportClient();
	virtual void Tick(float DeltaTime) override;
};
