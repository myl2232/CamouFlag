// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "SceneManagement.h"
#include "Core/XCoreType.h"
#include "Design/XUnrealWidget.h"
#include "TGameViewportClient.generated.h"

DECLARE_DELEGATE_RetVal(bool, FViewportStateGetter);
class FXEditorModeTools;
class FXWidget;
class FXMouseDeltaTracker;
class FXDragTool;

struct HXObjectBaseProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(CAMOUFLAGENET_API);

	HXObjectBaseProxy(EHitProxyPriority InPriority = HPP_Wireframe)
		: HHitProxy(InPriority)
	{
	}

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::Crosshairs;
	}
};

struct FXInputEventState
{
public:
	FXInputEventState(FViewport* InViewport, FKey InKey, EInputEvent InInputEvent)
		: Viewport(InViewport)
		, Key(InKey)
		, InputEvent(InInputEvent)
	{}

	FViewport* GetViewport() const { return Viewport; }
	EInputEvent GetInputEvent() const { return InputEvent; }
	FKey GetKey() const { return Key; }

	/** return true if the event causing button is a control key */
	bool IsCtrlButtonEvent() const { return (Key == EKeys::LeftControl || Key == EKeys::RightControl); }
	bool IsShiftButtonEvent() const { return (Key == EKeys::LeftShift || Key == EKeys::RightShift); }
	bool IsAltButtonEvent() const { return (Key == EKeys::LeftAlt || Key == EKeys::RightAlt); }

	bool IsLeftMouseButtonPressed() const { return IsButtonPressed(EKeys::LeftMouseButton); }
	bool IsMiddleMouseButtonPressed() const { return IsButtonPressed(EKeys::MiddleMouseButton); }
	bool IsRightMouseButtonPressed() const { return IsButtonPressed(EKeys::RightMouseButton); }

	bool IsMouseButtonEvent() const { return (Key == EKeys::LeftMouseButton || Key == EKeys::MiddleMouseButton || Key == EKeys::RightMouseButton); }
	bool IsButtonPressed(FKey InKey) const { return Viewport->KeyState(InKey); }
	bool IsAnyMouseButtonDown() const { return IsButtonPressed(EKeys::LeftMouseButton) || IsButtonPressed(EKeys::MiddleMouseButton) || IsButtonPressed(EKeys::RightMouseButton); }

	/** return true if alt is pressed right now.  This will be true even if the event was for a different key but an alt key is currently pressed */
	bool IsAltButtonPressed() const { return !(IsAltButtonEvent() && InputEvent == IE_Released) && (IsButtonPressed(EKeys::LeftAlt) || IsButtonPressed(EKeys::RightAlt)); }
	bool IsShiftButtonPressed() const { return !(IsShiftButtonEvent() && InputEvent == IE_Released) && (IsButtonPressed(EKeys::LeftShift) || IsButtonPressed(EKeys::RightShift)); }
	bool IsCtrlButtonPressed() const { return !(IsCtrlButtonEvent() && InputEvent == IE_Released) && (IsButtonPressed(EKeys::LeftControl) || IsButtonPressed(EKeys::RightControl)); }
	bool IsSpaceBarPressed() const { return IsButtonPressed(EKeys::SpaceBar); }

private:
	/** Viewport the event was sent to */
	FViewport* Viewport;
	/** Pressed Key */
	FKey Key;
	/** Key event */
	EInputEvent InputEvent;
};
/**
* Contains information about a mouse cursor position within a viewport, transformed into the correct coordinate
* system for the viewport.
*/
struct FXViewportCursorLocation
{
public:
	CAMOUFLAGENET_API FXViewportCursorLocation(const class FSceneView* View, UTGameViewportClient* InViewportClient, int32 X, int32 Y);
	CAMOUFLAGENET_API virtual ~FXViewportCursorLocation();

	const FVector&		GetOrigin()			const { return Origin; }
	const FVector&		GetDirection()		const { return Direction; }
	const FIntPoint&	GetCursorPos()		const { return CursorPos; }
	EXLevelViewportType::Type	GetViewportType()	const;
	UTGameViewportClient*	GetViewportClient()	const { return ViewportClient; }

private:
	FVector	Origin;
	FVector	Direction;
	FIntPoint CursorPos;
	UTGameViewportClient* ViewportClient;
};

struct FXViewportClick : public FXViewportCursorLocation
{
public:
	CAMOUFLAGENET_API FXViewportClick(const class FSceneView* View, class UTGameViewportClient* ViewportClient, FKey InKey, EInputEvent InEvent, int32 X, int32 Y);
	CAMOUFLAGENET_API ~FXViewportClick();

	/** @return The 2D screenspace cursor position of the mouse when it was clicked. */
	const FIntPoint&	GetClickPos()	const { return GetCursorPos(); }
	const FKey&			GetKey()		const { return Key; }
	EInputEvent			GetEvent()		const { return Event; }

	virtual bool	IsControlDown()	const { return ControlDown; }
	virtual bool	IsShiftDown()	const { return ShiftDown; }
	virtual bool	IsAltDown()		const { return AltDown; }

private:
	FKey		Key;
	EInputEvent	Event;
	bool		ControlDown,
		ShiftDown,
		AltDown;
};

/**
* Stores the transformation data for the viewport camera
*/
struct CAMOUFLAGENET_API FXViewportCameraTransform
{
public:
	FXViewportCameraTransform();

	/** Sets the transform's location */
	void SetLocation(const FVector& Position);

	/** Sets the transform's rotation */
	void SetRotation(const FRotator& Rotation)
	{
		ViewRotation = Rotation;
	}

	/** Sets the location to look at during orbit */
	void SetLookAt(const FVector& InLookAt)
	{
		LookAt = InLookAt;
	}

	/** Set the ortho zoom amount */
	void SetOrthoZoom(float InOrthoZoom)
	{
		OrthoZoom = InOrthoZoom;
	}

	/** Check if transition curve is playing. */
	bool IsPlaying();

	/** @return The transform's location */
	FORCEINLINE const FVector& GetLocation() const { return ViewLocation; }

	/** @return The transform's rotation */
	FORCEINLINE const FRotator& GetRotation() const { return ViewRotation; }

	/** @return The look at point for orbiting */
	FORCEINLINE const FVector& GetLookAt() const { return LookAt; }

	/** @return The ortho zoom amount */
	FORCEINLINE float GetOrthoZoom() const { return OrthoZoom; }

	/**
	* Animates from the current location to the desired location
	*
	* @param InDesiredLocation	The location to transition to
	* @param bInstant			If the desired location should be set instantly rather than transitioned to over time
	*/
	void TransitionToLocation(const FVector& InDesiredLocation, TWeakPtr<SWidget> EditorViewportWidget, bool bInstant);

	/**
	* Updates any current location transitions
	*
	* @return true if there is currently a transition
	*/
	bool UpdateTransition();

	/**
	* Computes a matrix to use for viewport location and rotation when orbiting
	*/
	FMatrix ComputeOrbitMatrix() const;

private:
	/** The time when a transition to the desired location began */
	//double TransitionStartTime;

	/** Curve for animating between locations */
	TSharedPtr<struct FCurveSequence> TransitionCurve;
	/** Current viewport Position. */
	FVector	ViewLocation;
	/** Current Viewport orientation; valid only for perspective projections. */
	FRotator ViewRotation;
	/** Desired viewport location when animating between two locations */
	FVector	DesiredLocation;
	/** When orbiting, the point we are looking at */
	FVector LookAt;
	/** Viewport start location when animating to another location */
	FVector StartLocation;
	/** Ortho zoom amount */
	float OrthoZoom;
};
/**
 * 
 */
UCLASS()
class CAMOUFLAGENET_API UTGameViewportClient : public UGameViewportClient, public FViewElementDrawer
{
	GENERATED_UCLASS_BODY()

public:
	~UTGameViewportClient();
	virtual void Tick(float DeltaTime) override;

	/** FViewElementDrawer interface */
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void Draw(FViewport* InViewport, FCanvas* SceneCanvas) override;
	void RenderDragTool(const FSceneView* View, FCanvas* Canvas);
	FXEditorModeTools* GetModeTools() const { return ModeTools; }

	/** FViewportClient interface */
	virtual bool RequiresHitProxyStorage() override { return 1; }
	virtual void PerspectiveCameraMoved() {}
	void ProcessClickInViewport(const FXInputEventState& InputState, FSceneView& View);
	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY);
	void StartTrackingDueToInput(const struct FXInputEventState& InputState);
	virtual void SetCurrentWidgetAxis(EAxisList::Type InAxis);
	EAxisList::Type GetCurrentWidgetAxis() const;
	virtual void TrackingStarted(const struct FXInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge);
	virtual void TrackingStopped();
	virtual void RedrawRequested(FViewport* Viewport) override;
	virtual void RequestInvalidateHitProxy(FViewport* Viewport) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual void MouseEnter(FViewport* Viewport, int32 x, int32 y) override;
	virtual void MouseMove(FViewport* Viewport, int32 x, int32 y) override;
	virtual void MouseLeave(FViewport* Viewport) override;
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass = eSSP_FULL);
	void ApplyDeltaToActors(const FVector& InDrag, const FRotator& InRot, const FVector& InScale);
	virtual void ApplyDeltaToActor(AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale);
	virtual void CheckHoveredHitProxy(HHitProxy* HoveredHitProxy);
	FBox GetActorCollisionBox(AActor* Actor) const;
	virtual TSharedPtr<FXDragTool> MakeDragTool(EXDragTool::Type DragToolType);
	void OnDollyPerspectiveCamera(const struct FXInputEventState& InputState);
	void MoveViewportCamera(const FVector& InDrag, const FRotator& InRot, bool bDollyCamera = false);
	virtual void UpdateLinkedOrthoViewports(bool bInvalidate = false) {}
	/*other*/
	bool IsVisible() const;
	bool IsPerspective() const;
	EXLevelViewportType::Type GetViewportType() const { return ViewportType; };
	FSceneInterface* GetScene() const;
	float GetNearClipPlane() const;
	virtual FVector GetWidgetLocation() const;
	FXWidget::EWidgetMode GetWidgetMode() const;
	virtual FMatrix GetWidgetCoordSystem() const;
	EXCoordSystem GetWidgetCoordSystemSpace() const;
	void Invalidate(bool bInvalidateChildViews = true, bool bInvalidateHitProxies = true);
	//
	bool IsAltPressed() const;
	bool IsCtrlPressed() const;
	bool IsShiftPressed() const;
	bool IsCmdPressed() const;
	bool IsTracking() const { return bIsTracking; }
	FVector TranslateDelta(FKey InKey, float InDelta, bool InNudge);
	//
	virtual bool CanUseDragTool() const;
	virtual bool ShouldOrbitCamera() const;
	void CancelDrop();
	void ModifyScale(AActor* InActor, FVector& ScaleDelta, bool bCheckSmallExtent = false) const;
	void ValidateScale(const FVector& CurrentScale, const FVector& BoxExtent, FVector& ScaleDelta, bool bCheckSmallExtent = false) const;
	//Orbit
	void ConvertMovementToDragRot(const FVector& InDelta, FVector& InDragDelta, FRotator& InRotDelta) const;
	void ConvertMovementToOrbitDragRot(const FVector& InDelta, FVector& InDragDelta, FRotator& InRotDelta) const;
	//void ToggleOrbitCamera(bool bEnableOrbitCamera);
	void SetViewLocationForOrbiting(const FVector& LookAtPoint, float DistanceToCamera = 256.f);
	//void MoveViewportCamera(const FVector& InDrag, const FRotator& InRot, bool bDollyCamera = false);

	//实时渲染设置相关
	bool ToggleRealtime();

	void SetRealtime(bool bInRealtime, bool bStoreCurrentValue = false);

	bool IsRealtime() const { return bIsRealtime || RealTimeFrameCount != 0; }

	void RequestRealTimeFrames(uint32 NumRealTimeFrames = 1)
	{
		RealTimeFrameCount = FMath::Max(NumRealTimeFrames, RealTimeFrameCount);
	}

	void RestoreRealtime(const bool bAllowDisable = false);
	/*------------viewport---------------*/
	FXViewportCameraTransform& GetViewTransform()
	{
		return IsPerspective() ? ViewTransformPerspective : ViewTransformOrthographic;
	}

	const FXViewportCameraTransform& GetViewTransform() const
	{
		return IsPerspective() ? ViewTransformPerspective : ViewTransformOrthographic;
	}

	void SetViewLocation(const FVector& NewLocation)
	{
		FXViewportCameraTransform& ViewTransform = GetViewTransform();
		ViewTransform.SetLocation(NewLocation);
	}

	void SetViewRotation(const FRotator& NewRotation)
	{
		FXViewportCameraTransform& ViewTransform = GetViewTransform();
		ViewTransform.SetRotation(NewRotation);
	}

	void SetLookAtLocation(const FVector& LookAt, bool bRecalculateView = false)
	{
		FXViewportCameraTransform& ViewTransform = GetViewTransform();

		ViewTransform.SetLookAt(LookAt);

		if (bRecalculateView)
		{
			FMatrix OrbitMatrix = ViewTransform.ComputeOrbitMatrix();
			OrbitMatrix = OrbitMatrix.InverseFast();

			ViewTransform.SetRotation(OrbitMatrix.Rotator());
			ViewTransform.SetLocation(OrbitMatrix.GetOrigin());
		}
	}

	/** Sets ortho zoom amount */
	void SetOrthoZoom(float InOrthoZoom)
	{
		FXViewportCameraTransform& ViewTransform = GetViewTransform();

		// A zero ortho zoom is not supported and causes NaN/div0 errors
		check(InOrthoZoom != 0);
		ViewTransform.SetOrthoZoom(InOrthoZoom);
	}
	/** @return the current viewport camera location */
	const FVector& GetViewLocation() const
	{
		const FXViewportCameraTransform& ViewTransform = GetViewTransform();
		return ViewTransform.GetLocation();
	}

	/** @return the current viewport camera rotation */
	const FRotator& GetViewRotation() const
	{
		const FXViewportCameraTransform& ViewTransform = GetViewTransform();
		return ViewTransform.GetRotation();
	}

	/** @return the current look at location */
	const FVector& GetLookAtLocation() const
	{
		const FXViewportCameraTransform& ViewTransform = GetViewTransform();
		return ViewTransform.GetLookAt();
	}

	/** @return the current ortho zoom amount */
	float GetOrthoZoom() const
	{
		const FXViewportCameraTransform& ViewTransform = GetViewTransform();
		return ViewTransform.GetOrthoZoom();
	}
	FLinearColor GetBackgroundColor() const
	{
		return FLinearColor(FColor(0XFF191A1D));
	}
	float GetOrthoUnitsPerPixel(const FViewport* InViewport) const;
	FScale2D GetMouseScale()
	{
		return FScale2D(MouseScale.X, MouseScale.Y);
	}
protected:
	void StopTracking();
	FVector4 PixelToWorld(float X, float Y, float Z);

	bool CanMouseRotate();
	void OpenMouseRotate();
	void CloseMouseRotate();
	virtual bool ShouldLockPitch() const;
	void MoveViewportPerspectiveCamera(const FVector& InDrag, const FRotator& InRot, bool bDollyCamera = false);
	void ConditionalCheckHoveredHitProxy();
public:
	FViewport* Viewport;
	FViewportStateGetter VisibilityDelegate;
	FXEditorModeTools*	ModeTools;
	FXWidget* Widget;
	FXMouseDeltaTracker* MouseDeltaTracker;

	/** 是否启用PlayerController的视角计算，否则会使用XCameraController计算视角 */
	bool bUserPlayerControllerView;

	bool bWidgetAxisControlledByDrag;
	bool bNeedsRedraw;
	bool bNeedsInvalidateHitProxy;
	bool bUsingOrbitCamera;
	bool bEnableClick;
	bool bUseControllingActorViewInfo;
	bool bShouldCheckHitProxy;

	FBox FlowAxisBoxOfSelectedActors;//一直跟随坐标系的包围盒，尺寸和当前选中物体集的包围盒相同
	//bool bAdsorbed;//是否已经吸附
	//bool bSnapTranslate;//是否开启吸附
	//bool bSnapTranslateTemp;//Alt键临时开启吸附
	//float AdsorbDistance;//吸附距离

	/**
	* true when within a FMouseDeltaTracker::StartTracking/EndTracking block.
	*/
	bool bIsTracking;
	/** true when we are in a state where we can check the hit proxy for hovering. */
	/** true when a brush is being transformed by its Widget */
	bool bIsTrackingBrushModification;
	/**
	* Used for drag duplication. Set to true on Alt+LMB so that the selected
	* objects (components or actors) will be duplicated as soon as the widget is displaced.
	*/
	bool bDuplicateOnNextDrag;
	/**true if only the pivot position has been moved*/
	bool bOnlyMovedPivot;
	/**
	* true if the user is dragging by a widget handle.
	*/
	bool bDraggingByHandle;
	/**
	* bDuplicateActorsOnNextDrag will not be set again while bDuplicateActorsInProgress is true.
	* The user needs to release ALT and all mouse buttons to clear bDuplicateActorsInProgress.
	*/
	bool bDuplicateActorsInProgress;

	FXViewportCameraTransform		ViewTransformPerspective;
	FXViewportCameraTransform		ViewTransformOrthographic;
	EXLevelViewportType::Type		ViewportType;
	/** The viewport's scene view state. */
	FSceneViewStateReference ViewState;
	/** Viewport's current horizontal field of view (can be modified by locked cameras etc.) */
	float ViewFOV;
	/** Viewport's stored horizontal field of view (saved in ini files). */
	float FOVAngle;
	float AspectRatio;
	bool bForceHiddenWidget;
	bool bUsesDrawHelper;
	bool bIsOrbit;
	bool bIsRealtime;
	bool bStoredRealtime;
	//视口缩放系数 避免鼠标拾取错误
	FVector2D MouseScale;
private:
	/** near plane adjustable for each editor view, if < 0 GNearClippingPlane should be used. */
	float NearPlane;
	/** If > 0, overrides the view's far clipping plane with a plane at the specified distance. */
	float FarPlane;
	// -1, -1 if not set
	FIntPoint CurrentMousePos;
	/** Represents the last known mouse position. If the mouse stops moving it's not the current but the last position before the current location. */
	uint32 CachedMouseX;
	uint32 CachedMouseY;
	/* Updated on each mouse drag start */
	uint32 LastMouseX;
	uint32 LastMouseY;
	//拖拽前后的鼠标世界坐标
	FVector WorldStartPos;
	FVector WorldEndPos;
	/** If true, the canvas has been been moved using bMoveCanvas Mode*/
	bool bHasMouseMovedSinceClick;
	//是否开启右键旋转功能
	bool isAllowedRotateByMouse = true;
	bool bShouldInvalidateViewportWidget;
	//class FXEditorCameraController* CameraController;
	uint32 RealTimeFrameCount;
};
