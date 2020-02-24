#pragma once
#include "CoreMinimal.h"

class CAMOUFLAGENET_API FXWorldManager
{
public:
	FXWorldManager();
	~FXWorldManager();
	static FXWorldManager& Get()
	{
		if (!mInstance.IsValid())
		{
			mInstance = MakeShareable(new FXWorldManager());
		}
		return *mInstance;
	}

	void Init(UWorld* world);
	void OpenLevel(FString LevelName, bool bAbsolute = true);

private:
	static TSharedPtr<FXWorldManager> mInstance;
};