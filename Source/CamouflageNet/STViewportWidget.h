// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCompoundWidget.h"
#include "Commands.h"

//class UGameViewClient;
/**
 * 
 */
class SXSysTitleBar;
struct FXMenuInfo;

class CAMOUFLAGENET_API STViewportWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(STViewportWidget)	{}
	SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
	SLATE_ARGUMENT(TArray<TSharedPtr<FXMenuInfo>>, MenuConstructionInfos)
	SLATE_END_ARGS()

public:
	STViewportWidget();
	~STViewportWidget();
	void Construct(const FArguments& InArgs);

public:
	TSharedPtr<SOverlay> mOverLay;
	//TSharedPtr<UGameViewClient> mViewClient;
	TSharedPtr<SXSysTitleBar> titleBar;
	TSharedPtr<SBox> LeftPanel;
	TSharedPtr<SBox> RightPanel;
};
