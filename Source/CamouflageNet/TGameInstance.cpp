// Fill out your copyright notice in the Description page of Project Settings.


#include "TGameInstance.h"
#include "STViewportWidget.h"
#include "Engine.h"
#include "TGameViewportClient.h"
#include "XWindow.h"
#include "TEditorGameEngine.h"

#define LOCTEXT_NAMESPACE "TGameInstance"

UTGameInstance* TGI = nullptr;

UTGameInstance::UTGameInstance()
	:mViewport(nullptr)
	,mWindow(nullptr)
{
	TGI = this;
}

void UTGameInstance::Init()
{
	Super::Init();
}

void UTGameInstance::StartGameInstance()
{
	Super::StartGameInstance();

	SAssignNew(mWindow, SXWindow);


	TSharedPtr<SWindow> RootWindow = GEngine->GameViewport->GetWindow();
	RootWindow->SetTitle(FText(LOCTEXT("Î±×°Íø±à¼­Æ÷","CamouFlagEditor")));
	RootWindow->SetContent(mWindow->AsShared());
	/*GEngine->GetGameViewportWidget()->SetContent(mWindow->AsShared());*/

	UTEditorGameEngine* engine = StaticCast<UTEditorGameEngine*>(GEngine);
	if (engine != nullptr)
	{
		TSharedPtr<SViewport> viewport = engine->GetGameViewportWidget();
		if (viewport != nullptr)
		{
			//viewport->SetContent(RootWindow);

		}
	}
	/*UTGameViewportClient* viewClient = StaticCast<UTGameViewportClient*>(GEngine->GameViewport);
	if (viewClient != nullptr)
	{		
		viewClient->SetViewportOverlayWidget(RootWindow, mViewport->mOverLay.ToSharedRef());
	}*/
	GetGameViewportClient()->EngineShowFlags.CompositeEditorPrimitives = true;
}

#undef LOCTEXT_NAMESPACE
