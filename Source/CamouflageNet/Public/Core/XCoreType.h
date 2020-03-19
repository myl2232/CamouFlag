#pragma once

class SWidget;

namespace EXDragTool
{
	enum Type
	{
		BoxSelect,
		FrustumSelect,
		Measure,
		ViewportChange
	};
}

namespace EXLevelViewportType
{
	UENUM()
		enum Type
	{
		/** Top */
		LVT_OrthoXY = 0,
		/** Front */
		LVT_OrthoXZ = 1,
		/** Left */
		LVT_OrthoYZ = 2,
		LVT_Perspective = 3,
		LVT_OrthoFreelook = 4,
		/** Bottom */
		LVT_OrthoNegativeXY = 5,
		/** Back */
		LVT_OrthoNegativeXZ = 6,
		/** Right */
		LVT_OrthoNegativeYZ = 7,
		LVT_OnlyCanvas = 8,
		LVT_MAX,

		LVT_None = 255,
	};
}

struct XArgument
{
#define X_ARGUMENT(ArgType, ArgName) \
ArgType _##ArgName; \
		XArgument& ArgName( ArgType InArg ) \
		{ \
			_##ArgName = InArg; \
			return *this; \
		}

	XArgument(int32 InType = -1) : _ArgUint8(0), _ArgInt32(InType), _ArgUint32(0), _ArgBoolean(false) {}
	X_ARGUMENT(uint8, ArgUint8);
	X_ARGUMENT(int32, ArgInt32);
	X_ARGUMENT(uint32, ArgUint32);
	X_ARGUMENT(bool, ArgBoolean);
	X_ARGUMENT(FString, ArgString);
	X_ARGUMENT(FName, ArgFName);
	X_ARGUMENT(FColor, ArgColor);
	X_ARGUMENT(float, ArgFloat);
	X_ARGUMENT(void*, ArgVoid);
};

enum EResType
{
	SKeletonMesh = 1,
	StaticMesh = 2,
	BlueprintClass = 3,
	Material = 4,
	Texture = 5,
	Default = 2,
};

struct FSingleListItem
{
public:
	FSingleListItem(FText name) :Name(name) {}
	FText Name = FText();
	FString Path;
};

struct FXKeyValue
{
	int32 Key;
	FString Value;
	FXKeyValue() {}
	FXKeyValue(int32 InKey, FString InValue)
	{
		Key = InKey;
		Value = InValue;
	}

	bool InitFromString(const TCHAR * InSourceString)
	{
		Key = -1;
		Value = TEXT("");

		return FParse::Value(InSourceString, TEXT("Key="), Key) && FParse::Value(InSourceString, TEXT("Value="), Value);
	}

	FString ToString()
	{
		return FString::Printf(TEXT("Key=%d Value=%s"), Key, *Value);
	}
};

DECLARE_DELEGATE_OneParam(SCommandOperator, XArgument);
DECLARE_DELEGATE_OneParam(FJsonObjectDelegate, TSharedPtr<class FJsonObject>);
//DECLARE_DELEGATE_TwoParams(SItemOperator, const ItemInfo&, int32);
DECLARE_DELEGATE_OneParam(FInputKeyDelegate, const struct FKeyEvent&);
DECLARE_DELEGATE_OneParam(FInt32Delegate, const int32);
DECLARE_DELEGATE_TwoParams(FTwoInt32Delegate, const int32, const int32);
DECLARE_DELEGATE_FourParams(FFourInt32Delegate, const int32, const int32, const int32, const int32);
DECLARE_DELEGATE_OneParam(FStringDelegate, const FString&);
DECLARE_DELEGATE_TwoParams(FTwoStringDelegate, FString, FString);
DECLARE_DELEGATE_TwoParams(FInt32StringDelegate, const int32, const FString&);
DECLARE_DELEGATE_OneParam(FFloatDelegate, const float);
DECLARE_DELEGATE_OneParam(FVectorDelegate, const FVector&);
DECLARE_DELEGATE_OneParam(FVector2DDelegate, const FVector2D&);
DECLARE_DELEGATE_OneParam(FBoolDelegate, bool);
DECLARE_DELEGATE_OneParam(FObjectDelegate, UObject*);
DECLARE_DELEGATE_OneParam(FActorDelegate, class AActor*);
DECLARE_DELEGATE_OneParam(FOnDelegateColor, const FLinearColor&);
DECLARE_DELEGATE_TwoParams(FLine2DDelegate, const FVector2D&, const FVector2D&);
DECLARE_DELEGATE_ThreeParams(FCircle2DDelegate, const FVector2D&, const FVector2D&, const FVector2D&);
DECLARE_DELEGATE_RetVal(TSharedPtr<class SWidget>, FGetWidgetDelegate);
DECLARE_DELEGATE_OneParam(FXRObjectesDelegate, TArray<TSharedPtr< class FXRObject>>&);
DECLARE_DELEGATE_TwoParams(FXRObjectesTransactionDelegate, TArray<TSharedPtr< class FXRObject>>&, bool);
DECLARE_DELEGATE_ThreeParams(FXRObjectDelegate, TSharedPtr<class FXRObject>, bool, bool);
DECLARE_DELEGATE_TwoParams(FXRObjectAndUObjectDelegate, TSharedPtr<class FXRObject>, class AActor*);

DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiTwoInt32Delegate, const int32, const int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiActorDelegate, class AActor*);
DECLARE_MULTICAST_DELEGATE(FMultiVoidDelegate);
DECLARE_DELEGATE_OneParam(FPointerEventDelegate, const FPointerEvent&)