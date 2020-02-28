// Fill out your copyright notice in the Description page of Project Settings.


#include "XPlayerController.h"

AXPlayerController::AXPlayerController()
{

}
void AXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	this->SetActorTickEnabled(true);
	this->bBlockInput = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}