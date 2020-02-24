// Fill out your copyright notice in the Description page of Project Settings.


#include "TEditorGameEngine.h"
#include "TGameViewportClient.h"
#include "STViewportWidget.h"
#include "TGameInstance.h"

UTGameInstance* TGI = nullptr;

UTEditorGameEngine::UTEditorGameEngine()
{

}

void UTEditorGameEngine::Init(class IEngineLoop* InEngineLoop)
{																					
	UGameEngine::Init(InEngineLoop);
	
	if (GameViewport != NULL)
	{
		TSharedPtr<SViewport> mViewport = GameViewport->GetGameViewportWidget();
		mViewport->SetRenderDirectlyToWindow(false);
		/*UTGameViewportClient* client = StaticCast<UTGameViewportClient*>(GameViewport);
		if (client != NULL)
		{
			client->AddViewportWidgetContent(SNew(STViewportWidget));
		}*/
		
	}
}

void UTEditorGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	Super::Tick(DeltaSeconds, bIdleMode);

	/*UTGameViewportClient* client = StaticCast<UTGameViewportClient*>(GameViewport);
	if (client->Viewport->GetSizeXY().X > 0 && client->IsVisible())
	{
		client->Tick(DeltaSeconds);
		client->Viewport->Draw();
	}*/
}