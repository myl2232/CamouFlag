#pragma once
#include "CoreMinimal.h"
#include "Commands.h"

class SNASDlg;

class FXFrameCommands : public TCommands<FXFrameCommands>
{
public:
	FXFrameCommands() : TCommands<FXFrameCommands>(
		TEXT("XFrame"),
		NSLOCTEXT("Contexts", "XFrame", "XFrame"),
		NAME_None,
		/*FXRStyle::Get().GetStyleSetName()*/
		"DefaultStyle"
		) {}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<class FUICommandList> GlobalCommandsList;
	//File
	TSharedPtr<FUICommandInfo> CommandMenuNew;		
	TSharedPtr<FUICommandInfo> CommandMenuOpen;		

	static TSharedPtr<SNASDlg> mNASDlg;
private:
	void BindGlobalCommands();
	
public:
	static bool DefaultCanExecuteAction();
	static void OnMenuNew();
	static void OnMenuOpen();
};