#pragma once
#include "SCompoundWidget.h"
class SVerticalBox;

class CAMOUFLAGENET_API SXModalDlg : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SXModalDlg) 
		: _bCloseImmediately(true)
		, _ConfirmText(TEXT("确认"))
		, _CancelText(TEXT("取消")) 
	{}
	/** 标题 */
	SLATE_ARGUMENT(FString, Title)
	/*内容*/
	SLATE_ARGUMENT(TSharedPtr<SWidget>, ContentWidget)
	/** 是否点击完确定按钮立即关闭对话框 */
	SLATE_ARGUMENT(bool, bCloseImmediately)
	/** 确认按钮的文字 */
	SLATE_ARGUMENT(FString, ConfirmText)
	/** 取消按钮的文字 */
	SLATE_ARGUMENT(FString, CancelText)
	/** 确定 */
	SLATE_EVENT(FSimpleDelegate, OnConfirm)
	/**取消 */
	SLATE_EVENT(FSimpleDelegate, OnCancel)
	/** 立即关闭 */
	SLATE_EVENT(FSimpleDelegate, OnImmediatelyClose)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnConfirmButtonClicked();
	FReply OnCloseButtonClicked();
	FReply OnCancelButtonClicked();

private:
	TSharedPtr<SVerticalBox> VB_Dialog;
	bool bCloseImmediately;
	FSimpleDelegate OnConfirm;
	FSimpleDelegate OnCancel;
	FSimpleDelegate OnImmediatelyClose;
	FString ConfirmText;
	FString CancelText;
};