// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCompoundWidget.h"

//class UGameViewClient;
/**
 * 
 */
class CAMOUFLAGENET_API STViewportWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(STViewportWidget)
		
	{}
	SLATE_END_ARGS()

public:
	STViewportWidget();
	~STViewportWidget();
	void Construct(const FArguments& InArgs);

public:
	TSharedPtr<SOverlay> mOverLay;
	//TSharedPtr<UGameViewClient> mViewClient;

	TSharedPtr<SBox> LeftPanel;
	TSharedPtr<SBox> RightPanel;
};
