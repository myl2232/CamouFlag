#pragma once

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

