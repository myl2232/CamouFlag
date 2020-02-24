// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class CamouflageModule : public IModuleInterface
{
	// IModuleInterface interface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
public:
	/** 程序关闭的代理 */
	FSimpleDelegate OnApplicationClosed;
};