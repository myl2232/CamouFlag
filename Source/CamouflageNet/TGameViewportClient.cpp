// Fill out your copyright notice in the Description page of Project Settings.


#include "TGameViewportClient.h"
#include "STViewportWidget.h"
#include "TGameInstance.h"
#include "SceneViewport.h"

UTGameInstance* TGI = nullptr;

UTGameViewportClient::UTGameViewportClient(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	//SetViewportOverlayWidget(nullptr,);
}

UTGameViewportClient::~UTGameViewportClient()
{

}

void UTGameViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FSceneViewport* sceneView = GetGameViewport();
	if (sceneView)
	{
		//sceneView->ShowCursor(true);
	}

}