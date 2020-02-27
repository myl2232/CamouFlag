#pragma once

#include "Core/XCoreType.h"

class FContentDriver 
{
public:
	FContentDriver() {};
	virtual void FillSource(TArray<TSharedPtr<FSingleListItem>>& listSource) {};
};

class FProjectDriver : public FContentDriver
{
public:
	FProjectDriver();
	virtual void FillSource(TArray<TSharedPtr<FSingleListItem>>& listSource) override;
};

class FRuleDefDriver : public FContentDriver
{
public:
	FRuleDefDriver();
	virtual void FillSource(TArray<TSharedPtr<FSingleListItem>>& listSource) override;
};