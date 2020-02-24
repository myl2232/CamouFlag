// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TGameInstance.generated.h"

class STViewportWidget;
class SXWindow;
struct FXMenuInfo;
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
	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	UFUNCTION(BlueprintCallable)
	void OnPostloadAsset();

protected:
	void InitUICommands();
	void InitStartUp();
	void OnPostLoadMap(UWorld* world);
public:
	TSharedPtr<STViewportWidget> mViewport;
	TSharedPtr<SXWindow> mWindow;
	TArray<TSharedPtr<FXMenuInfo>> MenuInfoArray;
};

extern CAMOUFLAGENET_API UTGameInstance* TGI;
