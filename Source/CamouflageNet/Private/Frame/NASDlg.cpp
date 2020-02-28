#include "NASDlg.h"
#include "SBox.h"
#include "SOverlay.h"
#include "TGameInstance.h"
#include "XWindow.h"
#include "SEditableTextBox.h"
#include "Style/XStyle.h"
#include "SComboBox.h"
#include "SButton.h"
#include "Data/ContentDriver.h"

UTGameInstance* TGI = nullptr;

void SNASDlg::Construct(const FArguments& InArgs)
{
	DataDriver = InArgs._ContentDriver;
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
			.FillWidth(0.4)
			[
				MakeNASContent()
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
TSharedRef<SWidget> SNASDlg::MakeNASContent()
{
	return SNew(SBox)		
		.HeightOverride(200)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
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
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2)
				.Padding(5, 1, 5, 1)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("装备名称")))
				]
				+ SHorizontalBox::Slot()
				.Padding(5, 1, 5, 1)
				[
					SNew(SEditableTextBox)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2)
				.Padding(5, 1, 5, 1)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("装备数量")))
				]
				+ SHorizontalBox::Slot()
					.Padding(5, 1, 5, 1)
				[
					SNew(SEditableTextBox)
				]
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2)
				.Padding(5, 1, 5, 1)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("伪装需求度")))
				]
				+ SHorizontalBox::Slot()
					.Padding(5, 1, 5, 1)
				[
					SNew(SEditableTextBox)
				]
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2)
				.Padding(5, 1, 5, 1)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("重要程度")))
				]
				+ SHorizontalBox::Slot()
					.Padding(5, 1, 5, 1)
				[
					SNew(SEditableTextBox)
				]
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.2)
				.Padding(5, 1, 5, 1)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("装备数量")))
				]
				+ SHorizontalBox::Slot()
					.Padding(5, 1, 5, 1)
				[
					SNew(SEditableTextBox)
				]
			]

			+SVerticalBox::Slot()
				.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SHorizontalBox)

					+SHorizontalBox::Slot()
					.Padding(10,10,10,10)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString(TEXT("添加")))						
					]

					+ SHorizontalBox::Slot()
					.Padding(10, 10, 10, 10)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString(TEXT("删除")))
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
	DataDriver->FillSource(ListSource);

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
			.Text(Item->Name)
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