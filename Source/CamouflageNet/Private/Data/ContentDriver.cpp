#include "ContentDriver.h"

#define LOCTEXT_NAMESPACE "ContentDriver"

FProjectDriver::FProjectDriver()
{

}
void FProjectDriver::FillSource(TArray<TSharedPtr<FSingleListItem>>& ListSource)
{
	//check(ListSource);
	ListSource.Reset();
	//temp
	ListSource.Add(MakeShareable(new FSingleListItem(FText::FromString(TEXT("方案1")))));
	ListSource.Add(MakeShareable(new FSingleListItem(FText::FromString(TEXT("方案2")))));
	ListSource.Add(MakeShareable(new FSingleListItem(FText::FromString(TEXT("方案3")))));
}
/*-----------------------------------------------*/
FRuleDefDriver::FRuleDefDriver()
{

}

void FRuleDefDriver::FillSource(TArray<TSharedPtr<FSingleListItem>>& ListSource)
{

}
/*------------------------------------------------*/
FNASDriver::FNASDriver()
{

}
void FNASDriver::FillSource(TArray<TSharedPtr<FSingleListItem>>& ListSource)
{
	//temp
	ListSource.Add(MakeShareable(new FSingleListItem(FText::FromString(TEXT("装备1")))));
	ListSource.Add(MakeShareable(new FSingleListItem(FText::FromString(TEXT("装备2")))));
	ListSource.Add(MakeShareable(new FSingleListItem(FText::FromString(TEXT("装备3")))));
}
#undef LOCTEXT_NAMESPACE