// Fill out your copyright notice in the Description page of Project Settings.


#include "TEditorGameEngine.h"
#include "Frame/TGameViewportClient.h"
#include "Frame/STViewportWidget.h"
#include "TGameInstance.h"
#include "Engine/Selection.h"
#include "Design/XEditorModeManager.h"
#include "Engine/StaticMeshActor.h"
#include "Core/XGroupActor.h"
#include "CamouflageNet/CamouflageNet.h"

UTEditorGameEngine* GXEngine = nullptr;
//UTGameInstance* TGI = nullptr;
/*----------Global Selection---------------------------*/


static inline USelection*& PrivateGetSelectedActors()
{
	static USelection* SSelectedActors = NULL;
	return SSelectedActors;
};

static inline USelection*& PrivateGetSelectedComponents()
{
	static USelection* SSelectedComponents = NULL;
	return SSelectedComponents;
}

static inline USelection*& PrivateGetSelectedObjects()
{
	static USelection* SSelectedObjects = NULL;
	return SSelectedObjects;
};

static void OnObjectSelected(UObject* Object)
{
	// Whenever an actor is unselected we must remove its components from the components selection
	if (!Object->IsSelected())
	{
		TArray<UActorComponent*> ComponentsToDeselect;
		for (FSelectionIterator It(*PrivateGetSelectedComponents()); It; ++It)
		{
			UActorComponent* Component = CastChecked<UActorComponent>(*It);
			if (Component->GetOwner() == Object)
			{
				ComponentsToDeselect.Add(Component);
			}
		}
		if (ComponentsToDeselect.Num() > 0)
		{
			PrivateGetSelectedComponents()->Modify();
			PrivateGetSelectedComponents()->BeginBatchSelectOperation();
			for (UActorComponent* Component : ComponentsToDeselect)
			{
				PrivateGetSelectedComponents()->Deselect(Component);
			}
			PrivateGetSelectedComponents()->EndBatchSelectOperation();
		}
	}
}

static void PrivateInitSelectedSets()
{
	PrivateGetSelectedActors() = NewObject<USelection>(GetTransientPackage(), TEXT("SelectedActors"), RF_Transactional);
	PrivateGetSelectedActors()->AddToRoot();
	PrivateGetSelectedActors()->Initialize(&GSelectedActorAnnotation);

	PrivateGetSelectedActors()->SelectObjectEvent.AddStatic(&OnObjectSelected);

	PrivateGetSelectedComponents() = NewObject<USelection>(GetTransientPackage(), TEXT("SelectedComponents"), RF_Transactional);
	PrivateGetSelectedComponents()->AddToRoot();
	PrivateGetSelectedComponents()->Initialize(&GSelectedComponentAnnotation);

	PrivateGetSelectedObjects() = NewObject<USelection>(GetTransientPackage(), TEXT("SelectedObjects"), RF_Transactional);
	PrivateGetSelectedObjects()->AddToRoot();
	PrivateGetSelectedObjects()->Initialize(&GSelectedObjectAnnotation);
}

/*-----------------------------------------------*/
UTEditorGameEngine::UTEditorGameEngine()
{
	//EditorModeToolsSingleton = new FXEditorModeTools;
}

void UTEditorGameEngine::Init(class IEngineLoop* InEngineLoop)
{	
	GXEngine = this;							
	PrivateInitSelectedSets();
	
	UGameEngine::Init(InEngineLoop);
	
	if (GameViewport != NULL)
	{
		TSharedPtr<SViewport> mViewport = GameViewport->GetGameViewportWidget();
		mViewport->SetRenderDirectlyToWindow(false);	
	}
}

void UTEditorGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	Super::Tick(DeltaSeconds, bIdleMode);

	UTGameViewportClient* EditorViewportClient = Cast<UTGameViewportClient>(GameViewport);

	if (EditorViewportClient != nullptr &&
		EditorViewportClient->Viewport &&
		EditorViewportClient->Viewport->GetSizeXY().X > 0 &&
		EditorViewportClient->IsVisible())
	{
		EditorViewportClient->Tick(DeltaSeconds);
		EditorViewportClient->Viewport->Draw();

		if (EditorViewportClient->bNeedsInvalidateHitProxy)
		{
			EditorViewportClient->Viewport->InvalidateHitProxy();
			EditorViewportClient->bNeedsInvalidateHitProxy = false;
		}
	}
}
void UTEditorGameEngine::SelectGroup(class AXGroupActor* InGroupActor, bool bForceSelection /*= false*/, bool bInSelected /*= true*/, bool bNotify /*= true*/)
{
	USelection* SelectedActors = GetSelectedActors();
	//SelectedActors->BeginBatchSelectOperation();
	SelectedActors->Modify();

	static bool bIteratingGroups = false;

	if (!bIteratingGroups)
	{
		bIteratingGroups = true;
		// Select all actors within the group (if locked or forced)
		if (bForceSelection || InGroupActor->IsLocked())
		{
			TArray<AActor*> GroupActors;
			InGroupActor->GetGroupActors(GroupActors);
			for (int32 ActorIndex = 0; ActorIndex < GroupActors.Num(); ++ActorIndex)
			{
				SelectActor(GroupActors[ActorIndex], bInSelected, false);
			}
			bForceSelection = true;

			// Recursively select any subgroups
			TArray<AXGroupActor*> SubGroups;
			InGroupActor->GetSubGroups(SubGroups);
			for (int32 GroupIndex = 0; GroupIndex < SubGroups.Num(); ++GroupIndex)
			{
				SelectGroup(SubGroups[GroupIndex], bForceSelection, bInSelected, false);
			}
		}

		//SelectedActors->EndBatchSelectOperation(bNotify);
		if (bNotify)
		{
			NoteSelectionChange();
		}

		bIteratingGroups = false;
	}
}

USelection* UTEditorGameEngine::GetSelectedActors() const
{
	return PrivateGetSelectedActors();
}
class USelection* UTEditorGameEngine::GetSelectedComponents() const
{
	return PrivateGetSelectedComponents();
}
class USelection* UTEditorGameEngine::GetSelectedObjects() const
{
	return PrivateGetSelectedObjects();
}
void UTEditorGameEngine::GetSelectedActors(TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	for (FSelectionIterator It(GetSelectedActorIterator()); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			OutActors.Add(Actor);
		}
	}
}
class FSelectionIterator UTEditorGameEngine::GetSelectedActorIterator() const
{
	return FSelectionIterator(*GetSelectedActors());
}
class FSelectionIterator UTEditorGameEngine::GetSelectedComponentIterator() const
{
	return FSelectionIterator(*GetSelectedComponents());
}
int32 UTEditorGameEngine::GetSelectedComponentCount() const
{
	int32 NumSelectedComponents = 0;
	for (FSelectionIterator It(GetSelectedComponentIterator()); It; ++It)
	{
		++NumSelectedComponents;
	}

	return NumSelectedComponents;
}

int32 UTEditorGameEngine::GetSelectedActorCount() const
{
	int32 NumSelectedActors = 0;
	for (FSelectionIterator It(GetSelectedActorIterator()); It; ++It)
	{
		++NumSelectedActors;
	}

	return NumSelectedActors;
}
bool UTEditorGameEngine::CanAllSelectedBeMoved()
{
	bool result = true;
	//当前不支持任何Component的移动
	if (GetSelectedComponentCount() > 0)
	{
		result = false;
	}

	int32 NumSelectedActor = 0;
	for (FSelectionIterator It(*GetSelectedActors()); It; ++It)
	{
		AStaticMeshActor* SMA = Cast<AStaticMeshActor>(*It);
		AActor* Actor = Cast<AActor>(*It);
		if (SMA)
		{
			//只有StaticMeshActor，并且Tag为可移动的家具，才会被人为是可以选择并能通过坐标系移动的物体
			if (SMA->Tags.Num() > 0 && SMA->Tags[0] == TEXT("MoveableMeshActor"))
			{
			}
			else
			{
				result = false;
			}

			//停用的不可移动
			if (Actor->Tags.Num() > 0 && Actor->Tags.Contains(TEXT("NoWidget")))
			{
				result = false;
			}

		}
		else if (Actor)
		{
			// 3d模式下选中门不显示坐标轴
			if (Actor->ActorHasTag("Door") || Actor->ActorHasTag("SecurityDoor"))
			{
				result = false;
			}

			//只有Actor，并且Tag为可移动的蓝图交互物体，才会被人为是可以选择并能通过坐标系移动的物体
			if (Actor->Tags.Num() > 0 && (Actor->Tags[0] == TEXT("BlueprintVRSActor") || Actor->Tags[0] == TEXT("EnvironmentAsset")) || Actor->Tags.Contains(TEXT("HydropowerActor")) || Actor->Tags.Contains(TEXT("HydropowerPipeActor")))
			{
			}
			else
			{
				result = false;
			}


		}
		NumSelectedActor++;
	}
	//如果没有选中任何Actor，则一定为False
	if (NumSelectedActor == 0)
		result = false;

	return result;
}

FBox UTEditorGameEngine::GetSelectedActorsBox()
{
	FBox SelectActorsBox(ForceInit);
	if (GetSelectedActorsBoundingBoxDelegate.IsBound())
	{
		SelectActorsBox = GetSelectedActorsBoundingBoxDelegate.Execute();
	}
	return SelectActorsBox;
}
/*-------------------*/
void UTEditorGameEngine::ApplyDeltaToActor(AActor* InActor, bool bDelta, const FVector* InTrans, const FRotator* InRot, const FVector* InScale, bool bAltDown /*= false*/, bool bShiftDown /*= false*/, bool bControlDown /*= false*/) const
{
	bool bTranslationOnly = true;

	///////////////////
	// Rotation

	// Unfortunately this can't be moved into ABrush::EditorApplyRotation, as that would
	// create a dependence in Engine on Editor.
	if (InRot)
	{
		const FRotator& InDeltaRot = *InRot;
		const bool bRotatingActor = !bDelta || !InDeltaRot.IsZero();
		if (bRotatingActor)
		{
			bTranslationOnly = false;

			if (bDelta)
			{
				if (InActor->GetRootComponent() != NULL)
				{
					const FRotator OriginalRotation = InActor->GetRootComponent()->GetComponentRotation();

					GXEngine->EditorApplyRotation(InActor, InDeltaRot, bAltDown, bShiftDown, bControlDown);

					// Check to see if we should transform the rigid body
				}

				FVector NewActorLocation = InActor->GetActorLocation();
				NewActorLocation -= GXLevelEditorModeTools().PivotLocation;
				NewActorLocation = FRotationMatrix(InDeltaRot).TransformPosition(NewActorLocation);
				NewActorLocation += GXLevelEditorModeTools().PivotLocation;
				NewActorLocation -= InActor->GetActorLocation();
				GXEngine->EditorApplyTranslation(InActor, NewActorLocation, bAltDown, bShiftDown, bControlDown);
			}
			else
			{
				InActor->SetActorRotation(InDeltaRot);
			}
		}
	}
	if (InTrans)
	{
		if (bDelta)
		{
			if (InActor->GetRootComponent() != NULL)
			{
				const FVector OriginalLocation = InActor->GetRootComponent()->GetComponentLocation();

				GXEngine->EditorApplyTranslation(InActor, *InTrans, bAltDown, bShiftDown, bControlDown);


				// Check to see if we should transform the rigid body
			}
		}
		else
		{
			InActor->SetActorLocation(*InTrans, false);
		}
	}

	///////////////////
	// Scaling
	if (InScale)
	{
		const FVector& InDeltaScale = *InScale;
		const bool bScalingActor = !bDelta || !InDeltaScale.IsNearlyZero(0.000001f);
		if (bScalingActor)
		{
			bTranslationOnly = false;

			FVector ModifiedScale = InDeltaScale;

			// Note: With the new additive scaling method, this is handled in FLevelEditorViewportClient::ModifyScale
			if (false/*GEditor->UsePercentageBasedScaling()*/)
			{
				// Get actor box extents
				const FBox BoundingBox = InActor->GetComponentsBoundingBox(true);
				const FVector BoundsExtents = BoundingBox.GetExtent();

				// Make sure scale on actors is clamped to a minimum and maximum size.
				const float MinThreshold = 1.0f;

				for (int32 Idx = 0; Idx < 3; Idx++)
				{
					if ((FMath::Pow(BoundsExtents[Idx], 2)) > BIG_NUMBER)
					{
						ModifiedScale[Idx] = 0.0f;
					}
					else if (SMALL_NUMBER < BoundsExtents[Idx])
					{
						const bool bBelowAllowableScaleThreshold = ((InDeltaScale[Idx] + 1.0f) * BoundsExtents[Idx]) < MinThreshold;

						if (bBelowAllowableScaleThreshold)
						{
							ModifiedScale[Idx] = (MinThreshold / BoundsExtents[Idx]) - 1.0f;
						}
					}
				}
			}

			if (bDelta)
			{
				// Flag actors to use old-style scaling or not
				// @todo: Remove this hack once we have decided on the scaling method to use.
				//AActor::bUsePercentageBasedScaling = GEditor->UsePercentageBasedScaling();

				GXEngine->EditorApplyScale(
					InActor,
					ModifiedScale,
					&GXLevelEditorModeTools().PivotLocation,
					bAltDown,
					bShiftDown,
					bControlDown
				);

			}
			else if (InActor->GetRootComponent() != NULL)
			{
				InActor->GetRootComponent()->SetRelativeScale3D(InDeltaScale);
			}
		}
	}

	InActor->MarkPackageDirty();
	InActor->InvalidateLightingCacheDetailed(bTranslationOnly);
	InActor->PostEditMove(false);

	//回调Actor被修改。用来刷新UI
	GXEngine->SelectionEditedEvent.ExecuteIfBound(InActor);
}
void UTEditorGameEngine::SetPivot(FVector NewPivot, bool bSnapPivotToGrid, bool bIgnoreAxis, bool bAssignPivot /*= false*/)
{
	FXEditorModeTools& EditorModeTools = GXLevelEditorModeTools();

	if (!bIgnoreAxis)
	{
		// Don't stomp on orthonormal axis.
		// TODO: this breaks if there is genuinely a need to set the pivot to a coordinate containing a zero component
		if (NewPivot.X == 0) NewPivot.X = EditorModeTools.PivotLocation.X;
		if (NewPivot.Y == 0) NewPivot.Y = EditorModeTools.PivotLocation.Y;
		if (NewPivot.Z == 0) NewPivot.Z = EditorModeTools.PivotLocation.Z;
	}

	// Set the pivot.
	EditorModeTools.SetPivotLocation(NewPivot, false);
}

void UTEditorGameEngine::UpdatePivotLocationForSelection(bool bOnChange)
{
	// Pick a new common pivot, or not.
	AActor* SingleActor = nullptr;
	USceneComponent* SingleComponent = nullptr;

	if (GetSelectedComponentCount() > 0)
	{
		for (FSelectedEditableComponentIterator It(*GetSelectedComponents()); It; ++It)
		{
			UActorComponent* Component = CastChecked<UActorComponent>(*It);
			AActor* ComponentOwner = Component->GetOwner();

			if (ComponentOwner != nullptr)
			{
				auto SelectedActors = GetSelectedActors();
				const bool bIsOwnerSelected = SelectedActors->IsSelected(ComponentOwner);
				check(bIsOwnerSelected);

				if (ComponentOwner->GetWorld() == GWorld)
				{
					SingleActor = ComponentOwner;
					if (Component->IsA<USceneComponent>())
					{
						SingleComponent = CastChecked<USceneComponent>(Component);
					}
				}
			}
		}
	}
	else
	{
		for (FSelectionIterator It(GetSelectedActorIterator()); It; ++It)
		{
			AActor* Actor = static_cast<AActor*>(*It);
			checkSlow(Actor->IsA(AActor::StaticClass()));

			if (Actor->GetWorld() == GWorld)
			{
				SingleActor = Actor;
			}
		}
	}

	if (SingleComponent != NULL)
	{
		SetPivot(SingleComponent->GetComponentLocation(), false, true);
	}
	else if (SingleActor != NULL)
	{
		// For geometry mode use current pivot location as it's set to selected face, not actor
		FXEditorModeTools& Tools = GXLevelEditorModeTools();
		/*if (Tools.IsModeActive(FBuiltinEditorModes::EM_Geometry) == false || bOnChange == true)*/
		{
			// Set pivot point to the actor's location, accounting for any set pivot offset
			FVector PivotPoint = SingleActor->GetTransform().TransformPosition(FVector(0, 0, 0) /*SingleActor->GetPivotOffset()*/);

			// If grouping is active, see if this actor is part of a locked group and use that pivot instead
			//if (GVRSEditor->bGroupingActive)
			{
				AXGroupActor* ActorGroupRoot = AXGroupActor::GetRootForActor(SingleActor, true, true);
				if (ActorGroupRoot)
				{
					PivotPoint = ActorGroupRoot->GetActorLocation();
				}
			}
			SetPivot(PivotPoint, false, true);
		}
	}

}

void UTEditorGameEngine::EditorApplyTranslation(AActor* InActor, const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	InActor->EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);
}

void UTEditorGameEngine::EditorApplyRotation(AActor* InActor, const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	USceneComponent* RootComponent = InActor->GetRootComponent();
	if (RootComponent != NULL)
	{
		const FRotator Rot = RootComponent->GetAttachParent() != NULL ? InActor->GetActorRotation() : RootComponent->GetRelativeRotation();

		FRotator ActorRotWind, ActorRotRem;
		Rot.GetWindingAndRemainder(ActorRotWind, ActorRotRem);

		const FQuat ActorQ = ActorRotRem.Quaternion();
		const FQuat DeltaQ = DeltaRotation.Quaternion();
		const FQuat ResultQ = DeltaQ * ActorQ;
		const FRotator NewActorRotRem = FRotator(ResultQ);
		FRotator DeltaRot = NewActorRotRem - ActorRotRem;
		DeltaRot.Normalize();

		if (RootComponent->GetAttachParent() != NULL)
		{
			RootComponent->SetWorldRotation(Rot + DeltaRot);
		}
		else
		{
			// No attachment.  Directly set relative rotation (to support winding)
			RootComponent->SetRelativeRotation(Rot + DeltaRot);
		}
	}
}

void UTEditorGameEngine::EditorApplyScale(AActor* InActor, const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	USceneComponent* RootComponent = InActor->GetRootComponent();
	if (RootComponent != NULL)
	{
		const FVector CurrentScale = RootComponent->GetRelativeScale3D();

		// @todo: Remove this hack once we have decided on the scaling method to use.
		FVector ScaleToApply;

		//if (AActor::bUsePercentageBasedScaling)
		//{
		//	ScaleToApply = CurrentScale * (FVector(1.0f) + DeltaScale);
		//}
		//else
		{
			ScaleToApply = CurrentScale + DeltaScale;
		}

		RootComponent->SetRelativeScale3D(ScaleToApply);

		if (PivotLocation)
		{
			const FVector CurrentScaleSafe(CurrentScale.X ? CurrentScale.X : 1.0f,
				CurrentScale.Y ? CurrentScale.Y : 1.0f,
				CurrentScale.Z ? CurrentScale.Z : 1.0f);

			FVector Loc = InActor->GetActorLocation();
			Loc -= *PivotLocation;
			Loc *= (ScaleToApply / CurrentScaleSafe);
			Loc += *PivotLocation;
			RootComponent->SetWorldLocation(Loc);
		}
	}
}
