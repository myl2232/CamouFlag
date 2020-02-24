#pragma once

#include "SCompoundWidget.h"
class SBox;
class SImage;

/**	菜单定义*/
struct FXMenuInfo
{
	struct FXMenuSectionInfo
	{
		FName ExtensionHook = NAME_None;
		FText HeadingText = FText();
		TArray<TSharedPtr<FUICommandInfo>> ActionList;
	};
	FText M_Label = FText();
	FText M_Tip = FText();

	TArray<FXMenuSectionInfo> SectionInfos;
};

class SXSysTitleBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SXSysTitleBar) {}

	SLATE_ARGUMENT(TSharedPtr<FUICommandList>, CommandsList)
	SLATE_ARGUMENT(TArray<TSharedPtr<FXMenuInfo>>, MenuConstrctionInfos)
	//SLATE_ARGUMENT(TArray<TSharedPtr<FString>>, ExtendComboList)
	//SLATE_EVENT(FOnComboBoxSelectionChanged, OnComboItemChanged)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static void FillMenu(FMenuBuilder& MenuBuilder, const TSharedRef<FExtender> Extender, TSharedPtr<FXMenuInfo> InMenuInfo);

protected:
	/** 创建菜单 */
	TSharedRef<SWidget> CreateMenu();

private:
	TSharedPtr<SBox> TitleBar;
	float TitleBarHeight = 32.f;
	TSharedPtr<SBox> Box_AppIcon;
	/** app图标 */
	TSharedPtr<SImage> AppIcon;
	/** 菜单栏 */
	TSharedPtr<SBox> MenuBar;

	TSharedPtr<FUICommandList> GlobalCommandsList;
	TArray<TSharedPtr<FXMenuInfo>> MenuConstrctionInfos;
};