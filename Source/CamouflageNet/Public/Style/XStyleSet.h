#pragma once

#include "Styling/SlateStyle.h"

class FXStyleSet : public FSlateStyleSet
{
public:
    FXStyleSet();

    void Initialize();
	void SetEngineContentRoot(const FString& InEngineContentRootDir);
	FString RootToEngineContentDir(const FString& RelativePath, const TCHAR* Extension);
    void SetupDrawPanelStyle();

    /** ������ɫ��ʽ */
    void SetColorStyle();

    /** ����������ʽ */
    void SetFontStyle();

    /** ����widget��ʽ */
    void SetWidgetStyle();

    /** ���ñ༭����ʽ */
    void SetEditorStyle();

    ///** ����ԭʼ����UICommand��ʽ */
    //void SetHomeModeStyle();

    ///** ���ò�Ļ���UICommand��ʽ */
    //void SetLayoutModeStyle();

    ///** ����ӲװģʽUICommand��ʽ */
    //void SetHardModeStyle();

    ///** ����Ӳװ����ģʽUICommand��ʽ */
    //void SetHardModeFloorStyle();

    ///** ������װģʽUICommand��ʽ */
    //void SetSoftModeStyle();

    ///**	����ͼֽģʽUICommand��ʽ */
    //void SetConstructionModeStyle();

    ///**	����ˮ��ģʽUICommand��ʽ */
    //void SetHydropowerModeStyle();

    //void SetWHCModeStyle();

private:
	FString EngineContentRoot;
};