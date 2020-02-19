// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TGameInstance.generated.h"

class STViewportWidget;
class SXWindow;
/**
 * 
 */
UCLASS()
class CAMOUFLAGENET_API UTGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UTGameInstance();
	virtual void Init() override;
	virtual void StartGameInstance() override;
public:
	TSharedPtr<STViewportWidget> mViewport;
	TSharedPtr<SXWindow> mWindow;
};

extern CAMOUFLAGENET_API UTGameInstance* TGI;
