#pragma once
#include "SCompoundWidget.h"
class SVerticalBox;

class CAMOUFLAGENET_API SXModalDlg : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SXModalDlg) 
		: _bCloseImmediately(true)
		, _ConfirmText(TEXT("ȷ��"))
		, _CancelText(TEXT("ȡ��")) 
	{}
	/** ���� */
	SLATE_ARGUMENT(FString, Title)
	/*����*/
	SLATE_ARGUMENT(TSharedPtr<SWidget>, ContentWidget)
	/** �Ƿ�����ȷ����ť�����رնԻ��� */
	SLATE_ARGUMENT(bool, bCloseImmediately)
	/** ȷ�ϰ�ť������ */
	SLATE_ARGUMENT(FString, ConfirmText)
	/** ȡ����ť������ */
	SLATE_ARGUMENT(FString, CancelText)
	/** ȷ�� */
	SLATE_EVENT(FSimpleDelegate, OnConfirm)
	/**ȡ�� */
	SLATE_EVENT(FSimpleDelegate, OnCancel)
	/** �����ر� */
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