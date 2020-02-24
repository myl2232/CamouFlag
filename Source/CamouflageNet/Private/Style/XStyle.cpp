#include "XStyle.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "XStyleSet.h"

TSharedPtr<ISlateStyle> FXStyle::Instance = nullptr;

TSharedPtr<FXStyleSet> FXStyle::Create()
{
    TSharedPtr<FXStyleSet> NewStyleSet = MakeShareable(new FXStyleSet());
    NewStyleSet->Initialize();
	FSlateStyleRegistry::RegisterSlateStyle(*NewStyleSet.Get());
    return NewStyleSet;
}

void FXStyle::Startup()
{
    Instance = Create();
    SetStyle(Instance.ToSharedRef());
}

void FXStyle::Shutdown()
{
    ResetToDefault();
    Instance.Reset();
}

ISlateStyle& FXStyle::Get()
{
    return *(Instance.Get());
}

const FSlateBrush* FXStyle::GetBrush(FName PropertyName, const ANSICHAR* Specifier /*= nullptr*/)
{
    return Instance->GetBrush(PropertyName, Specifier);
}

const FLinearColor& FXStyle::GetColor(FName PropertyName, const ANSICHAR* Specifier /*= nullptr*/)
{
    return Instance->GetColor(PropertyName, Specifier);
}

void FXStyle::ResetToDefault()
{
    SetStyle(FCoreStyle::Create("XRStyle"));
}

void FXStyle::SetStyle(const TSharedRef<class ISlateStyle>& NewStyle)
{
    if (Instance != NewStyle)
    {
        if (Instance.IsValid())
        {
            FSlateStyleRegistry::UnRegisterSlateStyle(*Instance.Get());
        }

        Instance = NewStyle;

        if (Instance.IsValid())
        {
            FSlateStyleRegistry::RegisterSlateStyle(*Instance.Get());
        }
        else
        {
            ResetToDefault();
        }
    }
}