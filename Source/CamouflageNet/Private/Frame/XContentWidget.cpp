#include "XContentWidget.h"
#include "Style/XStyle.h"

void SXContentWidget::Construct(const FArguments& InArgs)
{
	FillSource();

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(120.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FXStyle::Get().GetColor("Color.Gray.FF424447"))
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.Padding(FMargin(0.0f))
			[
				SAssignNew(SingleList, SListView<TSharedPtr<FSingleListItem>>)
				.ItemHeight(32.0f)
				.SelectionMode(ESelectionMode::Single)
				.ListItemsSource(&ListSource)
				.OnGenerateRow(this, &SXContentWidget::Callback_GenerateWidget)
				//.OnSelectionChanged(InArgs._OnSelectionChanged)
			]
		]
	];
}

void SXContentWidget::FillSource()
{
	ListSource.Add(MakeShareable(new FSingleListItem(TEXT("方案1"))));
	ListSource.Add(MakeShareable(new FSingleListItem(TEXT("方案2"))));
	ListSource.Add(MakeShareable(new FSingleListItem(TEXT("方案3"))));
}

TSharedRef<ITableRow> SXContentWidget::Callback_GenerateWidget(TSharedPtr<FSingleListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FSingleListItem>>, OwnerTable)
		.Padding(FMargin(16.0f, 10.0f, 0.0f, 10.0f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Name))
			.TextStyle(&FXStyle::Get().GetWidgetStyle<FTextBlockStyle>("XRText_12"))
		];
}