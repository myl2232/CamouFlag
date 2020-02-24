#include "XFrameCommands.h"
#include "SlateApplication.h"
#include "XResManager.h"

#define LOCTEXT_NAMESPACE "FXFrameCommands"

void FXFrameCommands::RegisterCommands()
{
	UI_COMMAND(CommandMenuNew, "�½�����", "New", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::N));
	UI_COMMAND(CommandMenuOpen, "�򿪷���", "Open", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::O));

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

}

void FXFrameCommands::OnMenuOpen()
{
	//������Դ
	TSharedPtr<FXObject> item = MakeShareable(new FXObject());
	item->FilePath = "/Game/MilitaryVehicles/Meshes/Tank_RU/Sm_Turret";
	AActor* tActor = UXResManagerInstance::GetInstance().GetResManager().CreateActor(item, FVector(0, 0, 120));
	if (tActor)
	{
		printf("xpxpxpxpxpxpxpxpxpxp");
	}
}

#undef LOCTEXT_NAMESPACE