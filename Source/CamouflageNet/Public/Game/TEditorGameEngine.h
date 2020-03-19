// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameEngine.h"
#include "Core/XCoreType.h"
#include "TEditorGameEngine.generated.h"

class AXGroupActor;
/**
 * 
 */
UCLASS()
class CAMOUFLAGENET_API UTEditorGameEngine : public UGameEngine
{
	GENERATED_BODY()
public:
	UTEditorGameEngine();
	virtual void Init(class IEngineLoop* InEngineLoop) override;
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;

	/*------------Selection--------------*/
	class USelection* GetSelectedActors() const;
	class USelection* GetSelectedComponents() const;
	class USelection* GetSelectedObjects() const;
	void GetSelectedActors(TArray<AActor*>& OutActors);
	class FSelectionIterator GetSelectedActorIterator() const;
	class FSelectionIterator GetSelectedComponentIterator() const;
	int32 GetSelectedComponentCount() const;
	int32 GetSelectedActorCount() const;
	bool CanAllSelectedBeMoved();

	virtual void SelectActor(AActor* Actor, bool bInSelected, bool bNotify, bool bSelectEvenIfHidden = false, bool bForceRefresh = false);
	virtual void SelectNone(bool bNoteSelectionChange, bool bDeselectBSPSurfs, bool WarnAboutManyActors = true);
	void DeselectActor(bool bNoteSelectionChange, bool bDeselectBSPSurfs, bool WarnAboutManyActors = true);
	virtual void NoteSelectionChange();
	virtual void SetActorSelectionFlags(AActor* InActor);
	virtual void SelectGroup(AXGroupActor* InGroupActor, bool bForceSelection = false, bool bInSelected = true, bool bNotify = true);
	virtual void SetPivot(FVector NewPivot, bool bSnapPivotToGrid, bool bIgnoreAxis, bool bAssignPivot = false);

	void ApplyDeltaToActor(AActor* InActor, bool bDelta, const FVector* InTranslation, const FRotator* InRotation, const FVector* InScaling, bool bAltDown = false, bool bShiftDown = false, bool bControlDown = false) const;
	/*--EngineEd-*/
	/**
	* Called to reset the editor's pivot (widget) location using the currently selected objects.  Usually
	* called when the selection changes.
	* @param bOnChange Set to true when we know for a fact the selected object has changed
	*/
	void UpdatePivotLocationForSelection(bool bOnChange = false);
	void EditorApplyTranslation(AActor* InActor, const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown);
	void EditorApplyRotation(AActor* InActor, const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown);
	void EditorApplyScale(AActor* InActor, const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown);
	//void EditorApplyMirror(AActor* InActor, const FVector& MirrorScale, const FVector& PivotLocation);
	


	DECLARE_DELEGATE_TwoParams(FOnDelegateActorsInt, TArray<AActor*>, int);
	DECLARE_DELEGATE_ThreeParams(FOnDelegateActorMIInt, AActor*, UMaterialInterface*, int32);
	DECLARE_DELEGATE_TwoParams(FOnDelegateIntMI, int32, UMaterialInterface*);
	DECLARE_DELEGATE_OneParam(FOnDelegateUnGroup, TArray<AXGroupActor*>&);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDelegateDynamicActor, AActor*, NewActor);
	DECLARE_DELEGATE_RetVal(FBox, FOnDelegateVector);

	DECLARE_EVENT_OneParam(UXREditorEngine, FOnEndTransformObject, UObject&);
	FOnDelegateVector GetSelectedActorsBoundingBoxDelegate;
	FOnDelegateActorsInt OnActorsOperation;
	/** Delegate broadcast when an actor or component has been moved, rotated, or scaled */
	FOnEndTransformObject OnEndObjectTransformEvent;
	/**
	* Called when an actor or component has been translated, rotated, or scaled by the editor
	*
	* @param Object	The actor or component that moved
	*/
	void BroadcastEndObjectMovement(UObject& Object) const
	{
		OnEndObjectTransformEvent.Broadcast(Object);
	}
	/** Editor-only event triggered after actor or component has moved, rotated or scaled by an editor system */

	FOnEndTransformObject& OnEndObjectMovement()
	{
		return OnEndObjectTransformEvent;
	}

	FBox GetSelectedActorsBox();
public:
	UPROPERTY()
	FVector ClickLocation;

	UPROPERTY()
	FPlane ClickPlane;

	UPROPERTY()
	FVector MouseMovement;

	FObjectDelegate SelectionEditedEvent;
	FSimpleDelegate RightSelectedEvent;
	FSimpleDelegate SelectNonePreEvent;
	
};

extern CAMOUFLAGENET_API class UTEditorGameEngine* GXEngine;