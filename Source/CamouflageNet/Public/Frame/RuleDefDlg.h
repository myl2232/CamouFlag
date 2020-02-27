#pragma once
#include "SCompoundWidget.h"
#include "Core/XCoreType.h"

class CAMOUFLAGENET_API SRuleDefDlg : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRuleDefDlg)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	bool bCloseImmediately;
};