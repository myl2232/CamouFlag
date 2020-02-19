// Fill out your copyright notice in the Description page of Project Settings.


#include "STViewportWidget.h"
#include "TGameViewportClient.h"
#include "SBox.h"
#include "SButton.h"
#include "TEditorGameEngine.h"

STViewportWidget::STViewportWidget()
{

}

STViewportWidget::~STViewportWidget()
{
}

void STViewportWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(mOverLay, SOverlay)
		+SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)			
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.AutoWidth()
			[
				SNew(SBorder)
				.Padding(0)
				[
					SAssignNew(LeftPanel, SBox)	
					.WidthOverride(360)
					.HeightOverride(720)
					[
						SNew(SButton)						
					]
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			.FillWidth(1.f)
			[
				SNew(SBorder)
				.Padding(0)
				[
					SAssignNew(RightPanel, SBox)
					[
						GEngine->GetGameViewportWidget().ToSharedRef()
					]					
				]
			]
		]	
	];

}