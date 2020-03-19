// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CamouflageNet.h"
#include "Modules/ModuleManager.h"
#include "Data/XResManager.h"
#include "Style/XStyle.h"
#include "Design/XEditorModeManager.h"

void CamouflageModule::StartupModule()
{
	UXResManagerInstance::GetInstance().Startup();
	FXStyle::Startup();
}
void CamouflageModule::ShutdownModule()
{
	UXResManagerInstance::GetInstance().ReleaseInstance();
	FXStyle::Shutdown();
}

class FXEditorModeTools& GXLevelEditorModeTools()
{
	static FXEditorModeTools* EditorModeToolsSingleton = new FXEditorModeTools;
	return *EditorModeToolsSingleton;
}

IMPLEMENT_PRIMARY_GAME_MODULE(CamouflageModule/*FDefaultGameModuleImpl*/, CamouflageNet, "CamouflageNet" );
