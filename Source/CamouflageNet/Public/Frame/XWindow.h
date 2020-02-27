#pragma once
#include "SCompoundWidget.h"
class SOverlay;
class SBox;
struct FXMenuInfo;

class CAMOUFLAGENET_API SXWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SXWindow) {}

	SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
	SLATE_ARGUMENT(TArray<TSharedPtr<FXMenuInfo>>, MenuConstructionInfos)

	SLATE_END_ARGS()
		
	void Construct(const FArguments& InArgs);
	void SetWindowContent(TSharedPtr<SWidget> Widget);
	void PresentModalDialog(TSharedRef<SWidget> InContentWidget);
	void DismissModalDialog();

	void PresentModalDialog(
		const FString& InTitle,
		TSharedRef<SWidget> InContentWidget,
		FSimpleDelegate OnConfirm = FSimpleDelegate(),
		FSimpleDelegate OnCancel = FSimpleDelegate(),
		FSimpleDelegate OnClose = FSimpleDelegate(),
		bool bCloseImmediately = false,
		const FString& ConfirmString = TEXT("确认"),
		const FString& CancelString = TEXT("取消"));
	/*void PresentModalDialog(const FString& InTitle, TSharedRef<SWidget> InContentWidget, 
		FSimpleDelegate OnConfirm = FSimpleDelegate(), bool bCloseImmediately = true);*/
	//interface SWidget
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

public:	
	TSharedPtr<SBox> WindowContainer;
	TSharedPtr<SOverlay> WindowOverlay;
	TSharedPtr<SWidget> mContent;
};