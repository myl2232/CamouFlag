#include "XModalDlg.h"
#include "SBox.h"
#include "Style/XStyle.h"
#include "SBoxPanel.h"

void SXModalDlg::Construct(const FArguments& InArgs)
{
	OnConfirm = InArgs._OnConfirm;
	OnCancel = InArgs._OnCancel;
	OnImmediatelyClose = InArgs._OnImmediatelyClose;
	bCloseImmediately = InArgs._bCloseImmediately;
	ConfirmText = InArgs._ConfirmText;
	CancelText = InArgs._CancelText;

	ChildSlot
	[
		SNew(SBorder)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0)
		.BorderBackgroundColor(FXStyle::Get().GetColor("Color.99000000"))
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		[
			SNew(SBorder)
			.Padding(0)
			.BorderBackgroundColor(FXStyle::Get().GetColor("Color.FF28292B"))
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			[
				SAssignNew(VB_Dialog, SVerticalBox)

				// 标题栏
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(48)
					[
						SNew(SBorder)
						.Padding(0)
						.BorderBackgroundColor(FXStyle::Get().GetColor("Color.Gray.FF212224"))
						.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						[
							SNew(SHorizontalBox)

							// 标题
							+ SHorizontalBox::Slot()
							.Padding(16, 0, 0, 0)
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(InArgs._Title))
								.TextStyle(&FXStyle::Get().GetWidgetStyle<FTextBlockStyle>("XRText_14"))
							]

							// 关闭按钮
							+ SHorizontalBox::Slot()
							.Padding(0, 0, 5, 0)
							.FillWidth(1)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(30)
								.HeightOverride(30)
								[
									SNew(SButton)
									.ButtonStyle(&FXStyle::Get().GetWidgetStyle<FButtonStyle>("Button.CloseApplication"))
									.OnClicked_Raw(this, &SXModalDlg::OnCloseButtonClicked)
								]
							]							
						]
					]
				]
				// 内容
				+ SVerticalBox::Slot()
				.Padding(0)
				.AutoHeight()
				[
					InArgs._ContentWidget.ToSharedRef()
				]
			]
		]				
	];
}

FReply SXModalDlg::OnCloseButtonClicked()
{
	OnImmediatelyClose.ExecuteIfBound();
	return FReply::Handled();
}
FReply SXModalDlg::OnConfirmButtonClicked()
{
	OnConfirm.ExecuteIfBound();

	if (bCloseImmediately)
	{
		OnImmediatelyClose.ExecuteIfBound();
	}

	return FReply::Handled();
}
FReply SXModalDlg::OnCancelButtonClicked()
{
	OnCancel.ExecuteIfBound();
	return OnCloseButtonClicked();
}