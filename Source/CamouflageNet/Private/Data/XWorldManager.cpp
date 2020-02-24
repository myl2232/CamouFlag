#include "XWorldManager.h"
#include "EngineUtils.h"
#include "Engine.h"

TSharedPtr<FXWorldManager> FXWorldManager::mInstance = nullptr;

FXWorldManager::FXWorldManager()
{

}

FXWorldManager::~FXWorldManager()
{

}

void FXWorldManager::Init(UWorld* world)
{
	for (TActorIterator<AActor> ActorIt(world); ActorIt; ++ActorIt)
	{
		FString AName = (*ActorIt)->GetName();
		if (AName == "MilitaryVehicle")
		{

		}
	}
}

void FXWorldManager::OpenLevel(FString LevelName, bool bAbsolute)
{
	const ETravelType TravelType = (bAbsolute ? TRAVEL_Absolute : TRAVEL_Relative);
	FWorldContext &WorldContext = GEngine->GetWorldContextFromWorldChecked(GWorld);
	FString Cmd = LevelName;
	/*if (Options.Len() > 0)
	{
		Cmd += FString(TEXT("?")) + Options;
	}*/
	FURL TestURL(&WorldContext.LastURL, *Cmd, TravelType);
	if (TestURL.IsLocalInternal())
	{
		// make sure the file exists if we are opening a local file
		if (!GEngine->MakeSureMapNameIsValid(TestURL.Map))
		{
			UE_LOG(LogLevel, Warning, TEXT("WARNING: The map '%s' does not exist."), *TestURL.Map);
		}
	}

	GEngine->SetClientTravel(GWorld, *Cmd, TravelType);
}