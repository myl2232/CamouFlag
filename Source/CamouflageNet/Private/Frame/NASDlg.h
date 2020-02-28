#pragma once
#include "SCompoundWidget.h"
#include "SListView.h"
#include "Core/XCoreType.h"

class FContentDriver;

//需求评估
class CAMOUFLAGENET_API SNASDlg : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNASDlg) {}
	SLATE_ARGUMENT(TSharedPtr<FContentDriver>, ContentDriver)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void OnConfirmClick();
	void OnCancelClick();
	void OnCloseClick();

private:
	TSharedRef<ITableRow> Callback_GenerateWidget(TSharedPtr<FSingleListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void FillSource();

	TSharedRef<SWidget> GenerateComboItem(TSharedPtr<FXKeyValue> InItem, int32 InType);
	void HandleComboItemChanged(TSharedPtr<FXKeyValue> NewSelection, ESelectInfo::Type SelectInfo, int32 InType);
	FText GetCurrentComboText(int32 InType) const;
	TSharedRef<SWidget> MakeNASContent();

public:
	TSharedPtr<SWidget> mParent;
	TSharedPtr<SWidget> mEquipCombo;
private:
	TSharedPtr<FContentDriver> DataDriver;

	TSharedPtr<SListView<TSharedPtr<FSingleListItem>>> SingleList;
	TArray<TSharedPtr<FSingleListItem>> ListSource;
	//combo
	TArray< TSharedPtr<FXKeyValue> > GroupTypeList;
	TSharedPtr<FXKeyValue> CurrentGroupValue = MakeShareable(new FXKeyValue(-1, TEXT("装备列表")));
};