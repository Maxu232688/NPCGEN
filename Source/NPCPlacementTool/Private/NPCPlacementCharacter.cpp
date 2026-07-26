#include "NPCPlacementCharacter.h"
#include "NPCCoordinator.h"
#include "PlacedNPCActor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"

static ANPCCoordinator* FindCoordinator(UWorld* World)
{
	if (!World) return nullptr;
	TArray<AActor*> Coordinators;
	UGameplayStatics::GetAllActorsOfClass(World, ANPCCoordinator::StaticClass(), Coordinators);
	return (Coordinators.Num() > 0) ? Cast<ANPCCoordinator>(Coordinators[0]) : nullptr;
}

ANPCPlacementCharacter::ANPCPlacementCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(42.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 50.0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

	bUseControllerRotationYaw = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANPCPlacementCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CreateInputActions();
}

void ANPCPlacementCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Character::BeginPlay - Actor=%s, Controller=%s"),
		*GetName(), GetController() ? *GetController()->GetName() : TEXT("null"));

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ANPCPlacementCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	AddMappingContextToController();
}

void ANPCPlacementCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CameraBoom) CameraBoom->TargetArmLength = CameraDistance;

	if (FSlateApplication::IsInitialized())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());

		if (!bMouseCaptured || bTopDownMode)
		{
			bool bLeftDown = FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton);
			if (bLeftDown && !bIsBoxSelecting)
			{
				bIsBoxSelecting = true;
				if (PC) PC->GetMousePosition(BoxSelectStart.X, BoxSelectStart.Y);
			}
			else if (bLeftDown && bIsBoxSelecting)
			{
				if (PC && GetWorld())
				{
					float CurX, CurY;
					PC->GetMousePosition(CurX, CurY);

					auto DeprojectToGround = [&](float SX, float SY) -> FVector {
						FVector WorldLoc, WorldDir;
						if (PC->DeprojectScreenPositionToWorld(SX, SY, WorldLoc, WorldDir))
						{
							float T = (50.0f - WorldLoc.Z) / WorldDir.Z;
							if (T > 0.0f) return WorldLoc + WorldDir * T;
						}
						return FVector::ZeroVector;
					};

					FVector C0 = DeprojectToGround(BoxSelectStart.X, BoxSelectStart.Y);
					FVector C1 = DeprojectToGround(CurX, BoxSelectStart.Y);
					FVector C2 = DeprojectToGround(CurX, CurY);
					FVector C3 = DeprojectToGround(BoxSelectStart.X, CurY);

					FColor RectColor = FColor::Cyan;
					float Thickness = 2.0f;
					DrawDebugLine(GetWorld(), C0, C1, RectColor, false, -1.0f, 0, Thickness);
					DrawDebugLine(GetWorld(), C1, C2, RectColor, false, -1.0f, 0, Thickness);
					DrawDebugLine(GetWorld(), C2, C3, RectColor, false, -1.0f, 0, Thickness);
					DrawDebugLine(GetWorld(), C3, C0, RectColor, false, -1.0f, 0, Thickness);
				}
			}
			else if (!bLeftDown && bIsBoxSelecting)
			{
				bIsBoxSelecting = false;
				if (!PC) return;

				float EndX, EndY;
				PC->GetMousePosition(EndX, EndY);
				FVector2D BoxSelectEnd(EndX, EndY);

				ANPCCoordinator* Coordinator = FindCoordinator(GetWorld());
				if (!Coordinator) return;

				float DragDist = FVector2D::Distance(BoxSelectStart, BoxSelectEnd);
				UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Tick - Mouse released, drag=%.1f"), DragDist);

				if (DragDist < 5.0f)
				{
					FHitResult Hit;
					FVector2D ClickPos = BoxSelectStart;
					if (PC->GetHitResultAtScreenPosition(ClickPos, ECC_Visibility, false, Hit))
					{
						if (APlacedNPCActor* NPC = Cast<APlacedNPCActor>(Hit.GetActor()))
							Coordinator->ToggleNPCSelection(NPC);
					}
				}
				else
				{
					Coordinator->SelectNPCsInScreenRect(BoxSelectStart, BoxSelectEnd, PC);
				}
			}
		}
		else
		{
			bIsBoxSelecting = false;
		}
	}
}

void ANPCPlacementCharacter::CreateInputActions()
{
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("MoveForward"));
	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;

	MoveRightAction = NewObject<UInputAction>(this, TEXT("MoveRight"));
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;

	LookAction = NewObject<UInputAction>(this, TEXT("Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	SprintAction = NewObject<UInputAction>(this, TEXT("Sprint"));
	SprintAction->ValueType = EInputActionValueType::Boolean;

	AltAction = NewObject<UInputAction>(this, TEXT("Alt"));
	AltAction->ValueType = EInputActionValueType::Boolean;

	ScrollAction = NewObject<UInputAction>(this, TEXT("Scroll"));
	ScrollAction->ValueType = EInputActionValueType::Axis1D;

	UndoAction = NewObject<UInputAction>(this, TEXT("Undo"));
	UndoAction->ValueType = EInputActionValueType::Boolean;

	RotateLeftAction = NewObject<UInputAction>(this, TEXT("RotateLeft"));
	RotateLeftAction->ValueType = EInputActionValueType::Boolean;

	RotateRightAction = NewObject<UInputAction>(this, TEXT("RotateRight"));
	RotateRightAction->ValueType = EInputActionValueType::Boolean;

	TopDownAction = NewObject<UInputAction>(this, TEXT("TopDown"));
	TopDownAction->ValueType = EInputActionValueType::Boolean;

	DeleteAction = NewObject<UInputAction>(this, TEXT("Delete"));
	DeleteAction->ValueType = EInputActionValueType::Boolean;

	FreeCamAction = NewObject<UInputAction>(this, TEXT("FreeCam"));
	FreeCamAction->ValueType = EInputActionValueType::Boolean;

	DefaultMappingContext = NewObject<UInputMappingContext>(this, TEXT("DefaultIMC"));

	DefaultMappingContext->MapKey(MoveForwardAction, EKeys::W);
	FEnhancedActionKeyMapping& SMapping = DefaultMappingContext->MapKey(MoveForwardAction, EKeys::S);
	SMapping.Modifiers.Add(NewObject<UInputModifierNegate>(this, TEXT("NegateModifier_S")));

	DefaultMappingContext->MapKey(MoveRightAction, EKeys::D);
	FEnhancedActionKeyMapping& AMapping = DefaultMappingContext->MapKey(MoveRightAction, EKeys::A);
	AMapping.Modifiers.Add(NewObject<UInputModifierNegate>(this, TEXT("NegateModifier_A")));

	DefaultMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	DefaultMappingContext->MapKey(SprintAction, EKeys::LeftShift);
	DefaultMappingContext->MapKey(AltAction, EKeys::LeftAlt);
	DefaultMappingContext->MapKey(ScrollAction, EKeys::MouseWheelAxis);
	DefaultMappingContext->MapKey(UndoAction, EKeys::Z);
	DefaultMappingContext->MapKey(RotateLeftAction, EKeys::Q);
	DefaultMappingContext->MapKey(RotateRightAction, EKeys::E);
	DefaultMappingContext->MapKey(TopDownAction, EKeys::X);
	DefaultMappingContext->MapKey(FreeCamAction, EKeys::F);
	DefaultMappingContext->MapKey(DeleteAction, EKeys::Delete);

	static const FKey NumberKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine };
	for (int32 i = 0; i < 9; i++)
	{
		UInputAction* HotkeyAction = NewObject<UInputAction>(this, *FString::Printf(TEXT("Hotkey%d"), i + 1));
		HotkeyAction->ValueType = EInputActionValueType::Boolean;
		HotkeyActions.Add(HotkeyAction);
		DefaultMappingContext->MapKey(HotkeyAction, NumberKeys[i]);
	}
}

void ANPCPlacementCharacter::AddMappingContextToController()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) { UE_LOG(LogTemp, Warning, TEXT("[NPCPlacementTool] Character::AddMappingContext - No PlayerController!")); return; }

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) { UE_LOG(LogTemp, Warning, TEXT("[NPCPlacementTool] Character::AddMappingContext - No LocalPlayer!")); return; }

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Character::AddMappingContext - IMC added (HotkeyActions=%d)"), HotkeyActions.Num());
	}
}

void ANPCPlacementCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) { UE_LOG(LogTemp, Error, TEXT("[NPCPlacementTool] Character::SetupPlayerInputComponent - NOT EnhancedInputComponent!")); return; }

	EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ANPCPlacementCharacter::MoveForward);
	EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ANPCPlacementCharacter::MoveRight);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANPCPlacementCharacter::Look);
	EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::StartSprint);
	EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANPCPlacementCharacter::StopSprint);
	EIC->BindAction(AltAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnAltPressed);
	EIC->BindAction(ScrollAction, ETriggerEvent::Triggered, this, &ANPCPlacementCharacter::OnScroll);
	EIC->BindAction(UndoAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnUndo);
	EIC->BindAction(RotateLeftAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnRotateLeft);
	EIC->BindAction(RotateRightAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnRotateRight);
	EIC->BindAction(TopDownAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnTopDownToggle);
	EIC->BindAction(DeleteAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnDeletePressed);
	EIC->BindAction(FreeCamAction, ETriggerEvent::Started, this, &ANPCPlacementCharacter::OnFreeCamToggle);

	typedef void (ANPCPlacementCharacter::*FHotkeyFunc)(const FInputActionValue&);
	static const FHotkeyFunc HotkeyFuncs[] = {
		&ANPCPlacementCharacter::OnHotkey0, &ANPCPlacementCharacter::OnHotkey1,
		&ANPCPlacementCharacter::OnHotkey2, &ANPCPlacementCharacter::OnHotkey3,
		&ANPCPlacementCharacter::OnHotkey4, &ANPCPlacementCharacter::OnHotkey5,
		&ANPCPlacementCharacter::OnHotkey6, &ANPCPlacementCharacter::OnHotkey7,
		&ANPCPlacementCharacter::OnHotkey8
	};
	for (int32 i = 0; i < HotkeyActions.Num() && i < 9; i++)
		EIC->BindAction(HotkeyActions[i], ETriggerEvent::Started, this, HotkeyFuncs[i]);

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Character::SetupPlayerInputComponent - All bindings complete"));
}

void ANPCPlacementCharacter::MoveForward(const FInputActionValue& Value)
{
	if (!bMouseCaptured && !bTopDownMode) return;
	float AxisValue = Value.Get<float>();
	if (Controller && AxisValue != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		if (bFreeCamera)
		{
			FVector Offset = ForwardDirection * AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds() * 2.0f;
			AddActorWorldOffset(Offset, true);
		}
		else
		{
			AddMovementInput(ForwardDirection, AxisValue);
		}
	}
}

void ANPCPlacementCharacter::MoveRight(const FInputActionValue& Value)
{
	if (!bMouseCaptured && !bTopDownMode) return;
	float AxisValue = Value.Get<float>();
	if (Controller && AxisValue != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		if (bFreeCamera)
		{
			FVector Offset = RightDirection * AxisValue * MovementSpeed * GetWorld()->GetDeltaSeconds() * 2.0f;
			AddActorWorldOffset(Offset, true);
		}
		else
		{
			AddMovementInput(RightDirection, AxisValue);
		}
	}
}

void ANPCPlacementCharacter::Look(const FInputActionValue& Value)
{
	if (!bMouseCaptured) return;
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller && LookAxisVector.SizeSquared() > 0.0f)
	{
		AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * MouseSensitivity);
	}
}

void ANPCPlacementCharacter::StartSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed * SprintMultiplier;
}

void ANPCPlacementCharacter::StopSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void ANPCPlacementCharacter::OnAltPressed(const FInputActionValue& Value)
{
	if (bTopDownMode) { ExitTopDownMode(); return; }
	SetMouseCapture(!bMouseCaptured);
}

void ANPCPlacementCharacter::OnScroll(const FInputActionValue& Value)
{
	float ScrollValue = Value.Get<float>();
	CameraDistance = FMath::Clamp(CameraDistance - ScrollValue * 50.0f,
		bTopDownMode ? 200.0f : 100.0f, bTopDownMode ? 8000.0f : 1000.0f);
}

void ANPCPlacementCharacter::EnterTopDownMode()
{
	if (bTopDownMode) return;
	if (bFreeCamera) { bFreeCamera = false; GetCharacterMovement()->SetMovementMode(MOVE_Walking); GetCharacterMovement()->GravityScale = 1.0f; }

	bTopDownMode = true;
	NormalCameraDistance = CameraDistance;

	if (Controller) { SavedControlRotation = Controller->GetControlRotation(); FRotator TopDownRot = SavedControlRotation; TopDownRot.Pitch = -85.0f; Controller->SetControlRotation(TopDownRot); }

	CameraDistance = 2500.0f;
	SetMouseCapture(false);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Entered top-down mode"));
}

void ANPCPlacementCharacter::ExitTopDownMode()
{
	if (!bTopDownMode) return;
	bTopDownMode = false;
	if (Controller) Controller->SetControlRotation(SavedControlRotation);
	CameraDistance = NormalCameraDistance;
	SetMouseCapture(true);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	if (ANPCCoordinator* Coordinator = FindCoordinator(GetWorld())) Coordinator->ClearSelection();
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Exited top-down mode"));
}

void ANPCPlacementCharacter::OnTopDownToggle(const FInputActionValue& Value)
{
	if (bTopDownMode) ExitTopDownMode(); else EnterTopDownMode();
}

void ANPCPlacementCharacter::OnUndo(const FInputActionValue& Value)
{
	if (ANPCCoordinator* Coordinator = FindCoordinator(GetWorld()))
		Coordinator->UndoLastPlacement();
}

void ANPCPlacementCharacter::OnRotateLeft(const FInputActionValue& Value)
{
	if (ANPCCoordinator* Coordinator = FindCoordinator(GetWorld()))
		Coordinator->RotateSelectedNPCs(-15.0f);
}

void ANPCPlacementCharacter::OnRotateRight(const FInputActionValue& Value)
{
	if (ANPCCoordinator* Coordinator = FindCoordinator(GetWorld()))
		Coordinator->RotateSelectedNPCs(15.0f);
}

void ANPCPlacementCharacter::OnFreeCamToggle(const FInputActionValue& Value)
{
	if (!bMouseCaptured || bTopDownMode) return;

	bFreeCamera = !bFreeCamera;

	if (bFreeCamera)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->GravityScale = 0.0f;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Entered free camera mode"));
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->GravityScale = 1.0f;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Exited free camera mode (third-person)"));
	}
}

void ANPCPlacementCharacter::OnDeletePressed(const FInputActionValue& Value)
{
	if (!bTopDownMode) return;
	if (ANPCCoordinator* Coordinator = FindCoordinator(GetWorld()))
		Coordinator->DeleteSelectedNPCs();
}

void ANPCPlacementCharacter::OnHotkeyPressed(int32 Index)
{
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Character::OnHotkeyPressed - Key=%d"), Index + 1);

	ANPCCoordinator* Coordinator = FindCoordinator(GetWorld());
	if (!Coordinator) { UE_LOG(LogTemp, Warning, TEXT("[NPCPlacementTool] Character::OnHotkeyPressed - No Coordinator!")); return; }

	int32 CurrentSelected = Coordinator->GetSelectedNPCIndex();

	if (CurrentSelected == Index)
		Coordinator->SpawnNPCAtPlayer(Index);
	else
		Coordinator->SelectNPCType(Index);
}

void ANPCPlacementCharacter::SetMouseCapture(bool bCapture)
{
	bMouseCaptured = bCapture;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		if (bCapture) { PC->SetShowMouseCursor(false); PC->SetInputMode(FInputModeGameOnly()); }
		else { PC->SetShowMouseCursor(true); PC->SetInputMode(FInputModeGameAndUI()); }
	}
}

void ANPCPlacementCharacter::SetConfig(float InCameraDistance, float InCameraPitch, float InMoveSpeed, float InSprint, float InMouseSens)
{
	CameraDistance = InCameraDistance;
	CameraPitch = InCameraPitch;
	MovementSpeed = InMoveSpeed;
	SprintMultiplier = InSprint;
	MouseSensitivity = InMouseSens;
}
