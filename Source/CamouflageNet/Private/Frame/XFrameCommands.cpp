#include "XFrameCommands.h"
#include "SlateApplication.h"
#include "Data/XResManager.h"
#include "TGameInstance.h"
#include "Frame/NASDlg.h"
#include "Frame/XWindow.h"
#include "SharedPointer.h"

#define LOCTEXT_NAMESPACE "FXFrameCommands"

UTGameInstance* TGI = nullptr;
TSharedPtr<SNASDlg> FXFrameCommands::mNASDlg = nullptr;

void FXFrameCommands::RegisterCommands()
{
	UI_COMMAND(CommandMenuNew, "新建方案", "New", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::N));
	UI_COMMAND(CommandMenuOpen, "打开方案", "Open", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::O));

	BindGlobalCommands();
}

void FXFrameCommands::BindGlobalCommands()
{
	GlobalCommandsList = MakeShareable(new FUICommandList);

	FCanExecuteAction DefaultExecuteAction = FCanExecuteAction::CreateStatic(&FXFrameCommands::DefaultCanExecuteAction);

	GlobalCommandsList->MapAction(CommandMenuNew, FExecuteAction::CreateStatic(&FXFrameCommands::OnMenuNew), DefaultExecuteAction);
	GlobalCommandsList->MapAction(CommandMenuOpen, FExecuteAction::CreateStatic(&FXFrameCommands::OnMenuOpen), DefaultExecuteAction);
}

bool FXFrameCommands::DefaultCanExecuteAction()
{
	return FSlateApplication::Get().IsNormalExecution();
}

void FXFrameCommands::OnMenuNew()
{
	if (TGI)
	{
		SAssignNew(mNASDlg, SNASDlg);
		TGI->mWindow->PresentModalDialog(TEXT("伪装需求评估"), mNASDlg->AsShared(), 
			FSimpleDelegate::CreateRaw(mNASDlg.Get(), &SNASDlg::OnConfirmClick));
	}
	
}

void FXFrameCommands::OnMenuOpen()
{
	//测试资源
	TSharedPtr<FXObject> item = MakeShareable(new FXObject());
	item->FilePath = "/Game/MilitaryVehicles/Meshes/Tank_RU/Sm_Turret";
	AActor* tActor = UXResManagerInstance::GetInstance().GetResManager().CreateActor(item, FVector(0, 0, 120));
	if (tActor)
	{
		printf("xpxpxpxpxpxpxpxpxpxp");
	}
}

#undef LOCTEXT_NAMESPACE