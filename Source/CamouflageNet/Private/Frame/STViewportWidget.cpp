// Fill out your copyright notice in the Description page of Project Settings.


#include "STViewportWidget.h"
#include "TGameViewportClient.h"
#include "SBox.h"
#include "SButton.h"
#include "Game/TEditorGameEngine.h"
#include "XSysTitleBar.h"
#include "Style/XStyle.h"
#include "XContentWidget.h"
#include "Widgets/SUserWidget.h"
#include "Data/ContentDriver.h"

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
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(titleBar, SXSysTitleBar)
				.MenuConstrctionInfos(InArgs._MenuConstructionInfos)
				.CommandsList(InArgs._CommandList)
			]

			+ SVerticalBox::Slot()
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
							SNew(SXContentWidget)
							.ContentDriver(MakeShareable(new FProjectDriver()))
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
		]	
		/*+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(0, 23.5, 23.5, 0)
		[
			SNew(SBox)
			[
				SNew(SButton)
				.ButtonStyle(&FXStyle::Get().GetWidgetStyle<FButtonStyle>("Button.CloseApplication"))
				.OnClicked(this, &STViewportWidget::OnCloseButtonClicked)
			]
		]*/
	];

}

//FReply STViewportWidget::OnCloseButtonClicked()
//{
//	TSharedPtr<SWindow> RootWindow = GEngine->GameViewport->GetWindow();
//
//	if (RootWindow.IsValid())
//	{
//		RootWindow->RequestDestroyWindow();
//	}
//
//	return FReply::Handled();
//}