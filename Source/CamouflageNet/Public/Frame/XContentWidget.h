#pragma once
#include "SCompoundWidget.h"
#include "SListView.h"

class SXContentWidget : public SCompoundWidget
{	
	struct FSingleListItem
	{
	public:
		FSingleListItem(FString name) :Name(name) {}
		FString Name;
		FString Path;
	};
public:
	SLATE_BEGIN_ARGS(SXContentWidget) {}

	//SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandList)
	//	SLATE_ARGUMENT(TArray<TSharedPtr<FXMenuInfo>>, MenuConstructionInfos)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<ITableRow> Callback_GenerateWidget(TSharedPtr<FSingleListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void FillSource();
private:
	TSharedPtr<SListView<TSharedPtr<FSingleListItem>>> SingleList;
	TArray<TSharedPtr<FSingleListItem>> ListSource;
};