#include "XResManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"

UXResManagerInstance* UXResManagerInstance::Instance = nullptr;

UXResManager::UXResManager()
{

}

UXResManager::~UXResManager()
{

}

void UXResManager::Init()
{
	//mIResInterface = NewObject<UResInterface>();
	//tArr.Add(2);
}

AActor* UXResManager::CreateActor(TSharedPtr<FXObject> data, FVector _Location,	FRotator _Rotation, FVector _Scale)
{
	AActor* newActor = LoadObject(data);
	check(newActor);
	newActor->SetActorLocationAndRotation(_Location, _Rotation);
	newActor->SetActorScale3D(_Scale);

	return newActor;
}

AActor* UXResManager::LoadObject(TSharedPtr<FXObject> data)
{	
	UObject* obj = StaticLoadObject(UObject::StaticClass(), NULL, *data->FilePath);
	//FXObject xObj(EResType::StaticMesh);
	//xObj.mObject = obj;
	//xObj.mType = EResType::StaticMesh;//test
	data->mObject = obj;
	data->mType = EResType::StaticMesh;
	AActor* newActor = NULL;
	{
		UStaticMesh* SM = StaticCast<UStaticMesh*>(obj);
		if (SM)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.bNoFail = false;
			SpawnInfo.ObjectFlags = RF_Transient;
			AStaticMeshActor* SMA = GWorld->SpawnActor<AStaticMeshActor>(SpawnInfo);
			SMA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			SMA->GetStaticMeshComponent()->SetStaticMesh(SM);
			SMA->Tags.Reset();
			SMA->Tags.Add(FName("MoveableMeshActor"));
			SMA->Rename(TEXT("MilitaryVehicle"));
			newActor = SMA;
		}
	}
	mActorList.Add(newActor);
	return newActor;
}

AActor* UXResManager::CreateActorInternal(FXObject& obj)
{
	AActor* newActor = NULL;
	if (obj.mType == EResType::StaticMesh)
	{
		UStaticMesh* SM = StaticCast<UStaticMesh*>(obj.mObject);
		if (SM)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.bNoFail = false;
			SpawnInfo.ObjectFlags = RF_Transient;
			AStaticMeshActor* SMA = GWorld->SpawnActor<AStaticMeshActor>(SpawnInfo);
			SMA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			SMA->GetStaticMeshComponent()->SetStaticMesh(SM);
			SMA->Tags.Reset();
			SMA->Tags.Add(FName("MoveableMeshActor"));
			SMA->Rename(TEXT("MilitaryVehicle"));
			newActor = SMA;
		}
	}

	return newActor;
}
/*------------------------------*/
UXResManagerInstance::UXResManagerInstance()
{
	mResManager = NULL;
}
UXResManagerInstance::~UXResManagerInstance()
{

}
void UXResManagerInstance::Startup()
{
	if (mResManager == NULL)
	{
		mResManager = NewObject<UXResManager>();
		mResManager->AddToRoot();
	}
	if (mResManager)
	{
		mResManager->Init();
	}
}
UXResManagerInstance & UXResManagerInstance::GetInstance()
{
	if (!Instance)
	{
		Instance = new UXResManagerInstance();
	}
	return *Instance;
}

void UXResManagerInstance::ReleaseInstance()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

UXResManager& UXResManagerInstance::GetResManager()
{
	check(mResManager);
	return *mResManager;
}