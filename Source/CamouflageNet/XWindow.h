#pragma once
#include "SCompoundWidget.h"
class SOverlay;
class SBox;

class SXWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SXWindow) {}
	SLATE_END_ARGS()
		
	void Construct(const FArguments& InArgs);
	void SetWindowContent(TSharedPtr<SWidget> Widget);
	void PresentModalDialog(TSharedRef<SWidget> InContentWidget);
	void DismissModalDialog();

	//interface SWidget
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

public:	
	TSharedPtr<SBox> WindowContainer;
	TSharedPtr<SOverlay> WindowOverlay;
	TSharedPtr<SWidget> mContent;
};