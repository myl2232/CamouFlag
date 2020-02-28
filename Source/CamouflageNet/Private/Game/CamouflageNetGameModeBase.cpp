// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "CamouflageNetGameModeBase.h"
#include "XPlayerController.h"

ACamouflageNetGameModeBase::ACamouflageNetGameModeBase(const FObjectInitializer &ObjectInitializer)
{
	PlayerControllerClass = AXPlayerController::StaticClass();
}