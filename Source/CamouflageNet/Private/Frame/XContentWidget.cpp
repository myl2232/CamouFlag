#include "XContentWidget.h"
#include "Style/XStyle.h"
#include "Data/ContentDriver.h"

#define LOCTEXT_NAMESPACE "XContentWidget"

void SXContentWidget::Construct(const FArguments& InArgs)
{
	DataDriver = InArgs._ContentDriver;

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
	check(DataDriver != nullptr);
	DataDriver->FillSource(ListSource);
}

TSharedRef<ITableRow> SXContentWidget::Callback_GenerateWidget(TSharedPtr<FSingleListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FSingleListItem>>, OwnerTable)
		.Padding(FMargin(16.0f, 10.0f, 0.0f, 10.0f))
		[
			SNew(STextBlock)
			.Text(Item->Name)
			.TextStyle(&FXStyle::Get().GetWidgetStyle<FTextBlockStyle>("XRText_12"))
		];
}

#undef LOCTEXT_NAMESPACE