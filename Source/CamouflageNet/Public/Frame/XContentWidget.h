#pragma once
#include "SCompoundWidget.h"
#include "SListView.h"
#include "Core/XCoreType.h"

class FContentDriver;

class SXContentWidget : public SCompoundWidget
{	

public:
	SLATE_BEGIN_ARGS(SXContentWidget) {}

	SLATE_ARGUMENT(TSharedPtr<FContentDriver>, ContentDriver)
	//	SLATE_ARGUMENT(TArray<TSharedPtr<FXMenuInfo>>, MenuConstructionInfos)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<ITableRow> Callback_GenerateWidget(TSharedPtr<FSingleListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void FillSource();
private:
	TSharedPtr<SListView<TSharedPtr<FSingleListItem>>> SingleList;
	TArray<TSharedPtr<FSingleListItem>> ListSource;

	TSharedPtr<FContentDriver> DataDriver;
};