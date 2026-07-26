#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "NPCPlacementCharacter.generated.h"

UCLASS()
class NPCPLACEMENTTOOL_API ANPCPlacementCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCPlacementCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyControllerChanged() override;
	virtual void PostInitializeComponents() override;

	void OnHotkeyPressed(int32 Index);

	UFUNCTION(BlueprintCallable)
	void SetMouseCapture(bool bCapture);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsMouseCaptured() const { return bMouseCaptured; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsTopDownMode() const { return bTopDownMode; }

	void SetConfig(float InCameraDistance, float InCameraPitch, float InMoveSpeed, float InSprint, float InMouseSens);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveForwardAction;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveRightAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY()
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY()
	TObjectPtr<UInputAction> AltAction;

	UPROPERTY()
	TObjectPtr<UInputAction> ScrollAction;

	UPROPERTY()
	TObjectPtr<UInputAction> UndoAction;

	UPROPERTY()
	TObjectPtr<UInputAction> RotateLeftAction;

	UPROPERTY()
	TObjectPtr<UInputAction> RotateRightAction;

	UPROPERTY()
	TObjectPtr<UInputAction> TopDownAction;

	UPROPERTY()
	TObjectPtr<UInputAction> FreeCamAction;

	UPROPERTY()
	TObjectPtr<UInputAction> DeleteAction;

	UPROPERTY()
	TArray<TObjectPtr<UInputAction>> HotkeyActions;

	bool bMouseCaptured = true;
	bool bTopDownMode = false;

	bool bFreeCamera = false;

	bool bIsBoxSelecting = false;
	FVector2D BoxSelectStart;

	float NormalCameraDistance = 300.0f;
	FRotator SavedControlRotation = FRotator(-30.0f, 0.0f, 0.0f);

	float CameraDistance = 300.0f;
	float CameraPitch = -30.0f;
	float MovementSpeed = 600.0f;
	float SprintMultiplier = 2.0f;
	float MouseSensitivity = 1.0f;

	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	void OnAltPressed(const FInputActionValue& Value);
	void OnScroll(const FInputActionValue& Value);
	void OnUndo(const FInputActionValue& Value);
	void OnRotateLeft(const FInputActionValue& Value);
	void OnRotateRight(const FInputActionValue& Value);
	void OnTopDownToggle(const FInputActionValue& Value);
	void OnFreeCamToggle(const FInputActionValue& Value);
	void OnDeletePressed(const FInputActionValue& Value);
	void OnHotkey0(const FInputActionValue& Value) { OnHotkeyPressed(0); }
	void OnHotkey1(const FInputActionValue& Value) { OnHotkeyPressed(1); }
	void OnHotkey2(const FInputActionValue& Value) { OnHotkeyPressed(2); }
	void OnHotkey3(const FInputActionValue& Value) { OnHotkeyPressed(3); }
	void OnHotkey4(const FInputActionValue& Value) { OnHotkeyPressed(4); }
	void OnHotkey5(const FInputActionValue& Value) { OnHotkeyPressed(5); }
	void OnHotkey6(const FInputActionValue& Value) { OnHotkeyPressed(6); }
	void OnHotkey7(const FInputActionValue& Value) { OnHotkeyPressed(7); }
	void OnHotkey8(const FInputActionValue& Value) { OnHotkeyPressed(8); }

	void EnterTopDownMode();
	void ExitTopDownMode();

	void CreateInputActions();
	void AddMappingContextToController();
};
