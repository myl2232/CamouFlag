#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "XCoreType.h"
#include "XResManager.generated.h"

class UResInterface;

USTRUCT()
struct CAMOUFLAGENET_API FXSceneItem //: public TSharedFromThis<FXSceneItem>
{
	GENERATED_USTRUCT_BODY()
public:
	FXSceneItem() {};
	~FXSceneItem() {};
public:
	FString FilePath;

};

//USTRUCT()
struct CAMOUFLAGENET_API FXObject //: public TSharedFromThis<FXObject>
{
	//GENERATED_USTRUCT_BODY()
public:
	//FXObject() {};
	FXObject(EResType type = EResType::Default)
		:mType(type)
	{
	};
	~FXObject() {};
public:
	UObject* mObject;
	EResType mType;
	FString FilePath;
};

UCLASS()
class CAMOUFLAGENET_API UXResManager : public UObject
{
	GENERATED_BODY()

private:
	friend class UXResManagerInstance;
	UXResManager();
	~UXResManager();
public:

	AActor* CreateActor(TSharedPtr<FXObject> data, FVector _Location = FVector::ZeroVector,
		FRotator _Rotation = FRotator::ZeroRotator, FVector _Scale = FVector(1, 1, 1));

	AActor* LoadObject(TSharedPtr<FXObject> data/*, FXObject& xObj*/);

	void Init();

protected:
	AActor* CreateActorInternal(FXObject& obj);

protected:
	TArray<AActor*> mActorList;
	//UResInterface* mIResInterface;
};

class CAMOUFLAGENET_API UXResManagerInstance
{
private:
	UXResManagerInstance();
	~UXResManagerInstance();
public:
	static UXResManagerInstance & GetInstance();
	static void ReleaseInstance();
	UXResManager& GetResManager();
	void Startup();

private:
	UPROPERTY()
	UXResManager* mResManager;
	static UXResManagerInstance* Instance;
};
