#include "XSysTitleBar.h"
#include "Style/XStyle.h"

void SXSysTitleBar::Construct(const FArguments& InArgs)
{
	MenuConstrctionInfos = InArgs._MenuConstrctionInfos;
	GlobalCommandsList = InArgs._CommandsList;

	ChildSlot
		[
			SAssignNew(TitleBar, SBox)
			.HeightOverride(TitleBarHeight)
			.Padding(FMargin(16, 0))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
				]
				
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

					// 菜单
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						CreateMenu()
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)

						// 最小化按钮
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonStyle(&FXStyle::Get().GetWidgetStyle<FButtonStyle>("Button.Minimize"))
							.OnClicked(this, &SXSysTitleBar::OnMinimizeButtonClicked)
						]

						// 关闭按钮
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.ButtonStyle(&FXStyle::Get().GetWidgetStyle<FButtonStyle>("Button.CloseApplication"))
							.OnClicked(this, &SXSysTitleBar::OnCloseButtonClicked)
						]
					]
				]
			]
		];
}

TSharedRef<SWidget> SXSysTitleBar::CreateMenu()
{
	const TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	FMenuBarBuilder MenuBuilder(GlobalCommandsList, MenuExtender);
	{
		for (auto MInfo : MenuConstrctionInfos)
		{
			MenuBuilder.AddPullDownMenu(
				MInfo->M_Label,
				MInfo->M_Tip,
				FNewMenuDelegate::CreateStatic(&SXSysTitleBar::FillMenu, MenuExtender, MInfo)
			);
		}
	}

	// Create the menu bar!
	return
		SAssignNew(MenuBar, SBox)
		.HeightOverride(24)
		.VAlign(VAlign_Bottom)
		[
			MenuBuilder.MakeWidget()
		];
}

void SXSysTitleBar::FillMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender, TSharedPtr<FXMenuInfo> InMenuInfo)
{
	for (auto & SectionInfo : InMenuInfo->SectionInfos)
	{
		MenuBuilder.BeginSection(SectionInfo.ExtensionHook, SectionInfo.HeadingText);
		{
			for (auto Command : SectionInfo.ActionList)
			{
				MenuBuilder.AddMenuEntry(Command);
			}
		}
		MenuBuilder.EndSection();
	}
}

/** 最小化窗口 */
FReply SXSysTitleBar::OnMinimizeButtonClicked()
{
	TSharedPtr<SWindow> RootWindow = GEngine->GameViewport->GetWindow();

	if (RootWindow.IsValid())
	{
		TSharedPtr<FGenericWindow> NativeWindow = RootWindow->GetNativeWindow();

		if (NativeWindow.IsValid())
		{
			NativeWindow->Minimize();
		}
	}

	return FReply::Handled();
}

/** 关闭窗口 */
FReply SXSysTitleBar::OnCloseButtonClicked()
{
	//FXRFrameModule::Get().OnApplicationClosed.ExecuteIfBound();
	TSharedPtr<SWindow> RootWindow = GEngine->GameViewport->GetWindow();	
	if (RootWindow.IsValid())
	{
		RootWindow->RequestDestroyWindow();
	}			
	return FReply::Handled();
}