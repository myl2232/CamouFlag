// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CamouflageNet.h"
#include "Modules/ModuleManager.h"
#include "Data/XResManager.h"

void CamouflageModule::StartupModule()
{
	UXResManagerInstance::GetInstance().Startup();
}
void CamouflageModule::ShutdownModule()
{
	UXResManagerInstance::GetInstance().ReleaseInstance();
}

IMPLEMENT_PRIMARY_GAME_MODULE(CamouflageModule/*FDefaultGameModuleImpl*/, CamouflageNet, "CamouflageNet" );
