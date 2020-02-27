#include "XWindow.h"
#include "SOverlay.h"
#include "STViewportWidget.h"
#include "XModalDlg.h"

void SXWindow::Construct(const FArguments& InArgs)
{	
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(WindowContainer, SBox)
			.WidthOverride(1280)
			.HeightOverride(960)
		]
		+ SOverlay::Slot()//叠加的模态窗口
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SAssignNew(WindowOverlay, SOverlay)
		]
	];

	SetWindowContent(SAssignNew(mContent, STViewportWidget)
		.MenuConstructionInfos(InArgs._MenuConstructionInfos)
		.CommandList(InArgs._CommandList));
}

void SXWindow::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//Super::OnMouseEnter(MyGeometry, MouseEvent);
}

void SXWindow::SetWindowContent(TSharedPtr<SWidget> Widget)
{
	WindowContainer->SetContent(Widget.ToSharedRef());
}

void SXWindow::PresentModalDialog(TSharedRef<SWidget> InContentWidget)
{
	WindowOverlay->AddSlot()
		[
			InContentWidget
		];
}

void SXWindow::DismissModalDialog()
{
	if (WindowOverlay->GetNumWidgets() > 0)
	{
		WindowOverlay->RemoveSlot();
	}
}

void SXWindow::PresentModalDialog(
	const FString& InTitle,
	TSharedRef<SWidget> InContentWidget,
	FSimpleDelegate OnConfirm,
	FSimpleDelegate OnCancel,
	FSimpleDelegate OnClose,
	bool bCloseImmediately,
	const FString& ConfirmString ,
	const FString& CancelString )
{
	TSharedPtr<SXModalDlg> ModalDialog =
		SNew(SXModalDlg)
		.Title(InTitle)
		.ContentWidget(InContentWidget)
		.OnConfirm(OnConfirm)
		.OnCancel(OnCancel)
		.OnImmediatelyClose(this, &SXWindow::DismissModalDialog)
		.bCloseImmediately(bCloseImmediately)
		.ConfirmText(ConfirmString)
		.CancelText(CancelString);

	PresentModalDialog(ModalDialog.ToSharedRef());
}

//void SXWindow::PresentModalDialog(const FString& InTitle, TSharedRef<SWidget> InContentWidget,
//	FSimpleDelegate OnConfirm , bool bCloseImmediately)
//{
//	TSharedPtr<SXModalDlg> ModalDialog =
//		SNew(SXModalDlg)
//		.Title(InTitle)
//		.ContentWidget(InContentWidget)
//		.OnConfirm(OnConfirm)
//		.OnCancel(FSimpleDelegate())
//		.OnImmediatelyClose(FSimpleDelegate())
//		.bCloseImmediately(bCloseImmediately)
//		.ConfirmText(TEXT("确认"))
//		.CancelText(TEXT("取消"));
//
//	PresentModalDialog(ModalDialog.ToSharedRef());
//}