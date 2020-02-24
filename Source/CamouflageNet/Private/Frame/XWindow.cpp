#include "XWindow.h"
#include "SOverlay.h"
#include "STViewportWidget.h"

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
			//.WidthOverride(1280)
			//.HeightOverride(960)
		]
		//+ SOverlay::Slot()//叠加的模态窗口
		//.HAlign(HAlign_Fill)
		//.VAlign(VAlign_Fill)
		//[
		//	SAssignNew(WindowOverlay, SOverlay)
		//]
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