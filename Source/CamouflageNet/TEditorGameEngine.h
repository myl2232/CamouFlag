// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "TEditorGameEngine.generated.h"

/**
 * 
 */
UCLASS()
class CAMOUFLAGENET_API UTEditorGameEngine : public UGameEngine
{
	GENERATED_BODY()
public:
	UTEditorGameEngine();
	virtual void Init(class IEngineLoop* InEngineLoop) override;
};
