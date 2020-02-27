// Fill out your copyright notice in the Description page of Project Settings.


#include "TGameInstance.h"
#include "Frame/STViewportWidget.h"
#include "Engine.h"
#include "TGameViewportClient.h"
#include "Frame/XWindow.h"
#include "TEditorGameEngine.h"
#include "Data/XResManager.h"
#include "Frame/XFrameCommands.h"
#include "Frame/XSysTitleBar.h"

#define LOCTEXT_NAMESPACE "TGameInstance"

UTGameInstance* TGI = nullptr;

UTGameInstance::UTGameInstance()
	:mViewport(nullptr)
	, mWindow(nullptr)
{
	TGI = this;
}

void UTGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTGameInstance::OnPostLoadMap);
}

void UTGameInstance::StartGameInstance()
{
	Super::StartGameInstance();

	InitUICommands();
	SAssignNew(mWindow, SXWindow)
		.MenuConstructionInfos(MenuInfoArray)
		.CommandList(FXFrameCommands::Get().GlobalCommandsList);
	
	TSharedPtr<SWindow> RootWindow = GEngine->GameViewport->GetWindow();
	RootWindow->SetTitle(FText(LOCTEXT("伪装网编辑器", "CamouFlagEditor")));
	RootWindow->SetContent(mWindow->AsShared());
	GetGameViewportClient()->EngineShowFlags.CompositeEditorPrimitives = true;

	InitStartUp();
}

void UTGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);

	FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::Mouse);
	//FSlateApplication::Get().ClearKeyboardFocus();
	//OnPostloadAsset();
}

void UTGameInstance::OnPostloadAsset()
{
	////测试资源
	//TSharedPtr<FXSceneItem> item = MakeShareable(new FXSceneItem());
	//item->FilePath = "/Game/MilitaryVehicles/Meshes/Tank_RU/Sm_Turret";
	//AActor* tActor = UXResManagerInstance::GetInstance().GetResManager().CreateActor(item, FVector(0, 0, 120));
	//if (tActor)
	//{
	//	printf("xpxpxpxpxpxpxpxpxpxp");
	//}
	//灯光
	//FActorSpawnParameters SpawnInfo;
	//SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//SpawnInfo.bNoFail = false;
	//SpawnInfo.ObjectFlags = RF_Transient;

	//ALight* dLight = GWorld->SpawnActor<ALight>(SpawnInfo);
	//FLinearColor color(1, 0, 0);
	//ULightComponent* lightCom = NewObject<ULightComponent>();
	//dLight->AddComponent(lightCom);
	//dLight->SetLightColor(color);
}
void UTGameInstance::InitStartUp()
{
	if (GWorld->GetCurrentLevel()->GetFName() != "Startup")
	{
		//UXResManagerInstance::GetInstance().Startup();

		//测试场景
		UGameplayStatics::OpenLevel(GWorld, "Startup");
	}	

}

void UTGameInstance::InitUICommands()
{
	FXFrameCommands::Register();
	TSharedPtr<FXMenuInfo> MenuInfoPtr = MakeShareable(new FXMenuInfo);
	MenuInfoPtr->M_Label = FText::FromString(TEXT("文件(F)"));
	MenuInfoPtr->M_Tip = FText::FromString(TEXT("Open the file menu"));
	MenuInfoPtr->SectionInfos.Push(FXMenuInfo::FXMenuSectionInfo());
	FXMenuInfo::FXMenuSectionInfo& RefSection = MenuInfoPtr->SectionInfos.Top();
	RefSection.HeadingText = FText::FromString(TEXT(""));
	RefSection.ActionList.Push(FXFrameCommands::Get().CommandMenuNew);
	RefSection.ActionList.Push(FXFrameCommands::Get().CommandMenuOpen);
	MenuInfoArray.Push(MenuInfoPtr);
}

void UTGameInstance::OnPostLoadMap(UWorld* world)
{
	//if(world->GetCurrentLevel()->GetFName() == "Startup")
	//	OnPostloadAsset();	
}

#undef LOCTEXT_NAMESPACE
