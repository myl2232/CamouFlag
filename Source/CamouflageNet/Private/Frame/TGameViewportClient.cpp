// Fill out your copyright notice in the Description page of Project Settings.


#include "TGameViewportClient.h"
#include "STViewportWidget.h"
#include "Game/TGameInstance.h"
#include "SceneViewport.h"
#include "Engine/Console.h"
#include "Game/TEditorGameEngine.h"
#include "Core/XActorPositioning.h"
#include "Design/XClickHandlers.h"
#include "Design/XUnrealWidget.h"
#include "Design/XEditorModeManager.h"
#include "CamouflageNet/CamouflageNet.h"
#include "EngineUtils.h"
#include "Design/XMouseDeltaTracker.h"
#include "Core/XGroupActor.h"
#include "Core/XCoreType.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "LegacyScreenPercentageDriver.h"
#include "Components/LineBatchComponent.h"
#include "Engine/Canvas.h"
#include "EngineModule.h"
//#include "Design/XDragTool_BoxSelect.h"
#include "Design/XDragTool_FrustumSelect.h"

#define LOCTEXT_NAMESPACE "TGameViewportClient"

//UTGameInstance* TGI = nullptr;
IMPLEMENT_HIT_PROXY(HXObjectBaseProxy, HHitProxy);

#define CAMERA_ZOOM_DAMPEN			200.f
#define MIN_ORTHOZOOM				10.0					/* Limit of 2D viewport zoom in */
#define MAX_ORTHOZOOM				1800000.0f					/* Limit of 2D viewport zoom out */

static const float MIN_ACTOR_BOUNDS_EXTENT = 1.0f;
namespace XOrbitConstants
{
	const float OrbitPanSpeed = 1.0f;
	const float IntialLookAtDistance = 1024.f;
}

namespace XFocusConstants
{
	const float TransitionTime = 0.25f;
}

namespace
{
	static const float GridSize = 20480.0f;
	static const int32 CellSize = 160;
	static const float AutoViewportOrbitCameraTranslate = 256.0f;
	static const float LightRotSpeed = 0.22f;
}

//** Helper function for ModifyScale - Convert the active Dragging Axis to per - axis flags * /
static void CheckActiveAxes(EAxisList::Type DraggingAxis, bool bActiveAxes[3])
{
	bActiveAxes[0] = bActiveAxes[1] = bActiveAxes[2] = false;
	switch (DraggingAxis)
	{
	default:
	case EAxisList::None:
		break;
	case EAxisList::X:
		bActiveAxes[0] = true;
		break;
	case EAxisList::Y:
		bActiveAxes[1] = true;
		break;
	case EAxisList::Z:
		bActiveAxes[2] = true;
		break;
	case EAxisList::XYZ:
	case EAxisList::All:
	case EAxisList::Screen:
		bActiveAxes[0] = bActiveAxes[1] = bActiveAxes[2] = true;
		break;
	case EAxisList::XY:
		bActiveAxes[0] = bActiveAxes[1] = true;
		break;
	case EAxisList::XZ:
		bActiveAxes[0] = bActiveAxes[2] = true;
		break;
	case EAxisList::YZ:
		bActiveAxes[1] = bActiveAxes[2] = true;
		break;
	}
}
/** Helper function for ModifyScale - Check scale criteria to see if this is allowed, returns modified absolute scale*/
static float CheckScaleValue(float ScaleDeltaToCheck, float CurrentScaleFactor, float CurrentExtent, bool bCheckSmallExtent, bool bSnap)
{
	float AbsoluteScaleValue = ScaleDeltaToCheck + CurrentScaleFactor;
	if (bSnap)
	{
		AbsoluteScaleValue = FMath::GridSnap(AbsoluteScaleValue, GSNAPSCALE/*GEditor->GetScaleGridSize()*/);
	}
	// In some situations CurrentExtent can be 0 (eg: when scaling a plane in Z), this causes a divide by 0 that we need to avoid.
	if (CurrentExtent < KINDA_SMALL_NUMBER) {
		return AbsoluteScaleValue;
	}
	float UnscaledExtent = CurrentExtent / CurrentScaleFactor;
	float ScaledExtent = UnscaledExtent * AbsoluteScaleValue;

	if ((FMath::Square(ScaledExtent)) > BIG_NUMBER)	// cant get too big...
	{
		return CurrentScaleFactor;
	}
	else if (bCheckSmallExtent &&
		(FMath::Abs(ScaledExtent) < MIN_ACTOR_BOUNDS_EXTENT * 0.5f ||		// ...or too small (apply sign in this case)...
		(CurrentScaleFactor < 0.0f) != (AbsoluteScaleValue < 0.0f)))	// ...also cant cross the zero boundary
	{
		return ((MIN_ACTOR_BOUNDS_EXTENT * 0.5) / UnscaledExtent) * (CurrentScaleFactor < 0.0f ? -1.0f : 1.0f);
	}

	return AbsoluteScaleValue;
}
static UCanvas* GetCanvasByName(FName CanvasName)
{
	// Cache to avoid FString/FName conversions/compares
	static TMap<FName, UCanvas*> CanvasMap;
	UCanvas** FoundCanvas = CanvasMap.Find(CanvasName);
	if (!FoundCanvas)
	{
		UCanvas* CanvasObject = FindObject<UCanvas>(GetTransientPackage(), *CanvasName.ToString());
		if (!CanvasObject)
		{
			CanvasObject = NewObject<UCanvas>(GetTransientPackage(), CanvasName);
			CanvasObject->AddToRoot();
		}

		CanvasMap.Add(CanvasName, CanvasObject);
		return CanvasObject;
	}

	return *FoundCanvas;
}

FXViewportCursorLocation::FXViewportCursorLocation(const FSceneView* View, UTGameViewportClient* InViewportClient, int32 X, int32 Y)
	: Origin(ForceInit), Direction(ForceInit), CursorPos(X, Y)
{
	FVector4 ScreenPos = View->PixelToScreen(X, Y, 0);

	const FMatrix InvViewMatrix = View->ViewMatrices.GetInvViewMatrix();
	const FMatrix InvProjMatrix = View->ViewMatrices.GetInvProjectionMatrix();

	const float ScreenX = ScreenPos.X;
	const float ScreenY = ScreenPos.Y;

	ViewportClient = InViewportClient;

	if (ViewportClient->IsPerspective())
	{
		Origin = View->ViewMatrices.GetViewOrigin();
		Direction = InvViewMatrix.TransformVector(FVector(InvProjMatrix.TransformFVector4(FVector4(ScreenX * GNearClippingPlane, ScreenY * GNearClippingPlane, 0.0f, GNearClippingPlane)))).GetSafeNormal();
	}
	else
	{
		Origin = InvViewMatrix.TransformFVector4(InvProjMatrix.TransformFVector4(FVector4(ScreenX, ScreenY, 0.5f, 1.0f)));
		Direction = InvViewMatrix.TransformVector(FVector(0, 0, 1)).GetSafeNormal();
	}
}

FXViewportCursorLocation::~FXViewportCursorLocation()
{
}

FXViewportClick::FXViewportClick(const FSceneView* View, UTGameViewportClient* ViewportClient, FKey InKey, EInputEvent InEvent, int32 X, int32 Y)
	: FXViewportCursorLocation(View, ViewportClient, X, Y)
	, Key(InKey), Event(InEvent)
{
	ControlDown = ViewportClient->IsCtrlPressed();
	ShiftDown = ViewportClient->IsShiftPressed();
	AltDown = ViewportClient->IsAltPressed();
}

FXViewportClick::~FXViewportClick()
{

}
FXViewportCameraTransform::FXViewportCameraTransform()
	: TransitionCurve(new FCurveSequence(0.0f, XFocusConstants::TransitionTime, ECurveEaseFunction::CubicOut))
	, ViewLocation(FVector::ZeroVector)
	, ViewRotation(FRotator::ZeroRotator)
	, DesiredLocation(FVector::ZeroVector)
	, LookAt(FVector::ZeroVector)
	, StartLocation(FVector::ZeroVector)
	, OrthoZoom(DEFAULT_ORTHOZOOM)
{}

void FXViewportCameraTransform::SetLocation(const FVector& Position)
{
	ViewLocation = Position;
	DesiredLocation = ViewLocation;
}

void FXViewportCameraTransform::TransitionToLocation(const FVector& InDesiredLocation, TWeakPtr<SWidget> EditorViewportWidget, bool bInstant)
{
	// 	if (bInstant || !EditorViewportWidget.IsValid())
	// 	{
	// 		SetLocation(InDesiredLocation);
	// 		TransitionCurve->JumpToEnd();
	// 	}
	// 	else
	// 	{
	// 		DesiredLocation = InDesiredLocation;
	// 		StartLocation = ViewLocation;
	// 
	// 		TransitionCurve->Play(EditorViewportWidget.Pin().ToSharedRef());
	// 	}
}

bool FXViewportCameraTransform::UpdateTransition()
{
	bool bIsAnimating = false;
	if (TransitionCurve->IsPlaying() || ViewLocation != DesiredLocation)
	{
		float LerpWeight = TransitionCurve->GetLerp();

		if (LerpWeight == 1.0f)
		{
			// Failsafe for the value not being exact on lerps
			ViewLocation = DesiredLocation;
		}
		else
		{
			ViewLocation = FMath::Lerp(StartLocation, DesiredLocation, LerpWeight);
		}


		bIsAnimating = true;
	}

	return bIsAnimating;
}

FMatrix FXViewportCameraTransform::ComputeOrbitMatrix() const
{
	FTransform Transform =
		FTransform(-LookAt) *
		FTransform(FRotator(0, ViewRotation.Yaw, 0)) *
		FTransform(FRotator(0, 0, ViewRotation.Pitch)) *
		FTransform(FVector(0, (ViewLocation - LookAt).Size(), 0));

	return Transform.ToMatrixNoScale() * FInverseRotationMatrix(FRotator(0, 90.f, 0));
}

bool FXViewportCameraTransform::IsPlaying()
{
	return TransitionCurve->IsPlaying();
}
/*------------------------*/
UTGameViewportClient::UTGameViewportClient(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	, ViewportType(EXLevelViewportType::LVT_Perspective)
	, bNeedsRedraw(true)
	, bNeedsInvalidateHitProxy(false)	
	, bEnableClick(true)
	, bUseControllingActorViewInfo(false)
	, bIsTracking(false)
	, ViewState()
	, ViewFOV(90.f)
	, FOVAngle(90.f)
	, AspectRatio(1.777777f)
	, CurrentMousePos(-1, -1)	
	, Widget(new FXWidget)	
	, Viewport(NULL)
	, bHasMouseMovedSinceClick(false)
	, bUserPlayerControllerView(false)
	, isAllowedRotateByMouse(false)		
	, bIsOrbit(false)
	, bShouldInvalidateViewportWidget(false)
	, bIsRealtime(false)
	, bShouldCheckHitProxy(false)
	, MouseScale(FVector2D(1, 1))
	//, bAdsorbed(false)
	//, bSnapTranslate(false)
	//, bSnapTranslateTemp(false)
	//, AdsorbDistance(15.f)	
	, MouseDeltaTracker(new FXMouseDeltaTracker)
	, bOnlyMovedPivot(false)
	, bForceHiddenWidget(false)
	, bUsesDrawHelper(true)
{
	ViewState.Allocate();
	ModeTools = &GXLevelEditorModeTools();
	ModeTools->SetWidgetMode(FXWidget::WM_Translate);
	Widget->SetUsesEditorModeTools(ModeTools);
}

UTGameViewportClient::~UTGameViewportClient()
{

}

void UTGameViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ConditionalCheckHoveredHitProxy();

}
void UTGameViewportClient::MouseEnter(FViewport* Viewport, int32 x, int32 y)
{
	//ModeTools->MouseEnter(this, Viewport, x, y);

	MouseMove(Viewport, x, y);
}
void UTGameViewportClient::MouseMove(FViewport* Viewport, int32 x, int32 y)
{
	check(IsInGameThread());

	CurrentMousePos = FIntPoint(x, y);

	//// Let the current editor mode know about the mouse movement.
	//ModeTools->MouseMove(this, Viewport, x, y);
}
void UTGameViewportClient::MouseLeave(FViewport* Viewport)
{
	check(IsInGameThread());

	//ModeTools->MouseLeave(this, Viewport);

	CurrentMousePos = FIntPoint(-1, -1);

	FCommonViewportClient::MouseLeave(Viewport);
}
void UTGameViewportClient::RenderDragTool(const FSceneView* View, FCanvas* Canvas)
{
	MouseDeltaTracker->RenderDragTool(View, Canvas);
}

void UTGameViewportClient::ConditionalCheckHoveredHitProxy()
{
	//bShouldCheckHitProxy = true;

	// If it has been decided that there is more important things to do than check hit proxies, then don't check them.
	if (!bShouldCheckHitProxy || bWidgetAxisControlledByDrag == true)
	{
		return;
	}
	//float Scale = EngineShowFlags.ScreenPercentage / 100.f;
	//const float XScale = (float)Viewport->.ViewRect.Max.X / (float)View.UnscaledViewRect.Max.X;
	//const float YScale = (float)View.ViewRect.Max.Y / (float)View.UnscaledViewRect.Max.Y;
	//HitX = HitX * XScale;
	//HitY = HitY * YScale;

	static const auto ScreenPercentageCVar = IConsoleManager::Get().FindTConsoleVariableDataFloat(TEXT("r.ScreenPercentage"));
	float Value = ScreenPercentageCVar->GetValueOnGameThread();
	float MouseX = CachedMouseX * Value / 100.f * MouseScale.X;
	float MouseY = CachedMouseY * Value / 100.f * MouseScale.Y;
	HHitProxy* HitProxy = Viewport->GetHitProxy(MouseX, MouseY);

	CheckHoveredHitProxy(HitProxy);

	// We need to set this to false here as if mouse is moved off viewport fast, it will keep doing CheckHoveredOverHitProxy for this viewport when it should not.
	bShouldCheckHitProxy = false;
}

/** FViewElementDrawer interface */
void UTGameViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (!bForceHiddenWidget)
		Widget->Render(View, PDI, this);
	//myl
	//if (bUsesDrawHelper)
	//	DrawHelper.Draw(View, PDI);

	// Remove temporary debug lines.
	// Possibly a hack. Lines may get added without the scene being rendered etc.
	if (World->LineBatcher != NULL && (World->LineBatcher->BatchedLines.Num() || World->LineBatcher->BatchedPoints.Num() || World->LineBatcher->BatchedMeshes.Num()))
	{
		World->LineBatcher->Flush();
	}

	if (World->ForegroundLineBatcher != NULL && (World->ForegroundLineBatcher->BatchedLines.Num() || World->ForegroundLineBatcher->BatchedPoints.Num() || World->ForegroundLineBatcher->BatchedMeshes.Num()))
	{
		World->ForegroundLineBatcher->Flush();
	}
}

void UTGameViewportClient::Draw(FViewport* InViewport, FCanvas* SceneCanvas)
{
	FViewport* ViewportBackup = Viewport;
	Viewport = InViewport ? InViewport : Viewport;
	FSceneView* NewView = NULL;
	
	//if (bUserPlayerControllerView)
	{
		Super::Draw(InViewport, SceneCanvas);
		return;
	}

	//创建命令框画布
	FIntPoint DebugCanvasSize = InViewport->GetSizeXY();
	UCanvas* DebugCanvasObject = GetCanvasByName("DebugCanvasObject");
	DebugCanvasObject->Init(DebugCanvasSize.X, DebugCanvasSize.Y, NULL, InViewport->GetDebugCanvas());

	//创建ViewFamily
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		//Viewport,
		SceneCanvas->GetRenderTarget(),
		GetScene(),
		EngineShowFlags)
		.SetRealtimeUpdate(true));

	//计算View
	NewView = CalcSceneView(&ViewFamily);
	//透视图
	if (IsPerspective())
	{
		ESplitScreenType::Type SplitScreenConfig = GetCurrentSplitscreenConfiguration();
		ViewFamily.ViewMode = VMI_Lit;
		EngineShowFlags.PostProcessing = true;
		EngineShowFlagOverride(ESFIM_Game, ViewFamily.ViewMode, ViewFamily.EngineShowFlags, SplitScreenConfig != ESplitScreenType::None);
		EngineShowFlagOrthographicOverride(IsPerspective(), ViewFamily.EngineShowFlags);
	}
	//其他正交视图
	else
	{
		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ESplitScreenType::Type SplitScreenConfig = GetCurrentSplitscreenConfiguration();
		ViewFamily.ViewMode = VMI_Unlit;
		EngineShowFlagOverride(ESFIM_Game, ViewFamily.ViewMode, ViewFamily.EngineShowFlags, SplitScreenConfig != ESplitScreenType::None);
		EngineShowFlagOrthographicOverride(IsPerspective(), ViewFamily.EngineShowFlags);
		ViewFamily.EngineShowFlags.SetAntiAliasing(true);
		ViewFamily.EngineShowFlags.SetWireframe(false);
		ViewFamily.EngineShowFlags.SetBloom(false);
		ViewFamily.EngineShowFlags.SetVisualizeHDR(false);
		ViewFamily.EngineShowFlags.PostProcessing = false;

	}

	//渲染调试命令
	PostRender(DebugCanvasObject);

	//渲染命令框
	if (ViewportConsole)
		ViewportConsole->PostRender_Console(DebugCanvasObject);

	//渲染调试命令
	DrawStatsHUD(GetWorld(), InViewport, InViewport->GetDebugCanvas(), DebugCanvasObject, DebugProperties, FVector::ZeroVector, FRotator::ZeroRotator);

	ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
		ViewFamily, /* GlobalResolutionFraction = */ 1.0f, /* AllowPostProcessSettingsScreenPercentage = */ true));

	//执行渲染命令
	GetRendererModule().BeginRenderingViewFamily(SceneCanvas, &ViewFamily);

	//if (NewView)
	//	RenderDragTool(NewView, SceneCanvas);

	//渲染坐标轴文字
	if (Widget)
		Widget->DrawHUD(SceneCanvas);

	//if (!IsPerspective() && bDrawAxes)
	//{
	//	const FRotator XYRot(-90.0f, -90.0f, 0.0f);
	//	DrawAxes(Viewport, SceneCanvas, &XYRot, EAxisList::XY);
	//	DrawScaleUnits(Viewport, SceneCanvas, *NewView);
	//}

	//Viewport = ViewportBackup;
}

void UTGameViewportClient::Invalidate(bool bInvalidateChildViews /*= true*/, bool bInvalidateHitProxies /*= true*/)
{
	if (Viewport)
	{
		if (bInvalidateHitProxies)
		{
			// Invalidate hit proxies and display pixels.
			Viewport->Invalidate();
		}
		else
		{
			// Invalidate only display pixels.
			Viewport->InvalidateDisplay();
		}
	}
}
void UTGameViewportClient::StopTracking()
{
	if (bIsTracking)
	{
		//cache the axis of any widget we might be moving
		EAxisList::Type DraggingAxis = EAxisList::None;
		if (Widget != NULL)
		{
			DraggingAxis = Widget->GetCurrentAxis();
		}

		MouseDeltaTracker->EndTracking(this);

		Widget->SetCurrentAxis(EAxisList::None);

		// Force an immediate redraw of the viewport and hit proxy.
		// The results are required straight away, so it is not sufficient to defer the redraw until the next tick.
		if (Viewport)
		{
			Viewport->InvalidateHitProxy();
			Viewport->Draw();

			// If there are child viewports, force a redraw on those too
			/*FSceneViewStateInterface* ParentView = ViewState.GetReference();
			if (ParentView->IsViewParent())
			{
			for (UXREditorViewportClient* ViewportClient : GEditor->AllViewportClients)
			{
			if (ViewportClient != nullptr)
			{
			FSceneViewStateInterface* ViewportParentView = ViewportClient->ViewState.GetReference();

			if (ViewportParentView != nullptr &&
			ViewportParentView->HasViewParent() &&
			ViewportParentView->GetViewParent() == ParentView &&
			!ViewportParentView->IsViewParent())
			{
			ViewportClient->Viewport->InvalidateHitProxy();
			ViewportClient->Viewport->Draw();
			}
			}
			}
			}*/
		}

		//SetRequiredCursorOverride(false);

		bWidgetAxisControlledByDrag = false;

		// Update the hovered hit proxy here.  If the user didnt move the mouse
		// they still need to be able to pick up the gizmo without moving the mouse again
		HHitProxy* HitProxy = Viewport->GetHitProxy(CachedMouseX, CachedMouseY);
		CheckHoveredHitProxy(HitProxy);

		bIsTracking = false;
	}

	bHasMouseMovedSinceClick = false;
}
void UTGameViewportClient::CheckHoveredHitProxy(HHitProxy* HoveredHitProxy)
{
	const EAxisList::Type SaveAxis = Widget->GetCurrentAxis();
	EAxisList::Type NewAxis = EAxisList::None;

	const bool LeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton) ? true : false;
	const bool MiddleMouseButtonDown = Viewport->KeyState(EKeys::MiddleMouseButton) ? true : false;
	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton) ? true : false;
	const bool bMouseButtonDown = (LeftMouseButtonDown || MiddleMouseButtonDown || RightMouseButtonDown);

	// Change the mouse cursor if the user is hovering over something they can interact with.
	if (HoveredHitProxy)
	{
		if (HoveredHitProxy->IsA(HXWidgetAxis::StaticGetType()) && !bUsingOrbitCamera && !bMouseButtonDown)
		{
			// In the case of the widget mode being overridden we can have a hit proxy
			// from the previous mode with an inappropriate axis for rotation.
			EAxisList::Type ProxyAxis = ((HXWidgetAxis*)HoveredHitProxy)->Axis;
			if (!IsOrtho() || GetWidgetMode() != FXWidget::WM_Rotate
				|| ProxyAxis == EAxisList::X || ProxyAxis == EAxisList::Y || ProxyAxis == EAxisList::Z)
			{
				NewAxis = ProxyAxis;
			}
			else
			{
				switch (GetViewportType())
				{
				case EXLevelViewportType::LVT_OnlyCanvas:
				case EXLevelViewportType::LVT_OrthoXY:
				case EXLevelViewportType::LVT_OrthoNegativeXY:
					NewAxis = EAxisList::Z;
					break;
				case EXLevelViewportType::LVT_OrthoXZ:
				case EXLevelViewportType::LVT_OrthoNegativeXZ:
					NewAxis = EAxisList::Y;
					break;
				case EXLevelViewportType::LVT_OrthoYZ:
				case EXLevelViewportType::LVT_OrthoNegativeYZ:
					NewAxis = EAxisList::X;
					break;
				default:
					break;
				}
			}
		}
		// If the current axis on the widget changed, repaint the viewport.
		if (NewAxis != SaveAxis)
		{
			SetCurrentWidgetAxis(NewAxis);
			//Invalidate(false, false);
		}
	}
}

bool UTGameViewportClient::IsVisible() const
{
	bool bIsVisible = false;

	if (VisibilityDelegate.IsBound())
		bIsVisible = VisibilityDelegate.Execute();

	return bIsVisible;
}
bool UTGameViewportClient::IsPerspective() const
{
	return (GetViewportType() == EXLevelViewportType::LVT_Perspective);
}

bool UTGameViewportClient::ShouldLockPitch() const
{
	return false;
	//myl
	//return CameraController->GetConfig().bLockedPitch;
}

bool UTGameViewportClient::ToggleRealtime()
{
	SetRealtime(!bIsRealtime);
	return bIsRealtime;
}

void UTGameViewportClient::SetRealtime(bool bInRealtime, bool bStoreCurrentValue)
{
	if (bStoreCurrentValue)
	{
		//Cache the Realtime and ShowStats flags
		bStoredRealtime = bIsRealtime;
		//bStoredShowStats = bShowStats;
	}

	bIsRealtime = bInRealtime;

	if (!bIsRealtime)
	{
		//SetShowStats(false);
	}
	else
	{
		bShouldInvalidateViewportWidget = true;
	}
}

void UTGameViewportClient::RestoreRealtime(const bool bAllowDisable)
{
	if (bAllowDisable)
	{
		bIsRealtime = bStoredRealtime;
		//bShowStats = bStoredShowStats;
	}
	else
	{
		bIsRealtime |= bStoredRealtime;
		//bShowStats |= bStoredShowStats;
	}

	if (bIsRealtime)
	{
		bShouldInvalidateViewportWidget = true;
	}
}

void UTGameViewportClient::OnDollyPerspectiveCamera(const struct FXInputEventState& InputState)
{
	FKey Key = InputState.GetKey();

	// Scrolling the mousewheel up/down moves the perspective viewport forwards/backwards.
	FVector Drag(0, 0, 0);

	const FRotator& ViewRotation = GetViewRotation();
	Drag.X = FMath::Cos(ViewRotation.Yaw * PI / 180.f) * FMath::Cos(ViewRotation.Pitch * PI / 180.f);
	Drag.Y = FMath::Sin(ViewRotation.Yaw * PI / 180.f) * FMath::Cos(ViewRotation.Pitch * PI / 180.f);
	Drag.Z = FMath::Sin(ViewRotation.Pitch * PI / 180.f);

	if (Key == EKeys::MouseScrollDown)
	{
		Drag = -Drag;
	}

	float CameraSpeed = 4.f;
	Drag *= CameraSpeed * 4.f;

	const bool bDollyCamera = true;
	MoveViewportCamera(Drag, FRotator::ZeroRotator, bDollyCamera);
}

void UTGameViewportClient::MoveViewportCamera(const FVector& InDrag, const FRotator& InRot, bool bDollyCamera)
{
	switch (GetViewportType())
	{
	case EXLevelViewportType::LVT_OnlyCanvas:
	case EXLevelViewportType::LVT_OrthoXY:
	case EXLevelViewportType::LVT_OrthoXZ:
	case EXLevelViewportType::LVT_OrthoYZ:
	case EXLevelViewportType::LVT_OrthoNegativeXY:
	case EXLevelViewportType::LVT_OrthoNegativeXZ:
	case EXLevelViewportType::LVT_OrthoNegativeYZ:
	{
		const bool LeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton);
		const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton);
		const bool bIsUsingTrackpad = FSlateApplication::Get().IsUsingTrackpad();

		if ((LeftMouseButtonDown || bIsUsingTrackpad) && RightMouseButtonDown)
		{
			SetOrthoZoom(GetOrthoZoom() + (GetOrthoZoom() / CAMERA_ZOOM_DAMPEN) * InDrag.Z);
			SetOrthoZoom(FMath::Clamp<float>(GetOrthoZoom(), MIN_ORTHOZOOM, MAX_ORTHOZOOM));
		}
		else
		{
			SetViewLocation(GetViewLocation() + InDrag);
		}

		// Update any linked orthographic viewports.
		UpdateLinkedOrthoViewports();
	}
	break;

	case EXLevelViewportType::LVT_OrthoFreelook:
	{
		SetViewLocation(GetViewLocation() + InDrag);
	}
	//@TODO: CAMERA: Not sure how to handle this
	break;

	case EXLevelViewportType::LVT_Perspective:
	{
		MoveViewportPerspectiveCamera(InDrag, InRot, bDollyCamera);
	}
	break;
	}
}
void UTGameViewportClient::MoveViewportPerspectiveCamera(const FVector& InDrag, const FRotator& InRot, bool bDollyCamera)
{
	check(IsPerspective());

	FVector ViewLocation = GetViewLocation();
	FRotator ViewRotation = GetViewRotation();

	if (ShouldLockPitch())
	{
		// Update camera Rotation
		ViewRotation += FRotator(InRot.Pitch, InRot.Yaw, InRot.Roll);

		// normalize to -180 to 180
		ViewRotation.Pitch = FRotator::NormalizeAxis(ViewRotation.Pitch);
		// Make sure its withing  +/- 90 degrees.
		ViewRotation.Pitch = FMath::Clamp(ViewRotation.Pitch, -90.f, 90.f);
	}
	else
	{
		//when not constraining the pitch (matinee feature) we need to rotate differently to avoid a gimbal lock
		const FRotator PitchRot(InRot.Pitch, 0, 0);
		const FRotator LateralRot(0, InRot.Yaw, InRot.Roll);

		//update lateral rotation
		ViewRotation += LateralRot;

		//update pitch separately using quaternions
		const FQuat ViewQuat = ViewRotation.Quaternion();
		const FQuat PitchQuat = PitchRot.Quaternion();
		const FQuat ResultQuat = ViewQuat * PitchQuat;

		//get our correctly rotated ViewRotation
		ViewRotation = ResultQuat.Rotator();
	}

	// Update camera Location
	ViewLocation += InDrag;

	if (!bDollyCamera)
	{
		const float DistanceToCurrentLookAt = FVector::Dist(GetViewLocation(), GetLookAtLocation());

		const FQuat CameraOrientation = FQuat::MakeFromEuler(ViewRotation.Euler());
		FVector Direction = CameraOrientation.RotateVector(FVector(1, 0, 0));

		SetLookAtLocation(ViewLocation + Direction * DistanceToCurrentLookAt);
	}

	SetViewLocation(ViewLocation);
	SetViewRotation(ViewRotation);

	if (bUsingOrbitCamera)
	{
		FVector LookAtPoint = GetLookAtLocation();
		const float DistanceToLookAt = FVector::Dist(ViewLocation, LookAtPoint);

		SetViewLocationForOrbiting(LookAtPoint, DistanceToLookAt);
	}

	PerspectiveCameraMoved();
}

void UTGameViewportClient::SetViewLocationForOrbiting(const FVector& LookAtPoint, float DistanceToCamera)
{
	FMatrix Matrix = FTranslationMatrix(-GetViewLocation());
	Matrix = Matrix * FInverseRotationMatrix(GetViewRotation());
	FMatrix CamRotMat = Matrix.InverseFast();
	FVector CamDir = FVector(CamRotMat.M[0][0], CamRotMat.M[0][1], CamRotMat.M[0][2]);
	SetViewLocation(LookAtPoint - DistanceToCamera * CamDir);
	SetLookAtLocation(LookAtPoint);
}

void UTGameViewportClient::RequestInvalidateHitProxy(FViewport* InViewport)
{
	bNeedsInvalidateHitProxy = true;
}

void UTGameViewportClient::RedrawRequested(FViewport* Viewport)
{
	bNeedsRedraw = true;
}

float UTGameViewportClient::GetOrthoUnitsPerPixel(const FViewport* InViewport) const
{
	const float SizeX = InViewport->GetSizeXY().X;
	// 15.0f was coming from the CAMERA_ZOOM_DIV marco, seems it was chosen arbitrarily
	return GetOrthoZoom() / SizeX;//(GetOrthoZoom() / (SizeX * 1.f)) * ComputeOrthoZoomFactor(SizeX);
}

FSceneInterface* UTGameViewportClient::GetScene() const
{
	UWorld* TempWorld = GetWorld();
	if (TempWorld)
	{
		return TempWorld->Scene;
	}

	return NULL;
}
FVector UTGameViewportClient::GetWidgetLocation() const
{
	return ModeTools->GetWidgetLocation();
}
FXWidget::EWidgetMode UTGameViewportClient::GetWidgetMode() const
{
	return ModeTools->GetWidgetMode();
}
float UTGameViewportClient::GetNearClipPlane() const
{
	return (NearPlane < 0.0f) ? GNearClippingPlane : NearPlane;
}
FMatrix UTGameViewportClient::GetWidgetCoordSystem() const
{
	return ModeTools->GetCustomInputCoordinateSystem();
}
EXCoordSystem UTGameViewportClient::GetWidgetCoordSystemSpace() const
{
	return ModeTools->GetCoordSystem();
}
void UTGameViewportClient::ModifyScale(AActor* InActor, FVector& ScaleDelta, bool bCheckSmallExtent /*= false*/) const
{
	if (InActor->GetRootComponent())
	{
		const FVector CurrentScale = InActor->GetRootComponent()->GetRelativeScale3D();

		const FBox LocalBox = InActor->GetComponentsBoundingBox(true);
		const FVector ScaledExtents = LocalBox.GetExtent() * CurrentScale;
		ValidateScale(CurrentScale, ScaledExtents, ScaleDelta, bCheckSmallExtent);

		if (ScaleDelta.IsNearlyZero())
		{
			ScaleDelta = FVector::ZeroVector;
		}
	}
}
void UTGameViewportClient::ValidateScale(const FVector& InCurrentScale, const FVector& InBoxExtent, FVector& InOutScaleDelta, bool bInCheckSmallExtent /*= false*/) const
{
	// get the axes that are active in this operation
	bool bActiveAxes[3];
	CheckActiveAxes(Widget != NULL ? Widget->GetCurrentAxis() : EAxisList::None, bActiveAxes);

	bool bSnapAxes = true;//OptionallyPreserveNonUniformScale(InCurrentScale, bActiveAxes, InOutScaleDelta);

	// check each axis
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		if (bActiveAxes[Axis])
		{
			float ModifiedScaleAbsolute = CheckScaleValue(InOutScaleDelta[Axis], InCurrentScale[Axis], InBoxExtent[Axis], bInCheckSmallExtent, bSnapAxes);
			InOutScaleDelta[Axis] = ModifiedScaleAbsolute - InCurrentScale[Axis];
		}
		else
		{
			InOutScaleDelta[Axis] = 0.0f;
		}
	}
}

void UTGameViewportClient::ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale)
{
	if ((InDrag.IsZero() && InRot.IsZero() && InScale.IsZero()))
	{
		return;
	}

	FVector ModifiedScale = InScale;

	TArray<AXGroupActor*> ActorGroups;
	TArray<AActor*> SelectActors;
	FBox SelectActorsBox(ForceInit);
	GXEngine->GetSelectedActors(SelectActors);
	for (auto AIt : SelectActors)
	{
		SelectActorsBox += GetActorCollisionBox(AIt);
	}

	//SelectActorsBox = GXREditor->GetSelectedActorsBox();


	FlowAxisBoxOfSelectedActors = SelectActorsBox;

	

	// Apply the deltas to any selected actors.
	for (FSelectionIterator SelectedActorIt(GXEngine->GetSelectedActorIterator()); SelectedActorIt; ++SelectedActorIt)
	{
		AActor* Actor = static_cast<AActor*>(*SelectedActorIt);
		checkSlow(Actor->IsA(AActor::StaticClass()));

		// Verify that the actor is in the same world as the viewport before moving it.
		// 		if (GEditor->PlayWorld)
		// 		{
		// 			if (bIsSimulateInEditorViewport)
		// 			{
		// 				// If the Actor's outer (level) outer (world) is not the PlayWorld then it cannot be moved in this viewport.
		// 				if (!(GEditor->PlayWorld == Actor->GetWorld()))
		// 				{
		// 					continue;
		// 				}
		// 			}
		// 			else if (!(GEditor->EditorWorld == Actor->GetWorld()))
		// 			{
		// 				continue;
		// 			}
		// 		}

		if (true)//!Actor->bLockLocation)
		{
			if (GXEngine->GetSelectedComponentCount() > 0)
			{
				// 				USelection* ComponentSelection = GEditor->GetSelectedComponents();
				// 
				// 				// Only move the parent-most component(s) that are selected 
				// 				// Otherwise, if both a parent and child are selected and the delta is applied to both, the child will actually move 2x delta
				// 				TInlineComponentArray<USceneComponent*> ComponentsToMove;
				// 				for (FSelectedEditableComponentIterator EditableComponentIt(GEditor->GetSelectedEditableComponentIterator()); EditableComponentIt; ++EditableComponentIt)
				// 				{
				// 					USceneComponent* SceneComponent = CastChecked<USceneComponent>(*EditableComponentIt);
				// 					if (SceneComponent)
				// 					{
				// 						USceneComponent* SelectedComponent = Cast<USceneComponent>(*EditableComponentIt);
				// 
				// 						// Check to see if any parent is selected
				// 						bool bParentAlsoSelected = false;
				// 						USceneComponent* Parent = SelectedComponent->GetAttachParent();
				// 						while (Parent != nullptr)
				// 						{
				// 							if (ComponentSelection->IsSelected(Parent))
				// 							{
				// 								bParentAlsoSelected = true;
				// 								break;
				// 							}
				// 
				// 							Parent = Parent->GetAttachParent();
				// 						}
				// 
				// 						// If no parent of this component is also in the selection set, move it!
				// 						if (!bParentAlsoSelected)
				// 						{
				// 							ComponentsToMove.Add(SelectedComponent);
				// 						}
				// 					}
				// 				}
				// 
				// 				// Now actually apply the delta to the appropriate component(s)
				// 				for (auto SceneComp : ComponentsToMove)
				// 				{
				// 					ApplyDeltaToComponent(SceneComp, InDrag, InRot, ModifiedScale);
				// 				}
			}
			else
			{
				AXGroupActor* ParentGroup = AXGroupActor::GetRootForActor(Actor, true, true);
				if (ParentGroup /*&& GVRSEditor->bGroupingActive*/)
				{
					ActorGroups.AddUnique(ParentGroup);
				}
				else
				{
					// Finally, verify that no actor in the parent hierarchy is also selected
					bool bHasParentInSelection = false;
					AActor* ParentActor = Actor->GetAttachParentActor();
					while (ParentActor != NULL && !bHasParentInSelection)
					{
						if (ParentActor->IsSelected())
						{
							bHasParentInSelection = true;
						}
						ParentActor = ParentActor->GetAttachParentActor();
					}
					if (!bHasParentInSelection)
					{	
						ApplyDeltaToActor(Actor, InDrag, InRot, ModifiedScale);
					}
				}
			}
		}
	}
	AXGroupActor::RemoveSubGroupsFromArray(ActorGroups);
	for (int32 ActorGroupsIndex = 0; ActorGroupsIndex < ActorGroups.Num(); ++ActorGroupsIndex)
	{
		ActorGroups[ActorGroupsIndex]->GroupApplyDelta(this, InDrag, InRot, ModifiedScale);
	}
}
FBox UTGameViewportClient::GetActorCollisionBox(AActor* _Actor) const
{
	FBox AggGeomBox(ForceInit);
	bool bFirst = true;
	AStaticMeshActor* SMA = Cast<AStaticMeshActor>(_Actor);

	bool bIsSkeletalMesh = false;
	FVector SkeletalBoxExtent = FVector::ZeroVector;
	//myl
	//UXRResourceManager* ResMgr = FXResourceModule::Get().GetResourceManager();
	//if (ResMgr)
	//{
	//	FVRSObject* Obj = ResMgr->GetObjFromObjID(_Actor->GetObjID());
	//	if (Obj)
	//	{
	//		if (Obj->GetSkeletalMeshObject())
	//		{
	//			bIsSkeletalMesh = true;
	//			SkeletalBoxExtent = Obj->BoundExtent;
	//		}
	//	}
	//}

	if (bIsSkeletalMesh)
	{
		FTransform Trans = _Actor->GetActorTransform();
		SkeletalBoxExtent *= Trans.GetScale3D();

		AggGeomBox += Trans.TransformPosition(-SkeletalBoxExtent);
		AggGeomBox += Trans.TransformPosition(FVector(-SkeletalBoxExtent.X, SkeletalBoxExtent.Y, -SkeletalBoxExtent.Z));
		AggGeomBox += Trans.TransformPosition(FVector(SkeletalBoxExtent.X, -SkeletalBoxExtent.Y, -SkeletalBoxExtent.Z));
		AggGeomBox += Trans.TransformPosition(FVector(SkeletalBoxExtent.X, SkeletalBoxExtent.Y, -SkeletalBoxExtent.Z));

		AggGeomBox += Trans.TransformPosition(SkeletalBoxExtent);
		AggGeomBox += Trans.TransformPosition(FVector(-SkeletalBoxExtent.X, SkeletalBoxExtent.Y, SkeletalBoxExtent.Z));
		AggGeomBox += Trans.TransformPosition(FVector(SkeletalBoxExtent.X, -SkeletalBoxExtent.Y, SkeletalBoxExtent.Z));
		AggGeomBox += Trans.TransformPosition(FVector(-SkeletalBoxExtent.X, -SkeletalBoxExtent.Y, SkeletalBoxExtent.Z));
	}
	else if (SMA)
	{
		UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent();
		if (SMC->GetStaticMesh() && SMC->GetStaticMesh()->BodySetup)
		{
			AggGeomBox = SMC->GetStaticMesh()->BodySetup->AggGeom.CalcAABB(SMC->GetComponentToWorld());
		}
		else
		{
			AggGeomBox = SMA->GetComponentsBoundingBox();
		}
	}
	else
	{
		UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(_Actor->GetRootComponent());
		if (SMC)
		{
			if (SMC->GetStaticMesh()->BodySetup)
			{
				AggGeomBox = SMC->GetStaticMesh()->BodySetup->AggGeom.CalcAABB(SMC->GetComponentToWorld());
			}
		}

		TArray<USceneComponent*> ChildList;
		_Actor->GetRootComponent()->GetChildrenComponents(true, ChildList);
		for (int32 i = 0; i < ChildList.Num(); i++)
		{
			UStaticMeshComponent* SMC2 = Cast<UStaticMeshComponent>(ChildList[i]);
			if (SMC2 && SMC2->GetStaticMesh())
			{
				if (SMC2->GetStaticMesh()->BodySetup)
				{
					FBox TempBox = SMC2->GetStaticMesh()->BodySetup->AggGeom.CalcAABB(SMC2->GetComponentToWorld());
					if (bFirst)
					{
						bFirst = false;
						AggGeomBox = TempBox;
					}
					else
					{
						//找到新的最大点 最小点
						if (TempBox.Max.X > AggGeomBox.Max.X)
							AggGeomBox.Max.X = TempBox.Max.X;
						if (TempBox.Max.Y > AggGeomBox.Max.Y)
							AggGeomBox.Max.Y = TempBox.Max.Y;
						if (TempBox.Max.Z > AggGeomBox.Max.Z)
							AggGeomBox.Max.Z = TempBox.Max.Z;

						if (TempBox.Min.X < AggGeomBox.Min.X)
							AggGeomBox.Min.X = TempBox.Min.X;
						if (TempBox.Min.Y < AggGeomBox.Min.Y)
							AggGeomBox.Min.Y = TempBox.Min.Y;
						if (TempBox.Min.Z < AggGeomBox.Min.Z)
							AggGeomBox.Min.Z = TempBox.Min.Z;
					}
				}
			}
		}
	}

	return AggGeomBox;
}

void UTGameViewportClient::ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale)
{
	FVector ModifiedDeltaScale = InDeltaScale;

	// we dont scale actors when we only have a very small scale change
	if (!InDeltaScale.IsNearlyZero())
	{
		if (true)
			//if (!GEditor->UsePercentageBasedScaling())
		{
			ModifyScale(InActor, ModifiedDeltaScale, false);
		}
	}
	else
	{
		ModifiedDeltaScale = FVector::ZeroVector;
	}

	GXEngine->ApplyDeltaToActor(
		InActor,
		true,
		&InDeltaDrag,
		&InDeltaRot,
		&ModifiedDeltaScale,
		IsAltPressed(),
		IsShiftPressed(),
		IsCtrlPressed());
}

void UTGameViewportClient::SetCurrentWidgetAxis(EAxisList::Type InAxis)
{
	Widget->SetCurrentAxis(InAxis);
}
EAxisList::Type UTGameViewportClient::GetCurrentWidgetAxis() const
{
	return Widget->GetCurrentAxis();
}
void UTGameViewportClient::TrackingStarted(const struct FXInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge)
{
	// Begin transacting.  Give the current editor mode an opportunity to do the transacting.
	const bool bTrackingHandledExternally = ModeTools->StartTracking(this, Viewport);

	//TrackingTransaction.End();

	// Re-initialize new tracking only if a new button was pressed, otherwise we continue the previous one.
	if (InInputState.GetInputEvent() == IE_Pressed)
	{
		EInputEvent Event = InInputState.GetInputEvent();
		FKey Key = InInputState.GetKey();

		if (InInputState.IsShiftButtonPressed() && bDraggingByHandle)
		{
			if (Event == IE_Pressed && (Key == EKeys::LeftMouseButton || Key == EKeys::RightMouseButton) && !bDuplicateActorsInProgress)
			{
				// Set the flag so that the actors actors will be duplicated as soon as the widget is displaced.
				bDuplicateOnNextDrag = true;
				bDuplicateActorsInProgress = true;
			}
		}
		else
		{
			bDuplicateOnNextDrag = false;
		}
	}

	bOnlyMovedPivot = false;

	const bool bIsDraggingComponents = GXEngine->GetSelectedComponentCount() > 0;
	//PreDragActorTransforms.Empty();
	if (bIsDraggingComponents)
	{
		if (bIsDraggingWidget)
		{
			//			Widget->SetSnapEnabled(true);
		}
	}
	else
	{
		for (FSelectionIterator It(GXEngine->GetSelectedActorIterator()); It && !bIsTrackingBrushModification; ++It)
		{
			AActor* Actor = static_cast<AActor*>(*It);
			checkSlow(Actor->IsA(AActor::StaticClass()));

			if (bIsDraggingWidget)
			{
				// Notify that this actor is beginning to move
				//				GVRSEditor->BroadcastBeginObjectMovement(*Actor);
			}

			//			Widget->SetSnapEnabled(true);

			// See if any brushes are about to be transformed via their Widget
			TArray<AActor*> AttachedActors;
			Actor->GetAttachedActors(AttachedActors);
			const bool bExactClass = true;
			// First, check for selected brush actors, check the actors attached actors for brush actors as well.  If a parent actor moves, the bsp needs to be rebuilt
			ABrush* Brush = Cast< ABrush >(Actor);
			//if (Brush && (!Brush->IsVolumeBrush() && !FActorEditorUtils::IsABuilderBrush(Actor)))
			//{
			//	bIsTrackingBrushModification = true;
			//}
			//else // Next, check for selected groups actors that contain brushes
			//{
			//AGroupActor* GroupActor = Cast<AGroupActor>(Actor);
			//if (GroupActor)
			//{
			//	TArray<AActor*> GroupMembers;
			//	GroupActor->GetAllChildren(GroupMembers, true);
			//	for (int32 GroupMemberIdx = 0; GroupMemberIdx < GroupMembers.Num(); ++GroupMemberIdx)
			//	{
			//		Brush = Cast< ABrush >(GroupMembers[GroupMemberIdx]);
			//		if (Brush && (!Brush->IsVolumeBrush() && false))//!FActorEditorUtils::IsABuilderBrush(Actor)))
			//		{
			//			bIsTrackingBrushModification = true;
			//		}
			//	}
			//}
			//}
		}
	}

	// Start a transformation transaction if required
	//if (!bTrackingHandledExternally)
	{
		if (bIsDraggingWidget)
		{
			TArray<AActor*> TheActors;
			GXEngine->GetSelectedActors(TheActors);
			if (TheActors.Num() > 0)
			{
				GXEngine->OnActorsOperation.ExecuteIfBound(TheActors, 0);
			}

			//TrackingTransaction.TransCount++;

			FText ObjectTypeBeingTracked;// = bIsDraggingComponents ? LOCTEXT("TransactionFocus_Components", "Components") : LOCTEXT("TransactionFocus_Actors", "Actors");
			FText TrackingDescription;

			switch (GetWidgetMode())
			{
			case FXWidget::WM_Translate:
				TrackingDescription = FText::Format(LOCTEXT("MoveTransaction", "Move {0}"), ObjectTypeBeingTracked);
				break;
			case FXWidget::WM_Rotate:
				TrackingDescription = FText::Format(LOCTEXT("RotateTransaction", "Rotate {0}"), ObjectTypeBeingTracked);
				break;
			case FXWidget::WM_Scale:
				TrackingDescription = FText::Format(LOCTEXT("ScaleTransaction", "Scale {0}"), ObjectTypeBeingTracked);
				break;
			case FXWidget::WM_TranslateRotateZ:
				TrackingDescription = FText::Format(LOCTEXT("TranslateRotateZTransaction", "Translate/RotateZ {0}"), ObjectTypeBeingTracked);
				break;
			case FXWidget::WM_2D:
				TrackingDescription = FText::Format(LOCTEXT("TranslateRotate2D", "Translate/Rotate2D {0}"), ObjectTypeBeingTracked);
				break;
			default:
				if (bNudge)
				{
					TrackingDescription = FText::Format(LOCTEXT("NudgeTransaction", "Nudge {0}"), ObjectTypeBeingTracked);
				}
			}

			if (!TrackingDescription.IsEmpty())
			{
				if (bNudge)
				{
					//TrackingTransaction.Begin(TrackingDescription);
				}
				else
				{
					// If this hasn't begun due to a nudge, start it as a pending transaction so that it only really begins when the mouse is moved
					//TrackingTransaction.BeginPending(TrackingDescription);
				}
			}

			//if (TrackingTransaction.IsActive() || TrackingTransaction.IsPending())
			//{
			// Suspend actor/component modification during each delta step to avoid recording unnecessary overhead into the transaction buffer
			//GEditor->DisableDeltaModification(true);
			//}
		}
	}
}

void UTGameViewportClient::TrackingStopped()
{
	const bool AltDown = IsAltPressed();
	const bool ShiftDown = IsShiftPressed();
	const bool ControlDown = IsCtrlPressed();
	const bool LeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton);
	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton);
	const bool MiddleMouseButtonDown = Viewport->KeyState(EKeys::MiddleMouseButton);

	// Only disable the duplicate on next drag flag if we actually dragged the mouse.
	bDuplicateOnNextDrag = false;

	// here we check to see if anything of worth actually changed when ending our MouseMovement
	// If the TransCount > 0 (we changed something of value) so we need to call PostEditMove() on stuff
	// if we didn't change anything then don't call PostEditMove()
	bool bDidAnythingActuallyChange = false;

	// Stop transacting.  Give the current editor mode an opportunity to do the transacting.
	const bool bTransactingHandledByEditorMode = ModeTools->EndTracking(this, Viewport);
	/*if (!bTransactingHandledByEditorMode)
	{
	if (TrackingTransaction.TransCount > 0)
	{
	bDidAnythingActuallyChange = true;
	TrackingTransaction.TransCount--;
	}
	}*/

	// Finish tracking a brush transform and update the Bsp
	if (bIsTrackingBrushModification)
	{
		//bDidAnythingActuallyChange = HaveSelectedObjectsBeenChanged() && !bOnlyMovedPivot;

		bIsTrackingBrushModification = false;
		if (bDidAnythingActuallyChange && bWidgetAxisControlledByDrag)
		{
			//GEditor->RebuildAlteredBSP();
		}
	}

	// Notify the selected actors that they have been moved.
	// Don't do this if AddDelta was never called.
	TArray<AXGroupActor*> ActorGroups;
	if (/*bDidAnythingActuallyChange && */MouseDeltaTracker->HasReceivedDelta())
	{
		for (FSelectionIterator It(GXEngine->GetSelectedActorIterator()); It; ++It)
		{
			AActor* Actor = static_cast<AActor*>(*It);
			checkSlow(Actor->IsA(AActor::StaticClass()));

			// Verify that the actor is in the same world as the viewport before moving it.
			//if (GEditor->PlayWorld)
			//{
			//	if (bIsSimulateInEditorViewport)
			//	{
			//		// If the Actor's outer (level) outer (world) is not the PlayWorld then it cannot be moved in this viewport.
			//		if (!(GEditor->PlayWorld == Actor->GetOuter()->GetOuter()))
			//		{
			//			continue;
			//		}
			//	}
			//	else if (!(GEditor->EditorWorld == Actor->GetOuter()->GetOuter()))
			//	{
			//		continue;
			//	}
			//}
			AXGroupActor* ParentGroup = AXGroupActor::GetRootForActor(Actor, true, true);
			if (ParentGroup /*&& GVRSEditor->bGroupingActive*/)
			{
				ActorGroups.AddUnique(ParentGroup);
			}
			else
			{

				Actor->PostEditMove(true);
				GXEngine->BroadcastEndObjectMovement(*Actor);
			}
		}
		AXGroupActor::RemoveSubGroupsFromArray(ActorGroups);
		for (int32 ActorGroupsIndex = 0; ActorGroupsIndex < ActorGroups.Num(); ++ActorGroupsIndex)
		{
			ActorGroups[ActorGroupsIndex]->PostEditMove(true);
		}
	}
}

FSceneView* UTGameViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass)
{
	const bool bStereoRendering = StereoPass != eSSP_FULL;

	FSceneViewInitOptions ViewInitOptions;

	// Takes care of HighDPI based screen percentage in editor viewport when not in VR editor.
	if (!bStereoRendering)
	{
		// Disables any screen percentage derived for game such as r.ScreenPercentage or FPostProcessSettings::ScreenPercentage.
		//ViewInitOptions.bDisableGameScreenPercentage = true;

		// Forces screen percentage showflag on so that we always upscale on HighDPI configuration.
		ViewFamily->EngineShowFlags.ScreenPercentage = true;
	}

	FXViewportCameraTransform& ViewTransform = GetViewTransform();
	const EXLevelViewportType::Type EffectiveViewportType = GetViewportType();

	ViewInitOptions.ViewOrigin = ViewTransform.GetLocation();
	const FRotator& ViewRotation = ViewTransform.GetRotation();

	
	const FIntPoint ViewportSizeXY = Viewport->GetSizeXY();

	FIntRect ViewRect = FIntRect(0, 0, ViewportSizeXY.X, ViewportSizeXY.Y);
	ViewInitOptions.SetViewRectangle(ViewRect);
	

	const bool bConstrainAspectRatio = bUseControllingActorViewInfo;// && ControllingActorViewInfo.bConstrainAspectRatio;
	const EAspectRatioAxisConstraint AspectRatioAxisConstraint = EAspectRatioAxisConstraint::AspectRatio_MaintainXFOV;//GetDefault<ULevelEditorViewportSettings>()->AspectRatioAxisConstraint;

	if (bUseControllingActorViewInfo)
	{
		ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewRotation) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));

		//FMinimalViewInfo::CalculateProjectionMatrixGivenView(ControllingActorViewInfo, AspectRatioAxisConstraint, Viewport, /*inout*/ ViewInitOptions);
	}
	else
	{
		//
		if (EffectiveViewportType == EXLevelViewportType::LVT_Perspective)
		{
			if (bUsingOrbitCamera)
			{
				ViewInitOptions.ViewRotationMatrix = FTranslationMatrix(ViewInitOptions.ViewOrigin) * ViewTransform.ComputeOrbitMatrix();
			}
			else
			{
				ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewRotation);
			}

			ViewInitOptions.ViewRotationMatrix = ViewInitOptions.ViewRotationMatrix * FMatrix(
				FPlane(0, 0, 1, 0),
				FPlane(1, 0, 0, 0),
				FPlane(0, 1, 0, 0),
				FPlane(0, 0, 0, 1));

			float MinZ = GetNearClipPlane();
			float MaxZ = MinZ;
			// Avoid zero ViewFOV's which cause divide by zero's in projection matrix
			float MatrixFOV = FMath::Max(0.001f, ViewFOV) * (float)PI / 360.0f;

			if (bConstrainAspectRatio)
			{
				if ((int32)ERHIZBuffer::IsInverted != 0)
				{
					ViewInitOptions.ProjectionMatrix = FReversedZPerspectiveMatrix(
						MatrixFOV,
						MatrixFOV,
						1.0f,
						AspectRatio,
						MinZ,
						MaxZ
					);
				}
				else
				{
					ViewInitOptions.ProjectionMatrix = FPerspectiveMatrix(
						MatrixFOV,
						MatrixFOV,
						1.0f,
						AspectRatio,
						MinZ,
						MaxZ
					);
				}
			}
			else
			{
				float XAxisMultiplier;
				float YAxisMultiplier;

				if (((ViewportSizeXY.X > ViewportSizeXY.Y) && (AspectRatioAxisConstraint == AspectRatio_MajorAxisFOV)) || (AspectRatioAxisConstraint == AspectRatio_MaintainXFOV))
				{
					//if the viewport is wider than it is tall
					XAxisMultiplier = 1.0f;
					YAxisMultiplier = ViewportSizeXY.X / (float)ViewportSizeXY.Y;
				}
				else
				{
					//if the viewport is taller than it is wide
					XAxisMultiplier = ViewportSizeXY.Y / (float)ViewportSizeXY.X;
					YAxisMultiplier = 1.0f;
				}

				if ((int32)ERHIZBuffer::IsInverted != 0)
				{
					ViewInitOptions.ProjectionMatrix = FReversedZPerspectiveMatrix(
						MatrixFOV,
						MatrixFOV,
						XAxisMultiplier,
						YAxisMultiplier,
						MinZ,
						MaxZ
					);
				}
				else
				{
					ViewInitOptions.ProjectionMatrix = FPerspectiveMatrix(
						MatrixFOV,
						MatrixFOV,
						XAxisMultiplier,
						YAxisMultiplier,
						MinZ,
						MaxZ
					);
				}
			}
		}
		else
		{
			static_assert((int32)ERHIZBuffer::IsInverted != 0, "Check all the Rotation Matrix transformations!");
			float ZScale = 0.5f / HALF_WORLD_MAX;
			float ZOffset = HALF_WORLD_MAX;
			//The divisor for the matrix needs to match the translation code.
			const float Zoom = GetOrthoUnitsPerPixel(Viewport);

			float OrthoWidth = Zoom * ViewportSizeXY.X / 2.0f;
			float OrthoHeight = Zoom * ViewportSizeXY.Y / 2.0f;

			if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoXY || EffectiveViewportType == EXLevelViewportType::LVT_OnlyCanvas)
			{
				ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewRotation);

				ViewInitOptions.ViewRotationMatrix = /*ViewInitOptions.ViewRotationMatrix * */FMatrix(
					FPlane(1, 0, 0, 0),
					FPlane(0, -1, 0, 0),
					FPlane(0, 0, -1, 0),
					FPlane(0, 0, -ViewInitOptions.ViewOrigin.Z, 1));
			}
			else if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoXZ)
			{
				ViewInitOptions.ViewRotationMatrix = FMatrix(
					FPlane(1, 0, 0, 0),
					FPlane(0, 0, -1, 0),
					FPlane(0, 1, 0, 0),
					FPlane(0, 0, -ViewInitOptions.ViewOrigin.Y, 1));
			}
			else if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoYZ)
			{
				ViewInitOptions.ViewRotationMatrix = FMatrix(
					FPlane(0, 0, 1, 0),
					FPlane(1, 0, 0, 0),
					FPlane(0, 1, 0, 0),
					FPlane(0, 0, ViewInitOptions.ViewOrigin.X, 1));
			}
			else if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoNegativeXY)
			{
				ViewInitOptions.ViewRotationMatrix = FMatrix(
					FPlane(-1, 0, 0, 0),
					FPlane(0, -1, 0, 0),
					FPlane(0, 0, 1, 0),
					FPlane(0, 0, -ViewInitOptions.ViewOrigin.Z, 1));
			}
			else if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoNegativeXZ)
			{
				ViewInitOptions.ViewRotationMatrix = FMatrix(
					FPlane(-1, 0, 0, 0),
					FPlane(0, 0, 1, 0),
					FPlane(0, 1, 0, 0),
					FPlane(0, 0, -ViewInitOptions.ViewOrigin.Y, 1));
			}
			else if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoNegativeYZ)
			{
				ViewInitOptions.ViewRotationMatrix = FMatrix(
					FPlane(0, 0, -1, 0),
					FPlane(-1, 0, 0, 0),
					FPlane(0, 1, 0, 0),
					FPlane(0, 0, ViewInitOptions.ViewOrigin.X, 1));
			}
			else if (EffectiveViewportType == EXLevelViewportType::LVT_OrthoFreelook)
			{
				ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewRotation);
				ViewInitOptions.ViewRotationMatrix = ViewInitOptions.ViewRotationMatrix*FMatrix(
					FPlane(0, 0, 1, 0),
					FPlane(1, 0, 0, 0),
					FPlane(0, 1, 0, 0),
					FPlane(0, 0, 0, 1));
			}
			else
			{
				// Unknown viewport type
				check(false);
			}

			ViewInitOptions.ProjectionMatrix = FReversedZOrthoMatrix(
				OrthoWidth,
				OrthoHeight,
				ZScale,
				ZOffset
			);
		}

		if (bConstrainAspectRatio)
		{
			ViewInitOptions.SetConstrainedViewRectangle(Viewport->CalculateViewExtents(AspectRatio, ViewRect));
		}
	}

	ViewInitOptions.ViewFamily = ViewFamily;
	ViewInitOptions.SceneViewStateInterface = ViewState.GetReference();
	ViewInitOptions.StereoPass = StereoPass;

	ViewInitOptions.ViewElementDrawer = this;

	ViewInitOptions.BackgroundColor = GetBackgroundColor();

	if (bUseControllingActorViewInfo)
	{
		ViewInitOptions.bUseFieldOfViewForLOD = false;// ControllingActorViewInfo.bUseFieldOfViewForLOD;
	}

	ViewInitOptions.OverrideFarClippingPlaneDistance = FarPlane;
	ViewInitOptions.CursorPos = CurrentMousePos;

	FSceneView* View = new FSceneView(ViewInitOptions);

	View->ViewLocation = ViewTransform.GetLocation();
	View->ViewRotation = ViewRotation;// FRotator(0, 0, 0);
#if WITH_EDITOR
	View->SubduedSelectionOutlineColor = GEngine->GetSubduedSelectionOutlineColor();
#endif
	ViewFamily->Views.Add(View);

	View->StartFinalPostprocessSettings(View->ViewLocation);

//	OverridePostProcessSettings(*View);


	View->EndFinalPostprocessSettings(ViewInitOptions);
#if WITH_EDITOR
	MouseScale.X = (float)View->UnscaledViewRect.Max.X / (float)View->UnscaledViewRect.Max.X;
	MouseScale.Y = (float)View->UnscaledViewRect.Max.Y / (float)View->UnscaledViewRect.Max.Y;
#endif
	return View;
}

void UTGameViewportClient::StartTrackingDueToInput(const struct FXInputEventState& InputState)
{
	// Check to see if the current event is a modifier key and that key was already in the
	// same state.
	EInputEvent Event = InputState.GetInputEvent();
	FViewport* InputStateViewport = InputState.GetViewport();
	FKey Key = InputState.GetKey();

	bool bIsRedundantModifierEvent =
		(InputState.IsAltButtonEvent() && ((Event != IE_Released) == IsAltPressed())) ||
		(InputState.IsCtrlButtonEvent() && ((Event != IE_Released) == IsCtrlPressed())) ||
		(InputState.IsShiftButtonEvent() && ((Event != IE_Released) == IsShiftPressed()));

	if (MouseDeltaTracker->UsingDragTool() && InputState.IsLeftMouseButtonPressed() && Event != IE_Released)
	{
		bIsRedundantModifierEvent = true;
	}

	const int32	HitX = InputStateViewport->GetMouseX();
	const int32	HitY = InputStateViewport->GetMouseY();


	//First mouse down, note where they clicked
	LastMouseX = HitX;
	LastMouseY = HitY;

	WorldStartPos = PixelToWorld(HitX, HitY, 0);
	WorldEndPos = PixelToWorld(HitX, HitY, 0);

	// Only start (or restart) tracking mode if the current event wasn't a modifier key that
	// was already pressed or released.
	if (!bIsRedundantModifierEvent)
	{
		const bool bWasTracking = bIsTracking;

		// Stop current tracking
		if (bIsTracking)
		{
			MouseDeltaTracker->EndTracking(this);
			bIsTracking = false;
		}

		bDraggingByHandle = (Widget && Widget->GetCurrentAxis() != EAxisList::None);

		if (Event == IE_Pressed)
		{
			// Tracking initialization:
			GXEngine->MouseMovement = FVector::ZeroVector;
		}

		// Start new tracking. Potentially reset the widget so that StartTracking can pick a new axis.
		if (Widget && (!bDraggingByHandle || InputState.IsCtrlButtonPressed()))
		{
			bWidgetAxisControlledByDrag = false;
			Widget->SetCurrentAxis(EAxisList::None);
		}
		const bool bNudge = false;
		MouseDeltaTracker->StartTracking(this, HitX, HitY, InputState, bNudge, !bWasTracking);
		bIsTracking = true;

		//if we are using a widget to drag by axis ensure the cursor is correct
		if (bDraggingByHandle == true)
		{
			//reset the flag to say we used a drag modifier	if we are using the widget handle
			if (bWidgetAxisControlledByDrag == false)
			{
				//MouseDeltaTracker->ResetUsedDragModifier();
			}

			//SetRequiredCursorOverride(true, EMouseCursor::CardinalCross);
		}

		//only reset the initial point when the mouse is actually clicked
		if (InputState.IsAnyMouseButtonDown() && Widget)
		{
			Widget->ResetInitialTranslationOffset();
		}

		//Don't update the cursor visibility if we don't have focus or mouse capture 
		if (InputStateViewport->HasFocus() || InputStateViewport->HasMouseCapture())
		{
			//Need to call this one more time as the axis variable for the widget has just been updated
			//UpdateRequiredCursorVisibility();
		}
	}
	//ApplyRequiredCursorVisibility(true);
}
FVector UTGameViewportClient::TranslateDelta(FKey InKey, float InDelta, bool InNudge)
{
	const bool LeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton);
	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton);
	const bool bIsUsingTrackpad = FSlateApplication::Get().IsUsingTrackpad();

	FVector vec(0.0f, 0.0f, 0.0f);

	float X = InKey == EKeys::MouseX ? InDelta : 0.f;
	float Y = InKey == EKeys::MouseY ? InDelta : 0.f;

	switch (GetViewportType())
	{
	case EXLevelViewportType::LVT_OnlyCanvas:
	case EXLevelViewportType::LVT_OrthoXY:
	{
		LastMouseX -= X;
		LastMouseY += Y;

		//if ((X != 0.0f) || (Y != 0.0f))
		//{
		//	MarkMouseMovedSinceClick();
		//}

		////only invert x,y if we're moving the camera
		//if (ShouldUseMoveCanvasMovement())
		//{
		//	if (Widget->GetCurrentAxis() == EAxisList::None)
		//	{
		//		X = -X;
		//		Y = -Y;
		//	}
		//}

		//update the position
		Viewport->SetSoftwareCursorPosition(FVector2D(LastMouseX, LastMouseY));
		//UE_LOG(LogEditorViewport, Log, *FString::Printf( TEXT("can:%d %d") , LastMouseX , LastMouseY ));
		//change to grab hand
		//SetRequiredCursorOverride(true, EMouseCursor::CardinalCross);
		//update and apply cursor visibility
		//UpdateAndApplyCursorVisibility();

		FXWidget::EWidgetMode WidgetMode = GetWidgetMode();
		bool bIgnoreOrthoScaling = (WidgetMode == FXWidget::WM_Scale) && (Widget->GetCurrentAxis() != EAxisList::None);

		if (InNudge || bIgnoreOrthoScaling)
		{
			vec = FVector(X, Y, 0.f);
		}
		else
		{
			const float UnitsPerPixel = GetOrthoUnitsPerPixel(Viewport);
			vec = FVector(X * UnitsPerPixel, Y * UnitsPerPixel, 0.f);
			vec *= 1.5f;
			if (Widget->GetCurrentAxis() == EAxisList::None)
			{
				switch (GetViewportType())
				{
				case EXLevelViewportType::LVT_OrthoXY:
					vec.Y *= -1.0f;
					break;
				case EXLevelViewportType::LVT_OrthoXZ:
					vec = FVector(X * UnitsPerPixel, 0.f, Y * UnitsPerPixel);
					break;
				case EXLevelViewportType::LVT_OrthoYZ:
					vec = FVector(0.f, X * UnitsPerPixel, Y * UnitsPerPixel);
					break;
				case EXLevelViewportType::LVT_OrthoNegativeXY:
					vec = FVector(-X * UnitsPerPixel, -Y * UnitsPerPixel, 0.0f);
					break;
				case EXLevelViewportType::LVT_OrthoNegativeXZ:
					vec = FVector(-X * UnitsPerPixel, 0.f, Y * UnitsPerPixel);
					break;
				case EXLevelViewportType::LVT_OrthoNegativeYZ:
					vec = FVector(0.f, -X * UnitsPerPixel, Y * UnitsPerPixel);
					break;
				case EXLevelViewportType::LVT_OrthoFreelook:
				case EXLevelViewportType::LVT_Perspective:
					break;
				}
			}
		}
	}
	break;
	case EXLevelViewportType::LVT_Perspective:
		// Update the software cursor position
		Viewport->SetSoftwareCursorPosition(FVector2D(Viewport->GetMouseX(), Viewport->GetMouseY()));
		vec = FVector(X, Y, 0.f);
		break;
	default:
		check(0);		// Unknown viewport type
		break;
	}

	return vec;
}
bool UTGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	//return Super::InputKey(EventArgs);
	

	bool bHandled = false;

	if (IgnoreInput())
	{
		return ViewportConsole ? ViewportConsole->InputKey(EventArgs.ControllerId, EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.IsGamepad()) : false;
	}
	
	// route to subsystems that care
	bHandled = (ViewportConsole ? ViewportConsole->InputKey(EventArgs.ControllerId, EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.IsGamepad()) : false);

	if (!bHandled)
	{
		ULocalPlayer* const TargetPlayer = GEngine->GetLocalPlayerFromControllerId(this, EventArgs.ControllerId);
		if (TargetPlayer && TargetPlayer->PlayerController)
		{
			TargetPlayer->PlayerController->InputKey(EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.IsGamepad());
		}
	}

	//bHandled = Super::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed, bGamepad);

	if (bHandled)
		return bHandled;

	if (!bEnableClick)
		return false;

	FXInputEventState InputState(EventArgs.Viewport, EventArgs.Key, EventArgs.Event);
	//bSnapTranslateTemp = InputState.IsAltButtonPressed();

	// Compute a view.
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		EventArgs.Viewport,
		GetScene(),
		EngineShowFlags)
		.SetRealtimeUpdate(true));
	FSceneView* View = CalcSceneView(&ViewFamily);

	int32 HitX = EventArgs.Viewport->GetMouseX();
	int32 HitY = EventArgs.Viewport->GetMouseY();

	const float XScale = (float)View->UnscaledViewRect.Max.X / (float)View->UnscaledViewRect.Max.X;
	const float YScale = (float)View->UnscaledViewRect.Max.Y / (float)View->UnscaledViewRect.Max.Y;
	HitX = HitX * XScale;
	HitY = HitY * YScale;

	// Compute the click location.
	if (InputState.IsAnyMouseButtonDown())
	{
		const FXViewportCursorLocation Cursor(View, this, HitX, HitY);
		const FXActorPositionTraceResult TraceResult = FXActorPositioning::TraceWorldForPositionWithDefault(Cursor, *View);
		//GXEngine->UnsnappedClickLocation = TraceResult.Location;
		GXEngine->ClickLocation = TraceResult.Location;
		GXEngine->ClickPlane = FPlane(TraceResult.Location, TraceResult.SurfaceNormal);
	}

	// Start tracking if any mouse button is down and it was a tracking event (MouseButton/Ctrl/Shift/Alt):
	if (InputState.IsAnyMouseButtonDown()
		&& (EventArgs.Event == IE_Pressed || EventArgs.Event == IE_Released)
		&& (InputState.IsMouseButtonEvent() || InputState.IsCtrlButtonEvent() || InputState.IsAltButtonEvent() || InputState.IsShiftButtonEvent()))
	{

		StartTrackingDueToInput(InputState);
		return true;
	}
	// If we are tracking and no mouse button is down and this input event released the mouse button stop tracking and process any clicks if necessary
	if (bIsTracking && !InputState.IsAnyMouseButtonDown() && InputState.IsMouseButtonEvent())
	{
		// Handle possible mouse click viewport
		ProcessClickInViewport(InputState, *View);

		// Stop tracking if no mouse button is down
		StopTracking();

		bHandled |= true;
	}

	if ((EventArgs.Key == EKeys::MouseScrollUp || EventArgs.Key == EKeys::MouseScrollDown || EventArgs.Key == EKeys::Add || EventArgs.Key == EKeys::Subtract) && (EventArgs.Event == IE_Pressed || EventArgs.Event == IE_Repeat))
	{
		//选中物体时滑动鼠标中键可以旋转物体
		if (GXEngine->GetSelectedActorCount() > 0 && CanMouseRotate())
		{
			TArray<AActor*> SelectedActors;
			bool bCanRotation = true;
			for (FSelectionIterator It(GXEngine->GetSelectedActorIterator()); It; ++It)
			{
				if (AActor* Actor = Cast<AActor>(*It))
				{
					SelectedActors.Add(Actor);
					if (Actor->Tags.Contains(TEXT("HydropowerActor")) || Actor->Tags.Contains(TEXT("HydropowerPipeActor"))
						|| Actor->Tags.Contains(TEXT("AreaHighLight")) || Actor->Tags.Contains(TEXT("Immovable")))//@梁晓菲 门窗墙放样线条在立面模式下不可以旋转
						bCanRotation = false;
				}
			}
			if (bCanRotation)
				ApplyDeltaToActors(FVector(0, 0, 0), FRotator(0, EventArgs.Key == EKeys::MouseScrollUp ? 5 : -5, 0), FVector(0, 0, 0));
			bHandled |= true;
		}
		// 3D场景下缩放
		else if (IsPerspective())
		{
			OnDollyPerspectiveCamera(InputState);
		}
	}


	// Clear Duplicate Actors mode when ALT and all mouse buttons are released
	if (!InputState.IsAltButtonPressed() && !InputState.IsAnyMouseButtonDown())
	{
		bDuplicateActorsInProgress = false;
	}

	return true;
}
void UTGameViewportClient::ProcessClickInViewport(const FXInputEventState& InputState, FSceneView& View)
{
	// Ignore actor manipulation if we're using a tool
	if (!MouseDeltaTracker->UsingDragTool())
	{
		EInputEvent Event = InputState.GetInputEvent();
		FViewport* InputStateViewport = InputState.GetViewport();
		FKey Key = InputState.GetKey();

		int32	HitX = InputStateViewport->GetMouseX();
		int32	HitY = InputStateViewport->GetMouseY();

		const float XScale = (float)View.UnscaledViewRect.Max.X / (float)View.UnscaledViewRect.Max.X;
		const float YScale = (float)View.UnscaledViewRect.Max.Y / (float)View.UnscaledViewRect.Max.Y;
		HitX = HitX * XScale;
		HitY = HitY * YScale;
		// Calc the raw delta from the mouse to detect if there was any movement
		FVector RawMouseDelta = MouseDeltaTracker->GetRawDelta();

		// Note: We are using raw mouse movement to double check distance moved in low performance situations.  In low performance situations its possible 
		// that we would get a mouse down and a mouse up before the next tick where GEditor->MouseMovment has not been updated.  
		// In that situation, legitimate drags are incorrectly considered clicks
		bool bNoMouseMovment = RawMouseDelta.SizeSquared() < MOUSE_CLICK_DRAG_DELTA && GXEngine->MouseMovement.SizeSquared() < MOUSE_CLICK_DRAG_DELTA;

		// If the mouse haven't moved too far, treat the button release as a click.
		if (bNoMouseMovment)// && !MouseDeltaTracker->WasExternalMovement())
		{

			Viewport->InvalidateHitProxy();
			HHitProxy* HitProxy = InputStateViewport->GetHitProxy(HitX, HitY);
			// When clicking, the cursor should always appear at the location of the click and not move out from undere the user
			InputStateViewport->SetPreCaptureMousePosFromSlateCursor();
			ProcessClick(View, HitProxy, Key, Event, HitX, HitY);
		}
	}
}

void UTGameViewportClient::ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FXViewportClick Click(&View, this, Key, Event, HitX, HitY);

	bool bRightMouseButtonPressed = Key == EKeys::RightMouseButton;

	if (HitProxy == NULL)
	{
		XClickHandlers::ClickBackdrop(this, Click);
		//if (bRightMouseButtonPressed && DroppedObjects.Num() > 0)
		//{
		//	CancelDrop();
		//}
	}
	else if(HitProxy->Priority >= 0 && HitProxy->OrthoPriority >= 0)
	{
		if (HitProxy->IsA(HXWidgetAxis::StaticGetType()))
		{
			// The user clicked on an axis translation/rotation hit proxy.  However, we want
			// to find out what's underneath the axis widget.  To do this, we'll need to render
			// the viewport's hit proxies again, this time *without* the axis widgets!

			// OK, we need to be a bit evil right here.  Basically we want to hijack the ShowFlags
			// for the scene so we can re-render the hit proxies without any axis widgets.  We'll
			// store the original ShowFlags and modify them appropriately
			const bool bOldModeWidgets1 = EngineShowFlags.ModeWidgets;
			const bool bOldModeWidgets2 = View.Family->EngineShowFlags.ModeWidgets;

			EngineShowFlags.SetModeWidgets(false);
			FSceneViewFamily* SceneViewFamily = const_cast<FSceneViewFamily*>(View.Family);
			SceneViewFamily->EngineShowFlags.SetModeWidgets(false);
			bool bWasWidgetDragging = Widget->IsDragging();
			Widget->SetDragging(false);

			// Invalidate the hit proxy map so it will be rendered out again when GetHitProxy
			// is called
			Viewport->InvalidateHitProxy();

			// This will actually re-render the viewport's hit proxies!
			HHitProxy* HitProxyWithoutAxisWidgets = Viewport->GetHitProxy(HitX, HitY);
			if (HitProxyWithoutAxisWidgets != NULL && !HitProxyWithoutAxisWidgets->IsA(HXWidgetAxis::StaticGetType()))
			{
				// Try this again, but without the widget this time!
				ProcessClick(View, HitProxyWithoutAxisWidgets, Key, Event, HitX, HitY);
			}

			// Undo the evil
			EngineShowFlags.SetModeWidgets(bOldModeWidgets1);
			SceneViewFamily->EngineShowFlags.SetModeWidgets(bOldModeWidgets2);

			Widget->SetDragging(bWasWidgetDragging);

			// Invalidate the hit proxy map again so that it'll be refreshed with the original
			// scene contents if we need it again later.
			Viewport->InvalidateHitProxy();
		}
		else if (HitProxy->IsA(HActor::StaticGetType()))
		{
			if (bRightMouseButtonPressed /*&& bDroppingPreviewTexture*/)
			{
				CancelDrop();
				return;
			}

			auto ActorHitProxy = (HActor*)HitProxy;
			const bool bSelectComponent = false;

			if (bSelectComponent)
			{
				//XClickHandlers::ClickComponent(this, ActorHitProxy, Click);
			}
			else
			{
				XClickHandlers::ClickActor(this, ActorHitProxy->Actor, Click, true);
				//更新跟随坐标轴的包围盒
				if (Click.GetKey() == EKeys::LeftMouseButton)
				{
					FBox SelectActorsBox(ForceInit);
					SelectActorsBox = GXEngine->GetSelectedActorsBox();
					FlowAxisBoxOfSelectedActors = SelectActorsBox;
				}				
			}
		}
		else if (HitProxy && HitProxy->IsA(HXObjectBaseProxy::StaticGetType()))
		{
		}
		else
		{
			// 没有选中要选的东西时，取消选中状态
			if (bRightMouseButtonPressed)
			{
				CancelDrop();
				return;
			}

		}
	}
}
bool UTGameViewportClient::CanMouseRotate() { return isAllowedRotateByMouse; }

void UTGameViewportClient::OpenMouseRotate() { isAllowedRotateByMouse = true; }

void UTGameViewportClient::CloseMouseRotate() { isAllowedRotateByMouse = false; }

FVector4 UTGameViewportClient::PixelToWorld(float X, float Y, float Z)
{
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		GetScene(),
		EngineShowFlags)
		.SetRealtimeUpdate(false));
	FSceneView* View = CalcSceneView(&ViewFamily);
	return View->PixelToWorld(X, Y, Z);
}
TSharedPtr<FXDragTool> UTGameViewportClient::MakeDragTool(EXDragTool::Type DragToolType)
{
	// Let the drag tool handle the transaction
	//TrackingTransaction.Cancel();

	TSharedPtr<FXDragTool> DragTool;
	switch (DragToolType)
	{
	case EXDragTool::BoxSelect:
		//DragTool = MakeShareable(new FXDragTool_ActorBoxSelect(this));
		break;
	case EXDragTool::FrustumSelect:
		DragTool = MakeShareable(new FXDragTool_ActorFrustumSelect(this));
		break;
	case EXDragTool::Measure:
		//DragTool = MakeShareable(new FVRSDragTool_Measure(this));
		break;
	case EXDragTool::ViewportChange:
		//DragTool = MakeShareable(new FVRSDragTool_ViewportChange(this));
		break;
	};

	return DragTool;
}
void UTGameViewportClient::ConvertMovementToDragRot(const FVector& InDelta, FVector& InDragDelta, FRotator& InRotDelta) const
{
	const FRotator& ViewRotation = GetViewRotation();

	const bool LeftMouseButtonDown = Viewport->KeyState(EKeys::LeftMouseButton);
	const bool MiddleMouseButtonDown = Viewport->KeyState(EKeys::MiddleMouseButton);
	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton);
	const bool bIsUsingTrackpad = FSlateApplication::Get().IsUsingTrackpad();

	InDragDelta = FVector::ZeroVector;
	InRotDelta = FRotator::ZeroRotator;

	switch (GetViewportType())
	{
	case EXLevelViewportType::LVT_OnlyCanvas:
	case EXLevelViewportType::LVT_OrthoXY:
	case EXLevelViewportType::LVT_OrthoXZ:
	case EXLevelViewportType::LVT_OrthoYZ:
	case EXLevelViewportType::LVT_OrthoNegativeXY:
	case EXLevelViewportType::LVT_OrthoNegativeXZ:
	case EXLevelViewportType::LVT_OrthoNegativeYZ:
	{
		if ((LeftMouseButtonDown || bIsUsingTrackpad) && RightMouseButtonDown)
		{
			// Both mouse buttons change the ortho viewport zoom.
			InDragDelta = FVector(0, 0, InDelta.Z);
		}
		else if (RightMouseButtonDown)
		{
			// @todo: set RMB to move opposite to the direction of drag, in other words "grab and pull".
			//InDragDelta = InDelta;
		}
		else if (LeftMouseButtonDown)
		{
			// LMB moves in the direction of the drag.
			InDragDelta = InDelta;
		}
		else if (MiddleMouseButtonDown)
		{
			// LMB moves in the direction of the drag.
			InDragDelta = InDelta;
		}
	}
	break;
	case EXLevelViewportType::LVT_Perspective:

	{
		//const ULevelEditorViewportSettings* ViewportSettings = GetDefault<ULevelEditorViewportSettings>();
		float MouseSensitivty = 0.6;
		if (LeftMouseButtonDown && !RightMouseButtonDown)
		{
			// Move forward and yaw

			InDragDelta.X = InDelta.Y * FMath::Cos(ViewRotation.Yaw * PI / 180.f);
			InDragDelta.Y = InDelta.Y * FMath::Sin(ViewRotation.Yaw * PI / 180.f);

			InRotDelta.Yaw = InDelta.X * MouseSensitivty;
		}
		else if (MiddleMouseButtonDown || bIsUsingTrackpad || ((LeftMouseButtonDown || bIsUsingTrackpad) && RightMouseButtonDown))
		{
			// Pan left/right/up/down
			const bool bInvert = !bIsUsingTrackpad && MiddleMouseButtonDown /*&& GetDefault<ULevelEditorViewportSettings>()->bInvertMiddleMousePan*/;


			float Direction = bInvert ? 1 : -1;
			InDragDelta.X = InDelta.X * Direction * FMath::Sin(ViewRotation.Yaw * PI / 180.f);
			InDragDelta.Y = InDelta.X * -Direction * FMath::Cos(ViewRotation.Yaw * PI / 180.f);
			InDragDelta.Z = -Direction * InDelta.Y;
		}
		else if (RightMouseButtonDown && !LeftMouseButtonDown)
		{
			// Change viewing angle

			// inverting orbit axis is handled elsewhere
			const bool bInvertY = !ShouldOrbitCamera() && false;/*&& GetDefault<ULevelEditorViewportSettings>()->bInvertMouseLookYAxis*/;
			float Direction = bInvertY ? -1 : 1;

			InRotDelta.Yaw = InDelta.X * /*ViewportSettings->*/MouseSensitivty;
			InRotDelta.Pitch = InDelta.Y * /*ViewportSettings->*/MouseSensitivty * Direction;
		}
	}

	break;
	case EXLevelViewportType::LVT_OrthoFreelook:
	{
		InDragDelta = InDelta;
	}
	break;
	default:
		check(0);	// unknown viewport type
		break;
	}
}

void UTGameViewportClient::ConvertMovementToOrbitDragRot(const FVector& InDelta,
	FVector& InDragDelta,
	FRotator& InRotDelta) const
{

}
bool UTGameViewportClient::ShouldOrbitCamera() const
{
	return bIsOrbit;
}
bool UTGameViewportClient::CanUseDragTool() const
{
	return /*!ShouldOrbitCamera() &&*/ (GetCurrentWidgetAxis() == EAxisList::None) && ((ModeTools == nullptr) || ModeTools->AllowsViewportDragTool());
}
void UTGameViewportClient::CancelDrop()
{
	////取消选中之前，把上次选中的物体设置为Static
	//TArray<AActor*> SelectedActors;
	//GXEngine->GetSelectedActors(SelectedActors);

	//for (auto& It : DroppedObjects)
	//{
	//	AActor* Actor = Cast<AActor>(It);
	//	if (Actor)
	//	{
	//		Actor->Destroy();
	//	}
	//}

	//TArray<UObject*> Objects;
	//SetDroppedObjects(Objects);
	//SetDroppingPreviewTexture(false);

	//GXEngine->SelectNone(true, true);
	//CurCursor = EMouseCursor::Default;
	//bAdsorbed = false;
}
bool UTGameViewportClient::IsAltPressed() const
{
	return Viewport->KeyState(EKeys::LeftAlt) || Viewport->KeyState(EKeys::RightAlt);
}

bool UTGameViewportClient::IsCtrlPressed() const
{
	return Viewport->KeyState(EKeys::LeftControl) || Viewport->KeyState(EKeys::RightControl);
}

bool UTGameViewportClient::IsShiftPressed() const
{
	return Viewport->KeyState(EKeys::LeftShift) || Viewport->KeyState(EKeys::RightShift);
}

bool UTGameViewportClient::IsCmdPressed() const
{
	return Viewport->KeyState(EKeys::LeftCommand) || Viewport->KeyState(EKeys::RightCommand);
}

#undef LOCTEXT_NAMESPACE