#include "NASDlg.h"
#include "SBox.h"
#include "SOverlay.h"
#include "TGameInstance.h"
#include "XWindow.h"
#include "SEditableTextBox.h"
#include "Style/XStyle.h"
#include "SComboBox.h"

UTGameInstance* TGI = nullptr;

void SNASDlg::Construct(const FArguments& InArgs)
{
	mParent = TGI->mWindow->WindowOverlay;//InArgs._ParentWidget;
	FillSource();

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(1280)
		.HeightOverride(960)
		[			
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)
				
				+SVerticalBox::Slot()
				.Padding(1, 1, 1, 5)
				.AutoHeight()
				[
					SAssignNew(mEquipCombo, SComboBox< TSharedPtr<FXKeyValue> >)
					.ItemStyle(&FXStyle::Get().GetWidgetStyle<FTableRowStyle>("TableRow.PropertyCombobox"))
					.ComboBoxStyle(FXStyle::Get(), "ComboBox")
					.HasDownArrow(true)
					.ForegroundColor(FLinearColor(FColor(0XFFFFFFFF)))
					.OptionsSource(&GroupTypeList)
					.OnGenerateWidget(this, &SNASDlg::GenerateComboItem, 0)
					.OnSelectionChanged(this, &SNASDlg::HandleComboItemChanged, 0)
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.Text(this, &SNASDlg::GetCurrentComboText, 0)
						.TextStyle(&FXStyle::Get().GetWidgetStyle<FTextBlockStyle>("XRText_12"))
					]				
				]
				
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()					
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("装备数量")))
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SEditableTextBox)
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("伪装需求度")))
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SEditableTextBox)
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("重要程度")))
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SEditableTextBox)
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("装备数量")))
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SEditableTextBox)
					]
				]
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
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
					.OnGenerateRow(this, &SNASDlg::Callback_GenerateWidget)
					//.OnSelectionChanged(InArgs._OnSelectionChanged)
					]
				]
			]

		]
	];
}

TSharedRef<SWidget> SNASDlg::GenerateComboItem(TSharedPtr<FXKeyValue> InItem, int32 InType)
{
	return
		SNew(STextBlock)
		.Text(FText::FromString(InItem->Value))
		.ColorAndOpacity(FLinearColor::White)
		.TextStyle(FXStyle::Get(), "XRText_12");
}

void SNASDlg::HandleComboItemChanged(TSharedPtr<FXKeyValue> NewSelection, ESelectInfo::Type SelectInfo, int32 InType)
{
	if (!NewSelection.IsValid())
	{
		return;
	}

}

FText SNASDlg::GetCurrentComboText(int32 InType) const
{
	TSharedPtr<FXKeyValue> ReturnValue = CurrentGroupValue;
	return FText::FromString(ReturnValue->Value);
}

void SNASDlg::FillSource()
{
	ListSource.Add(MakeShareable(new FSingleListItem(TEXT("装备1"))));
	ListSource.Add(MakeShareable(new FSingleListItem(TEXT("装备2"))));
	ListSource.Add(MakeShareable(new FSingleListItem(TEXT("装备3"))));

	GroupTypeList.Add(MakeShareable(new FXKeyValue(1,TEXT("装备1"))));
	GroupTypeList.Add(MakeShareable(new FXKeyValue(2, TEXT("装备2"))));
	GroupTypeList.Add(MakeShareable(new FXKeyValue(3, TEXT("装备3"))));
}

TSharedRef<ITableRow> SNASDlg::Callback_GenerateWidget(TSharedPtr<FSingleListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FSingleListItem>>, OwnerTable)
		.Padding(FMargin(3.0f,3.0f, 0.0f, 5.0f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->Name))
			.TextStyle(&FXStyle::Get().GetWidgetStyle<FTextBlockStyle>("XRText_12"))
		];
}
void SNASDlg::OnConfirmClick()
{
	//mParent->SetVisibility(EVisibility::Hidden);
}

void SNASDlg::OnCancelClick()
{
	//mParent->SetVisibility(EVisibility::Hidden);
}

void SNASDlg::OnCloseClick()
{
	//mParent->SetVisibility(EVisibility::Hidden);
}