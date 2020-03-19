// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
class FXEditorModeTools;

class CamouflageModule : public IModuleInterface
{
	// IModuleInterface interface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
public:
	/** ����رյĴ��� */
	FSimpleDelegate OnApplicationClosed;
};

CAMOUFLAGENET_API class FXEditorModeTools& GXLevelEditorModeTools();