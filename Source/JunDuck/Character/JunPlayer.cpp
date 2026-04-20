#include "Character/JunPlayer.h"
#include "Camera/JunSpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/WeaponActor.h"
#include "JunGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/UnrealType.h"

AJunPlayer::AJunPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetupMeshAndCollision();
	SetupCameraComponents();
	SetupMovementDefaults();
}

void AJunPlayer::SetupMeshAndCollision()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("PlayerCapsule"));
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));
}

void AJunPlayer::SetupCameraComponents()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAnchor"));
	CameraAnchor->SetupAttachment(GetCapsuleComponent());
	CameraAnchor->SetUsingAbsoluteLocation(true);
	CameraAnchor->SetRelativeLocation(FVector::ZeroVector);

	SpringArm = CreateDefaultSubobject<UJunSpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CameraAnchor);
	SpringArm->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 60.f), FRotator(-20.0f, 0, 0));
	SpringArm->TargetArmLength = 300.f;
	SpringArm->bUsePawnControlRotation = true;

	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;

	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraLagSpeed = 15.f;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void AJunPlayer::SetupMovementDefaults()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->bOrientRotationToMovement = true;
	MovementComponent->RotationRate = FRotator(0.f, 700.f, 0.f);
	MovementComponent->JumpZVelocity = 900.f;
	MovementComponent->GravityScale = 2.4f;
	MovementComponent->AirControl = 0.25f;
	TargetMaxWalkSpeed = FreeRunMoveSpeed;
	MovementComponent->MaxWalkSpeed = FreeRunMoveSpeed;
}

void AJunPlayer::BeginPlay()
{
	Super::BeginPlay();

	SetTeamTag(JunGameplayTags::Team_Player);
	SpawnAndAttachWeapon();
	SpawnAndAttachScabbard();
	GetPlayerAnimInstance();
	CacheDefaultMovementBrakingSettings();
	TryInitializeCameraRotationFromController();

	if (CameraAnchor)
	{
		CameraAnchor->SetWorldLocation(GetCapsuleComponent()->GetComponentLocation());
	}
}

void AJunPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryInitializeCameraRotationFromController();
	UpdateCameraAnchor(DeltaTime);

	ValidateLockOnTarget();

	UpdateJunCamera(DeltaTime);

	UpdateCharacterRotationForCurrentCameraMode(DeltaTime);

	UpdateMovementSpeed(DeltaTime);

	ApplyRunningStateToAnimInstance(ShouldUseRunningAnim());

	UpdateBasicAttackJumpAndMoveCancelState(DeltaTime);

	UpdateBasicAttackRecoveryBuffer(DeltaTime);

	UpdateDodgeState(DeltaTime);

	UpdateDefenseInput(DeltaTime);

	UpdatePlayerHitState(DeltaTime);

	UpdateJumpStartAnimTrigger(DeltaTime);
}

void AJunPlayer::UpdateCameraAnchor(float DeltaTime)
{
	if (!CameraAnchor || !GetCapsuleComponent())
	{
		return;
	}

	const FVector TargetAnchorLocation = GetCapsuleComponent()->GetComponentLocation();
	const float InterpSpeed = HasGameplayTag(JunGameplayTags::State_Action_Dodge)
		? CameraAnchorDodgeInterpSpeed
		: CameraAnchorInterpSpeed;

	const FVector NewAnchorLocation = FMath::VInterpTo(
		CameraAnchor->GetComponentLocation(),
		TargetAnchorLocation,
		DeltaTime,
		InterpSpeed
	);

	CameraAnchor->SetWorldLocation(NewAnchorLocation);
}

void AJunPlayer::UpdateMovementBraking(float DeltaTime)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (KnockbackBrakingOverrideRemainTime > 0.f)
	{
		KnockbackBrakingOverrideRemainTime = FMath::Max(0.f, KnockbackBrakingOverrideRemainTime - DeltaTime);
		MovementComponent->BrakingDecelerationWalking = KnockbackBrakingDecelerationOverride;
		MovementComponent->GroundFriction = KnockbackGroundFrictionOverride;
		MovementComponent->BrakingFrictionFactor = KnockbackBrakingFrictionFactorOverride;
		return;
	}

	const bool bHasMoveInput =
		!FMath::IsNearlyZero(PendingMoveForward) ||
		!FMath::IsNearlyZero(PendingMoveRight);

	const bool bCanUseFreeRunSlide =
		!bLockOnActive &&
		DefenseState == EJunDefenseState::None &&
		!GetPlayerIsFalling() &&
		!HasGameplayTag(JunGameplayTags::State_Action_Dodge) &&
		!HasGameplayTag(JunGameplayTags::State_Block_Move);

	if (!bCanUseFreeRunSlide || bHasMoveInput)
	{
		bFreeRunSlideActive = false;
		FreeRunSlideSpeed = 0.f;
		RestoreDefaultMovementBrakingSettings();
		return;
	}

	RestoreDefaultMovementBrakingSettings();

	if (!bFreeRunSlideActive || FreeRunSlideDirection.IsNearlyZero())
	{
		return;
	}

	const float CurrentVerticalSpeed = MovementComponent->Velocity.Z;
	FreeRunSlideSpeed = FMath::Max(0.f, FreeRunSlideSpeed - (FreeRunStopBrakingDeceleration * DeltaTime));

	if (FreeRunSlideSpeed <= 0.f)
	{
		bFreeRunSlideActive = false;
		return;
	}

	MovementComponent->Velocity = FVector(
		FreeRunSlideDirection.X * FreeRunSlideSpeed,
		FreeRunSlideDirection.Y * FreeRunSlideSpeed,
		CurrentVerticalSpeed
	);
}

void AJunPlayer::OnMoveInputReleased()
{
	const bool bCanStartFreeRunSlide =
		!bLockOnActive &&
		DefenseState == EJunDefenseState::None &&
		!GetPlayerIsFalling() &&
		!HasGameplayTag(JunGameplayTags::State_Action_Dodge) &&
		!HasGameplayTag(JunGameplayTags::State_Block_Move);

	if (!bCanStartFreeRunSlide)
	{
		bFreeRunSlideActive = false;
		FreeRunSlideSpeed = 0.f;
		return;
	}

	FVector HorizontalVelocity = GetVelocity();
	HorizontalVelocity.Z = 0.f;

	const float HorizontalSpeed = HorizontalVelocity.Size();
	if (HorizontalSpeed < FreeRunSlideMinStartSpeed)
	{
		bFreeRunSlideActive = false;
		FreeRunSlideSpeed = 0.f;
		return;
	}

	bFreeRunSlideActive = true;
	FreeRunSlideDirection = HorizontalVelocity.GetSafeNormal();
	FreeRunSlideSpeed = HorizontalSpeed;
}

void AJunPlayer::CacheDefaultMovementBrakingSettings()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		DefaultWalkingBrakingDeceleration = MovementComponent->BrakingDecelerationWalking;
		DefaultGroundFriction = MovementComponent->GroundFriction;
		DefaultBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;
	}
}

void AJunPlayer::RestoreDefaultMovementBrakingSettings()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->BrakingDecelerationWalking = DefaultWalkingBrakingDeceleration;
		MovementComponent->GroundFriction = DefaultGroundFriction;
		MovementComponent->BrakingFrictionFactor = DefaultBrakingFrictionFactor;
	}
}

void AJunPlayer::TryInitializeCameraRotationFromController()
{
	if (bCameraRotationInitialized || !Controller)
	{
		return;
	}

	const FRotator ControlRot = Controller->GetControlRotation();
	TargetCameraYaw = ControlRot.Yaw;
	TargetCameraPitch = ControlRot.Pitch;
	bCameraRotationInitialized = true;
}

void AJunPlayer::UpdateMovementSpeed(float DeltaTime)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	UpdateMovementBraking(DeltaTime);

	TargetMaxWalkSpeed = GetDesiredMaxWalkSpeed();
	MovementComponent->MaxWalkSpeed = FMath::FInterpTo(
		MovementComponent->MaxWalkSpeed,
		TargetMaxWalkSpeed,
		DeltaTime,
		MoveSpeedInterpRate
	);
}

void AJunPlayer::ValidateLockOnTarget()
{
	if (!bLockOnActive || IsLockOnTargetValid())
	{
		return;
	}

	EndLockOn();
}

void AJunPlayer::UpdateCharacterRotationForCurrentCameraMode(float DeltaTime)
{
	if (bLockOnActive)
	{
		TryStartLockOnTurn();
		UpdateLockOnCharacterRotation(DeltaTime);
	}
	else
	{
		UpdateAttackFacing(DeltaTime);
	}
}

void AJunPlayer::UpdateJumpStartAnimTrigger(float DeltaTime)
{
	if (JumpStartAnimTriggerRemainTime > 0.f)
	{
		JumpStartAnimTriggerRemainTime = FMath::Max(0.f, JumpStartAnimTriggerRemainTime - DeltaTime);
	}
}

void AJunPlayer::TryStartLockOnTurn()
{
	if (!CanStartLockOnTurn())
	{
		return;
	}

	UAnimInstance* PlayerAnimInstance = GetPlayerAnimInstance();
	if (!PlayerAnimInstance)
	{
		return;
	}

	const float YawDelta = GetLockOnTargetYawDelta();
	UAnimMontage* TurnMontage = ChooseLockOnTurnMontage(YawDelta);
	if (!TurnMontage)
	{
		return;
	}

	PlayerAnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnLockOnTurnMontageEnded);
	PlayerAnimInstance->OnMontageEnded.AddDynamic(this, &AJunPlayer::OnLockOnTurnMontageEnded);

	if (PlayerAnimInstance->Montage_Play(TurnMontage) <= 0.f)
	{
		PlayerAnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnLockOnTurnMontageEnded);
		return;
	}

	CurrentLockOnTurnMontage = TurnMontage;
	bLockOnTurnInProgress = true;
}

void AJunPlayer::CancelLockOnTurn(float BlendOutTime)
{
	if (!IsLockOnTurnPlaying())
	{
		return;
	}

	const float ResolvedBlendOutTime =
		BlendOutTime >= 0.f ? BlendOutTime : LockOnTurnCancelBlendOutTime;
	UAnimMontage* PlayingTurnMontage = CurrentLockOnTurnMontage.Get();
	bLockOnTurnInProgress = false;
	CurrentLockOnTurnMontage = nullptr;

	if (UAnimInstance* PlayerAnimInstance = GetPlayerAnimInstance())
	{
		PlayerAnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnLockOnTurnMontageEnded);

		if (PlayingTurnMontage)
		{
			PlayerAnimInstance->Montage_Stop(ResolvedBlendOutTime, PlayingTurnMontage);
		}
	}
}

bool AJunPlayer::CanStartLockOnTurn() const
{
	if (!bLockOnActive || !LockOnTarget || bLockOnTurnInProgress)
	{
		return false;
	}

	if (DefenseState != EJunDefenseState::None)
	{
		return false;
	}

	if (bIsBasicAttacking || (GetCharacterMovement() && GetCharacterMovement()->IsFalling()))
	{
		return false;
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge) ||
		HasGameplayTag(JunGameplayTags::State_Condition_HitReact))
	{
		return false;
	}

	return GetVelocity().Size2D() <= LockOnTurnMaxGroundSpeed;
}

bool AJunPlayer::IsLockOnTurnPlaying() const
{
	return bLockOnTurnInProgress && CurrentLockOnTurnMontage != nullptr;
}

float AJunPlayer::GetLockOnTargetYawDelta() const
{
	if (!LockOnTarget)
	{
		return 0.f;
	}

	FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	if (ToTarget.IsNearlyZero())
	{
		return 0.f;
	}

	const FRotator TargetRotation = ToTarget.Rotation();
	return FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, TargetRotation.Yaw);
}

UAnimMontage* AJunPlayer::ChooseLockOnTurnMontage(float YawDelta) const
{
	const float AbsYawDelta = FMath::Abs(YawDelta);
	if (AbsYawDelta < LockOnTurnStartAngle)
	{
		return nullptr;
	}

	const bool bUse180 = AbsYawDelta >= LockOnTurn180Threshold;
	const bool bTurnRight = YawDelta > 0.f;

	if (bUse180)
	{
		return bTurnRight ? LockOnTurnRight180Montage.Get() : LockOnTurnLeft180Montage.Get();
	}

	return bTurnRight ? LockOnTurnRight90Montage.Get() : LockOnTurnLeft90Montage.Get();
}

float AJunPlayer::GetCurrentRunMoveSpeed() const
{
	return bLockOnActive ? LockOnRunMoveSpeed : FreeRunMoveSpeed;
}

FRotator AJunPlayer::GetJumpLaunchBasisRotation() const
{
	if (bLockOnActive)
	{
		return GetActorRotation();
	}

	if (Controller)
	{
		FRotator ControlRotation = Controller->GetControlRotation();
		ControlRotation.Pitch = 0.f;
		ControlRotation.Roll = 0.f;
		return ControlRotation;
	}

	return GetActorRotation();
}

FVector AJunPlayer::BuildJumpLaunchVelocity(const FVector2D& MoveInput) const
{
	FVector LaunchVelocity(0.f, 0.f, GetCharacterMovement() ? GetCharacterMovement()->JumpZVelocity : 0.f);
	const FVector LaunchDirection = GetJumpLaunchDirection(MoveInput);

	if (LaunchDirection.IsNearlyZero())
	{
		return LaunchVelocity;
	}

	FVector CurrentHorizontalVelocity = GetVelocity();
	CurrentHorizontalVelocity.Z = 0.f;

	float CurrentDirectionalHorizontalSpeed =
		FMath::Max(0.f, FVector::DotProduct(CurrentHorizontalVelocity, LaunchDirection));

	if (bLockOnActive)
	{
		CurrentDirectionalHorizontalSpeed = FMath::Max(
			CurrentDirectionalHorizontalSpeed,
			LockOnJumpMinimumHorizontalSpeed
		);
	}

	LaunchVelocity.X = LaunchDirection.X * CurrentDirectionalHorizontalSpeed;
	LaunchVelocity.Y = LaunchDirection.Y * CurrentDirectionalHorizontalSpeed;
	return LaunchVelocity;
}

void AJunPlayer::Jump()
{
	if (!CanJump())
	{
		return;
	}

	CancelLockOnTurn();
	JumpStartAnimTriggerRemainTime = JumpStartAnimTriggerDuration;

	const FVector2D MoveInput(DesiredMoveForward, DesiredMoveRight);
	LaunchCharacter(BuildJumpLaunchVelocity(MoveInput), true, true);
}

void AJunPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
}

void AJunPlayer::HandleGameplayEventNotify(FGameplayTag EventTag)
{
	Super::HandleGameplayEventNotify(EventTag);

	if (EventTag.MatchesTagExact(JunGameplayTags::Event_Notify_BasicAttack_ComboWindow))
	{
		OnBasicAttackComboWindowBegin();
	}
	else if (EventTag.MatchesTagExact(JunGameplayTags::Event_Notify_Dodge_Start))
	{
	}
	else if (EventTag.MatchesTagExact(JunGameplayTags::Event_Notify_Defense_ParryWindowBegin))
	{
		// Code-driven parry timing.
	}
	else if (EventTag.MatchesTagExact(JunGameplayTags::Event_Notify_Defense_ParryWindowEnd))
	{
		// Code-driven parry timing.
	}
	else if (EventTag.MatchesTagExact(JunGameplayTags::Event_Notify_Defense_EndBaseRelease))
	{
		// GuardEnd base release is now code-driven.
	}
}

void AJunPlayer::BeginAttackTraceWindow()
{
	if (!EquippedWeapon)
	{
		return;
	}

	EquippedWeapon->StartAttackTrace();
}

void AJunPlayer::EndAttackTraceWindow()
{
	if (!EquippedWeapon)
	{
		return;
	}

	EquippedWeapon->EndAttackTrace();
}

void AJunPlayer::BasicAttack()
{
	if (!BasicAttackMontage || !AnimInstance)
	{
		return;
	}

	if (!bIsBasicAttacking)
	{
		CancelLockOnTurn();
		StartBasicAttack();
		return;
	}

	if (bBasicAttackComboAdvanceStateActive)
	{
		bBufferedBasicAttackComboInput = true;
		TryAdvanceBasicAttackCombo();
		return;
	}

	if (bCanBufferBasicAttackComboInput)
	{
		bBufferedBasicAttackComboInput = true;
		return;
	}

	bBufferedBasicAttackInput = true;
}

void AJunPlayer::StartDodge()
{
	if (!AnimInstance)
	{
		return;
	}

	CancelLockOnTurn();

	// 援щⅤ湲곕뒗 ?낅젰 異⑸룎????븘???곹깭 ?쒓렇瑜?癒쇱? ?좉렐 ??紐쏀?二쇰? ?쒖옉?쒕떎.
	// ?먯쑀 移대찓?쇰뒗 ?낅젰 諛⑺뼢?쇰줈 罹먮┃?곕? 癒쇱? 留욎텛怨?
	// ?쎌삩? StartDodge ?덉뿉???좏깮??諛⑺뼢蹂?紐쏀?二쇨? ?뚰뵾 諛⑺뼢??梨낆엫吏꾨떎.
	UAnimMontage* DodgeMontageToPlay = GetDodgeMontageToPlay();
	if (!DodgeMontageToPlay)
	{
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		return;
	}

	if (HasGameplayTag(JunGameplayTags::State_Block_Dodge) || !bDodgeInputReleasedSinceLastDodge)
	{
		return;
	}

	GetCharacterMovement()->bOrientRotationToMovement = false;

	if (!bLockOnActive)
	{
		AlignActorToDesiredMoveDirectionForDodge();
	}

	AddGameplayTag(JunGameplayTags::State_Action_Dodge);
	AddGameplayTag(JunGameplayTags::State_Block_Dodge);
	AddGameplayTag(JunGameplayTags::State_Block_Move);
	bDodgeInputReleasedSinceLastDodge = false;
	if (DodgeMontageToPlay == LockOnDodgeBackMontage)
	{
		DodgeInternalCooldownRemainTime = BackwardDodgeInternalCooldownDuration;
		DodgeFinishRemainTime = BackwardDodgeFinishTime;
		DodgeInvincibleRemainTime = BackwardDodgeInvincibleDuration;
	}
	else if (DodgeMontageToPlay == LockOnDodgeLeftMontage || DodgeMontageToPlay == LockOnDodgeRightMontage)
	{
		DodgeInternalCooldownRemainTime = SideDodgeInternalCooldownDuration;
		DodgeFinishRemainTime = SideDodgeFinishTime;
		DodgeInvincibleRemainTime = SideDodgeInvincibleDuration;
	}
	else
	{
		DodgeInternalCooldownRemainTime = ForwardDodgeInternalCooldownDuration;
		DodgeFinishRemainTime = ForwardDodgeFinishTime;
		DodgeInvincibleRemainTime = ForwardDodgeInvincibleDuration;
	}

	if (DodgeInvincibleRemainTime > 0.f)
	{
		AddGameplayTag(JunGameplayTags::State_Condition_Invincible);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DodgeInvincibleRemainTime, FColor::Yellow, TEXT("Invincible"));
		}
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->StopAttackTrace();
	}

	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnDodgeMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AJunPlayer::OnDodgeMontageEnded);

	CurrentDodgeMontage = DodgeMontageToPlay;
	AnimInstance->Montage_Play(DodgeMontageToPlay);
}

void AJunPlayer::OnDefenseStarted()
{
	if (CurrentHitState == EJunPlayerHitState::ParrySuccess)
	{
		if (ChainParryWindowRemainTime > 0.f)
		{
			bDefenseButtonHeld = true;
			bHoldRequestedForCurrentDeflect = true;
			bParryWindowOpen = true;
			ParryWindowRemainTime = DefaultParryWindowDuration;
		}
		return;
	}

	if (HasGameplayTag(JunGameplayTags::State_Condition_HitReact))
	{
		return;
	}

	CancelLockOnTurn();

	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		bDefenseButtonHeld = true;
		return;
	}

	// Defense is split into:
	// None -> Starting(deflect attempt) -> Guarding(block) -> Ending -> None.
	// Repeated taps restart the deflect attempt immediately, while hold transitions into Guarding
	// only after GuardStart finishes and the button is still held.
	bDefenseButtonHeld = true;
	bHoldRequestedForCurrentDeflect = true;
	if (bIsBasicAttacking || CanCancelBasicAttackIntoRecoveryAction(EJunBufferedRecoveryAction::Defense))
	{
		if (CanCancelBasicAttackIntoRecoveryAction(EJunBufferedRecoveryAction::Defense))
		{
			BufferBasicAttackRecoveryAction(EJunBufferedRecoveryAction::Defense);
		}
		return;
	}

	if (DefenseState == EJunDefenseState::Guarding)
	{
		return;
	}

	if (DefenseState == EJunDefenseState::Starting || DefenseState == EJunDefenseState::Ending)
	{
		StartDefenseSequence();
		return;
	}

	StartDefenseSequence();
}

void AJunPlayer::OnDefenseReleased()
{
	// Release only changes intent during the deflect attempt.
	// BlockGuard transitions into Ending immediately on release.
	bDefenseButtonHeld = false;
	bHoldRequestedForCurrentDeflect = false;

	if (CurrentHitState == EJunPlayerHitState::GuardBlock)
	{
		if (AnimInstance && GuardBlockMontage)
		{
			AnimInstance->Montage_Stop(GuardBlockReleaseBlendOutTime, GuardBlockMontage);
		}

		CurrentHitState = EJunPlayerHitState::None;
		PlayerHitStateRemainTime = 0.f;
		CurrentHitReactType = EHitReactType::None;
		CurrentHitReactDirection = ECharacterHitReactDirection::Front_F;

		if (GetCharacterMovement())
		{
			RestoreDefaultMovementBrakingSettings();
		}

		RemoveGameplayTag(JunGameplayTags::State_Condition_HitReact);
		RemoveGameplayTag(JunGameplayTags::State_Condition_ControlLocked);
		RemoveGameplayTag(JunGameplayTags::State_Block_Move);
		RemoveGameplayTag(JunGameplayTags::State_Block_Jump);
		RemoveGameplayTag(JunGameplayTags::State_Block_Attack);
		RemoveGameplayTag(JunGameplayTags::State_Block_Dodge);

		GetWorldTimerManager().ClearTimer(GuardBlockReleaseIntoGuardEndTimerHandle);
		GetWorldTimerManager().SetTimer(
			GuardBlockReleaseIntoGuardEndTimerHandle,
			this,
			&AJunPlayer::CompleteGuardBlockReleaseIntoGuardEnd,
			GuardBlockReleaseBlendOutTime,
			false
		);
		return;
	}

	if (DefenseState == EJunDefenseState::Guarding)
	{
		BeginGuardEnd();
	}
}

void AJunPlayer::AddCameraLookInput(const FVector2D& Input)
{
	PendingCameraLookInput = Input;
}

void AJunPlayer::ToggleLockOn()
{
	if (bLockOnActive)
	{
		EndLockOn();
		return;
	}

	AJunCharacter* NewTarget = FindBestAttackTarget();
	if (!NewTarget)
	{
		return;
	}

	StartLockOn(NewTarget);
}

UAnimInstance* AJunPlayer::GetPlayerAnimInstance()
{
	if (!AnimInstance)
	{
		AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	}

	return AnimInstance;
}

void AJunPlayer::SpawnAndAttachWeapon()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (DefaultWeaponClass)
	{
		EquippedWeapon = World->SpawnActor<AWeaponActor>(
			DefaultWeaponClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (!EquippedWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn weapon."));
		}
		else
		{
			EquippedWeapon->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				WeaponSocketName
			);
		}
	}
}

void AJunPlayer::SpawnAndAttachScabbard()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (DefaultScabbardClass)
	{
		EquippedScabbard = World->SpawnActor<AWeaponActor>(
			DefaultScabbardClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (!EquippedScabbard)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn scabbard."));
		}
		else
		{
			EquippedScabbard->StopAttackTrace();
			EquippedScabbard->SetActorTickEnabled(false);
			EquippedScabbard->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				ScabbardSocketName
			);
		}
	}
}

bool AJunPlayer::GetPlayerIsFalling()
{
	return GetMovementComponent()->IsFalling();
}

bool AJunPlayer::IsLockOn()
{
	return bLockOnActive;
}

bool AJunPlayer::IsGuardOn()
{
	return DefenseState == EJunDefenseState::Guarding;
}

bool AJunPlayer::IsGuardPoseActive()
{
	return DefenseState != EJunDefenseState::None;
}

bool AJunPlayer::IsBasicAttacking() const
{
	return bIsBasicAttacking;
}

bool AJunPlayer::IsWalking() const
{
	return GetMoveState() == EJunMoveState::Walk;
}

bool AJunPlayer::IsSprinting() const
{
	return false;
}

bool AJunPlayer::IsInParrySuccess() const
{
	return CurrentHitState == EJunPlayerHitState::ParrySuccess;
}

bool AJunPlayer::IsJumpStartAnimTriggered() const
{
	return JumpStartAnimTriggerRemainTime > 0.f;
}

EJunMoveState AJunPlayer::GetMoveState() const
{
	if (DefenseState == EJunDefenseState::Starting ||
		DefenseState == EJunDefenseState::Guarding)
	{
		return EJunMoveState::Guard;
	}

	if (bWalkRequested)
	{
		return EJunMoveState::Walk;
	}

	return EJunMoveState::Run;
}

bool AJunPlayer::CanCancelBasicAttackIntoRecoveryAction(EJunBufferedRecoveryAction Action) const
{
	if (!bIsBasicAttacking)
	{
		switch (Action)
		{
		case EJunBufferedRecoveryAction::Dodge:
		case EJunBufferedRecoveryAction::Jump:
			return PostBasicAttackJumpDodgeBufferRemainTime > 0.f;
		case EJunBufferedRecoveryAction::Defense:
			return PostBasicAttackDefenseBufferRemainTime > 0.f;
		default:
			return false;
		}
	}

	const int32 ComboArrayIndex = FMath::Max(0, BasicAttackComboIndex - 1);
	const float DodgeCancelOpenTime = BasicAttackDodgeCancelOpenTimes.IsValidIndex(ComboArrayIndex)
		? BasicAttackDodgeCancelOpenTimes[ComboArrayIndex]
		: 0.18f;
	const float JumpCancelOpenTime = BasicAttackJumpCancelOpenTimes.IsValidIndex(ComboArrayIndex)
		? BasicAttackJumpCancelOpenTimes[ComboArrayIndex]
		: DodgeCancelOpenTime;
	const float DefenseCancelOpenTime = BasicAttackDefenseCancelOpenTimes.IsValidIndex(ComboArrayIndex)
		? BasicAttackDefenseCancelOpenTimes[ComboArrayIndex]
		: JumpCancelOpenTime;

	switch (Action)
	{
	case EJunBufferedRecoveryAction::Dodge:
		return BasicAttackSectionElapsedTime >= DodgeCancelOpenTime;
	case EJunBufferedRecoveryAction::Jump:
		return BasicAttackSectionElapsedTime >= JumpCancelOpenTime;
	case EJunBufferedRecoveryAction::Defense:
		return BasicAttackSectionElapsedTime >= DefenseCancelOpenTime;
	default:
		return false;
	}
}

bool AJunPlayer::CanBufferDefenseTransitionCancel() const
{
	return DefenseState == EJunDefenseState::Starting ||
		DefenseState == EJunDefenseState::Ending ||
		PostDefenseTransitionCancelBufferRemainTime > 0.f;
}

bool AJunPlayer::CanCancelDefenseTransitionIntoMove() const
{
	return DefenseState == EJunDefenseState::Ending &&
		DefenseTransitionElapsedTime >= DefenseMoveCancelOpenTime;
}

bool AJunPlayer::CanBufferParrySuccessCancel(EJunBufferedParrySuccessCancelAction Action) const
{
	if (CurrentHitState != EJunPlayerHitState::ParrySuccess)
	{
		return false;
	}

	switch (Action)
	{
	case EJunBufferedParrySuccessCancelAction::BasicAttack:
		return ParrySuccessElapsedTime >= ParrySuccessBasicAttackCancelOpenTime;
	case EJunBufferedParrySuccessCancelAction::Jump:
		return ParrySuccessElapsedTime >= ParrySuccessJumpCancelOpenTime;
	case EJunBufferedParrySuccessCancelAction::Dodge:
		return ParrySuccessElapsedTime >= ParrySuccessDodgeCancelOpenTime;
	case EJunBufferedParrySuccessCancelAction::Move:
		return ParrySuccessElapsedTime >= ParrySuccessMoveCancelOpenTime;
	default:
		return false;
	}
}

bool AJunPlayer::ShouldUseGuardBase() const
{
	return DefenseState == EJunDefenseState::Starting ||
		DefenseState == EJunDefenseState::Guarding ||
		(DefenseState == EJunDefenseState::Ending && bKeepGuardBaseWhileEnding);
}

EJunDefenseState AJunPlayer::GetDefenseState() const
{
	return DefenseState;
}

bool AJunPlayer::IsParryWindowOpen()
{
	return bParryWindowOpen;
}

float AJunPlayer::GetDesiredMoveForward() const
{
	return DesiredMoveForward;
}

float AJunPlayer::GetDesiredMoveRight() const
{
	return DesiredMoveRight;
}

float AJunPlayer::GetGuardMoveBlendRemainTime() const
{
	return GuardMoveBlendRemainTime;
}

bool AJunPlayer::ShouldDeferGuardMoveInput() const
{
	return bDeferGuardMoveInput;
}

float AJunPlayer::GetDesiredMaxWalkSpeed() const
{
	switch (GetMoveState())
	{
	case EJunMoveState::Walk:
		return WalkMoveSpeed;
	case EJunMoveState::Guard:
		return GuardMoveSpeed;
	case EJunMoveState::Sprint:
		return SprintMoveSpeed;
	case EJunMoveState::Run:
	default:
		return GetCurrentRunMoveSpeed();
	}
}

bool AJunPlayer::ShouldUseRunningAnim() const
{
	return GetMoveState() == EJunMoveState::Run;
}

void AJunPlayer::ToggleWalkingState()
{
	if (DefenseState == EJunDefenseState::Guarding)
	{
		return;
	}

	bWalkRequested = !bWalkRequested;

	if (bWalkRequested)
	{
		bSprintRequested = false;
	}

	TargetMaxWalkSpeed = GetDesiredMaxWalkSpeed();
	ApplyRunningStateToAnimInstance(ShouldUseRunningAnim());
}

void AJunPlayer::SetDesiredMoveAxes(float NewForward, float NewRight)
{
	PendingMoveForward = FMath::Clamp(NewForward, -1.f, 1.f);
	PendingMoveRight = FMath::Clamp(NewRight, -1.f, 1.f);

	if ((!FMath::IsNearlyZero(PendingMoveForward) || !FMath::IsNearlyZero(PendingMoveRight)) &&
		IsLockOnTurnPlaying())
	{
		CancelLockOnTurn();
	}

	if (bDeferGuardMoveInput)
	{
		return;
	}

	DesiredMoveForward = PendingMoveForward;
	DesiredMoveRight = PendingMoveRight;
}

void AJunPlayer::BufferBasicAttackRecoveryAction(EJunBufferedRecoveryAction Action)
{
	if (!CanCancelBasicAttackIntoRecoveryAction(Action))
	{
		return;
	}

	BufferedBasicAttackRecoveryAction = Action;
	TryExecuteBufferedBasicAttackRecoveryAction();
}

void AJunPlayer::BufferDefenseTransitionCancelAction(EJunBufferedDefenseCancelAction Action)
{
	const bool bCanBufferFromParryState =
		DefenseState == EJunDefenseState::Starting ||
		DefenseState == EJunDefenseState::Ending ||
		PostDefenseTransitionCancelBufferRemainTime > 0.f;

	if (!bCanBufferFromParryState)
	{
		return;
	}

	BufferedDefenseTransitionCancelAction = Action;
	TryExecuteBufferedDefenseTransitionCancelAction();
}

void AJunPlayer::BufferParrySuccessCancelAction(EJunBufferedParrySuccessCancelAction Action)
{
	if (CurrentHitState != EJunPlayerHitState::ParrySuccess)
	{
		return;
	}

	BufferedParrySuccessCancelAction = Action;
	TryExecuteBufferedParrySuccessCancelAction();
}

void AJunPlayer::ReceiveHit(EHitReactType HitType, float DamageAmount, AActor* DamageCauser, const FVector& SwingDirection)
{
	if (Is_Dead())
	{
		return;
	}

	LastIncomingSwingDirection = SwingDirection;
	LastIncomingKnockbackDirection = DetermineKnockbackDirectionFromDamageCauser(DamageCauser);

	const EJunPlayerHitResolveResult ResolveResult = ResolveIncomingHitResult(HitType);
	AJunCharacter* AttackerCharacter = Cast<AJunCharacter>(DamageCauser);

	switch (ResolveResult)
	{
	case EJunPlayerHitResolveResult::Ignored:
		return;
	case EJunPlayerHitResolveResult::ParrySuccess:
		StartParrySuccess();
		return;
	case EJunPlayerHitResolveResult::GuardBlock:
		StartGuardBlock();
		return;
	case EJunPlayerHitResolveResult::HitReact:
	default:
		OnDamaged(FMath::RoundToInt(DamageAmount), AttackerCharacter);
		if (Is_Dead())
		{
			return;
		}
		StartHitReact(HitType, DetermineHitReactDirection(SwingDirection));
		return;
	}
}

void AJunPlayer::OnBasicAttackComboWindowBegin()
{
	bCanBufferBasicAttackComboInput = true;

	if (bBufferedBasicAttackInput)
	{
		bBufferedBasicAttackComboInput = true;
		bBufferedBasicAttackInput = false;
	}
}

void AJunPlayer::OnBasicAttackComboAdvanceStateBegin()
{
	bBasicAttackComboAdvanceStateActive = true;
	bCanBufferBasicAttackComboInput = false;

	if (bBufferedBasicAttackComboInput)
	{
		TryAdvanceBasicAttackCombo();
	}
}

void AJunPlayer::OnBasicAttackComboAdvanceStateEnd()
{
	bBasicAttackComboAdvanceStateActive = false;
	bBufferedBasicAttackComboInput = false;
}

void AJunPlayer::UpdateDodgeState(float DeltaTime)
{
	if (DodgeInvincibleRemainTime > 0.f)
	{
		DodgeInvincibleRemainTime = FMath::Max(0.f, DodgeInvincibleRemainTime - DeltaTime);

		if (DodgeInvincibleRemainTime <= 0.f)
		{
			RemoveGameplayTag(JunGameplayTags::State_Condition_Invincible);
		}
	}

	if (DodgeInternalCooldownRemainTime > 0.f)
	{
		DodgeInternalCooldownRemainTime = FMath::Max(0.f, DodgeInternalCooldownRemainTime - DeltaTime);

		if (DodgeInternalCooldownRemainTime <= 0.f && !HasGameplayTag(JunGameplayTags::State_Action_Dodge))
		{
			RemoveGameplayTag(JunGameplayTags::State_Block_Dodge);
		}
	}

	if (!HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		return;
	}

	if (DodgeFinishRemainTime <= 0.f)
	{
		return;
	}

	DodgeFinishRemainTime = FMath::Max(0.f, DodgeFinishRemainTime - DeltaTime);

	if (DodgeFinishRemainTime <= 0.f)
	{
		FinishDodgeState();
	}
}

void AJunPlayer::OnDodgeInputReleased()
{
	bDodgeInputReleasedSinceLastDodge = true;
}

void AJunPlayer::TryExecuteBufferedBasicAttackRecoveryAction()
{
	if (BufferedBasicAttackRecoveryAction == EJunBufferedRecoveryAction::None ||
		!CanCancelBasicAttackIntoRecoveryAction(BufferedBasicAttackRecoveryAction))
	{
		return;
	}

	const EJunBufferedRecoveryAction PendingAction = BufferedBasicAttackRecoveryAction;
	BufferedBasicAttackRecoveryAction = EJunBufferedRecoveryAction::None;
	const bool bShouldCancelCurrentAttack = bIsBasicAttacking && CanCancelBasicAttackIntoRecoveryAction(PendingAction);
	PostBasicAttackJumpDodgeBufferRemainTime = 0.f;
	PostBasicAttackDefenseBufferRemainTime = 0.f;

	switch (PendingAction)
	{
	case EJunBufferedRecoveryAction::Dodge:
		if (bShouldCancelCurrentAttack)
		{
			CancelBasicAttackForRecoveryTransition(BasicAttackDodgeCancelBlendOutTime);
		}
		StartDodge();
		break;
	case EJunBufferedRecoveryAction::Jump:
		if (bShouldCancelCurrentAttack)
		{
			CancelBasicAttackForRecoveryTransition(BasicAttackJumpCancelBlendOutTime);
		}
		Jump();
		break;
	case EJunBufferedRecoveryAction::Defense:
		if (bShouldCancelCurrentAttack)
		{
			CancelBasicAttackForRecoveryTransition(BasicAttackDefenseCancelBlendOutTime);
		}
		BeginDefenseFromBufferedInput();
		break;
	default:
		break;
	}
}

void AJunPlayer::UpdateBasicAttackJumpAndMoveCancelState(float DeltaTime)
{
	if (!bIsBasicAttacking)
	{
		return;
	}

	BasicAttackSectionElapsedTime += DeltaTime;

	const int32 ComboArrayIndex = FMath::Max(0, BasicAttackComboIndex - 1);
	const float JumpCancelOpenTime = BasicAttackJumpCancelOpenTimes.IsValidIndex(ComboArrayIndex)
		? BasicAttackJumpCancelOpenTimes[ComboArrayIndex]
		: 0.18f;
	const float MoveCancelOpenTime = BasicAttackMoveCancelOpenTimes.IsValidIndex(ComboArrayIndex)
		? BasicAttackMoveCancelOpenTimes[ComboArrayIndex]
		: JumpCancelOpenTime;

	if (BasicAttackSectionElapsedTime >= JumpCancelOpenTime)
	{
		RemoveGameplayTag(JunGameplayTags::State_Block_Jump);
	}

	if (BasicAttackSectionElapsedTime >= MoveCancelOpenTime)
	{
		RemoveGameplayTag(JunGameplayTags::State_Block_Move);
	}
}

bool AJunPlayer::CanCancelBasicAttackIntoMove() const
{
	if (!bIsBasicAttacking)
	{
		return false;
	}

	const int32 ComboArrayIndex = FMath::Max(0, BasicAttackComboIndex - 1);
	const float MoveCancelOpenTime = BasicAttackMoveCancelOpenTimes.IsValidIndex(ComboArrayIndex)
		? BasicAttackMoveCancelOpenTimes[ComboArrayIndex]
		: 0.18f;

	return BasicAttackSectionElapsedTime >= MoveCancelOpenTime;
}

bool AJunPlayer::TryCancelBasicAttackIntoMove()
{
	if (!CanCancelBasicAttackIntoMove())
	{
		return false;
	}

	CancelBasicAttackForRecoveryTransition(BasicAttackMoveCancelBlendOutTime);
	return true;
}

void AJunPlayer::UpdateBasicAttackRecoveryBuffer(float DeltaTime)
{
	if (bIsBasicAttacking)
	{
		return;
	}

	if (PostBasicAttackJumpDodgeBufferRemainTime > 0.f)
	{
		PostBasicAttackJumpDodgeBufferRemainTime = FMath::Max(0.f, PostBasicAttackJumpDodgeBufferRemainTime - DeltaTime);
	}

	if (PostBasicAttackDefenseBufferRemainTime > 0.f)
	{
		PostBasicAttackDefenseBufferRemainTime = FMath::Max(0.f, PostBasicAttackDefenseBufferRemainTime - DeltaTime);
	}
}

void AJunPlayer::CompleteGuardBlockReleaseIntoGuardEnd()
{
	GetWorldTimerManager().ClearTimer(GuardBlockReleaseIntoGuardEndTimerHandle);

	if (DefenseState == EJunDefenseState::Guarding && !bDefenseButtonHeld)
	{
		BeginGuardEnd();
	}
}

void AJunPlayer::TryExecuteBufferedDefenseTransitionCancelAction()
{
	const bool bCanExecute =
		(DefenseState == EJunDefenseState::Starting ||
		 DefenseState == EJunDefenseState::Ending) ||
		PostDefenseTransitionCancelBufferRemainTime > 0.f;

	if (!bCanExecute)
	{
		return;
	}

	ExecuteBufferedDefenseTransitionCancelAction();
}

void AJunPlayer::ExecuteBufferedDefenseTransitionCancelAction()
{
	if (BufferedDefenseTransitionCancelAction == EJunBufferedDefenseCancelAction::None)
	{
		return;
	}

	const EJunBufferedDefenseCancelAction PendingAction = BufferedDefenseTransitionCancelAction;
	BufferedDefenseTransitionCancelAction = EJunBufferedDefenseCancelAction::None;
	PostDefenseTransitionCancelBufferRemainTime = 0.f;

	switch (PendingAction)
	{
	case EJunBufferedDefenseCancelAction::BasicAttack:
		CancelDefenseForCancelTransition();
		BasicAttack();
		break;
	case EJunBufferedDefenseCancelAction::Jump:
		CancelDefenseForCancelTransition();
		Jump();
		break;
	case EJunBufferedDefenseCancelAction::Dodge:
		CancelDefenseForCancelTransition();
		StartDodge();
		break;
	case EJunBufferedDefenseCancelAction::Move:
		if (!CanCancelDefenseTransitionIntoMove())
		{
			BufferedDefenseTransitionCancelAction = PendingAction;
			return;
		}

		CancelDefenseForCancelTransition(DefenseMoveCancelBlendOutTime);
		break;
	default:
		break;
	}
}

void AJunPlayer::TryExecuteBufferedParrySuccessCancelAction()
{
	if (BufferedParrySuccessCancelAction == EJunBufferedParrySuccessCancelAction::None)
	{
		return;
	}

	if (!CanBufferParrySuccessCancel(BufferedParrySuccessCancelAction))
	{
		return;
	}

	ExecuteBufferedParrySuccessCancelAction();
}

void AJunPlayer::CancelParrySuccessForCancelTransition(float BlendOutTime)
{
	if (AnimInstance && CurrentParrySuccessMontage)
	{
		AnimInstance->Montage_Stop(BlendOutTime, CurrentParrySuccessMontage);
	}

	FinishPlayerHitState();
}

void AJunPlayer::ExecuteBufferedParrySuccessCancelAction()
{
	if (BufferedParrySuccessCancelAction == EJunBufferedParrySuccessCancelAction::None)
	{
		return;
	}

	const EJunBufferedParrySuccessCancelAction PendingAction = BufferedParrySuccessCancelAction;
	BufferedParrySuccessCancelAction = EJunBufferedParrySuccessCancelAction::None;

	switch (PendingAction)
	{
	case EJunBufferedParrySuccessCancelAction::BasicAttack:
		CancelParrySuccessForCancelTransition(ParrySuccessBasicAttackCancelBlendOutTime);
		BasicAttack();
		break;
	case EJunBufferedParrySuccessCancelAction::Jump:
		CancelParrySuccessForCancelTransition(ParrySuccessJumpCancelBlendOutTime);
		Jump();
		break;
	case EJunBufferedParrySuccessCancelAction::Dodge:
		CancelParrySuccessForCancelTransition(ParrySuccessDodgeCancelBlendOutTime);
		StartDodge();
		break;
	case EJunBufferedParrySuccessCancelAction::Move:
		CancelParrySuccessForCancelTransition(ParrySuccessMoveCancelBlendOutTime);
		break;
	default:
		break;
	}
}

void AJunPlayer::CancelDefenseForCancelTransition(float BlendOutTime)
{
	if (DefenseState == EJunDefenseState::None || DefenseState == EJunDefenseState::Guarding)
	{
		return;
	}

	const float ResolvedBlendOutTime =
		BlendOutTime >= 0.f ? BlendOutTime : DefenseCancelBlendOutTime;

	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardStartMontageEnded);
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardEndMontageEnded);

		if (GuardStartMontage)
		{
			AnimInstance->Montage_Stop(ResolvedBlendOutTime, GuardStartMontage);
		}

		if (GuardEndMontage)
		{
			AnimInstance->Montage_Stop(ResolvedBlendOutTime, GuardEndMontage);
		}
	}

	FinishDefenseForCancel();
}

void AJunPlayer::FinishDodgeState()
{
	if (!HasGameplayTag(JunGameplayTags::State_Action_Dodge) && !CurrentDodgeMontage)
	{
		return;
	}

	RemoveGameplayTag(JunGameplayTags::State_Action_Dodge);
	RemoveGameplayTag(JunGameplayTags::State_Block_Move);
	DodgeFinishRemainTime = 0.f;
	DodgeInvincibleRemainTime = 0.f;
	RemoveGameplayTag(JunGameplayTags::State_Condition_Invincible);
	if (DodgeInternalCooldownRemainTime <= 0.f)
	{
		RemoveGameplayTag(JunGameplayTags::State_Block_Dodge);
	}
	GetCharacterMovement()->bOrientRotationToMovement = !bLockOnActive;
	CurrentDodgeMontage = nullptr;
}

void AJunPlayer::FinishDefenseForCancel()
{
	DefenseState = EJunDefenseState::None;
	bParryWindowOpen = false;
	ParryWindowRemainTime = 0.f;
	BufferedDefenseTransitionCancelAction = EJunBufferedDefenseCancelAction::None;
	bKeepGuardBaseWhileEnding = false;
	bDeferGuardMoveInput = false;
	bHoldRequestedForCurrentDeflect = false;
	bDeflectResolveReceived = false;
	DeflectResolveRemainTime = 0.f;
	DefenseTransitionElapsedTime = 0.f;
	GuardEndFinishRemainTime = 0.f;
	GuardEndBaseReleaseRemainTime = 0.f;
	PostDefenseTransitionCancelBufferRemainTime = 0.f;
	CurrentDeflectHeldDuration = 0.f;
	GuardExitMoveReleaseRemainTime = 0.f;
	GuardMoveBlendRemainTime = 0.f;
	ClearDefenseBlockTags();
	DesiredMoveForward = PendingMoveForward;
	DesiredMoveRight = PendingMoveRight;
}

void AJunPlayer::CancelBasicAttackForRecoveryTransition(float BlendOutTime)
{
	if (!bIsBasicAttacking)
	{
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->StopAttackTrace();
	}

	if (AnimInstance && BasicAttackMontage)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnBasicAttackMontageEnded);
		AnimInstance->Montage_Stop(BlendOutTime, BasicAttackMontage);
	}

	FinishBasicAttack();
}

void AJunPlayer::ResetBasicAttackCombo()
{
	FinishBasicAttack();
	bBufferedBasicAttackInput = false;
}

void AJunPlayer::FinishBasicAttack()
{
	EndAttackFacingAssist();

	RemoveGameplayTag(JunGameplayTags::State_Action_Attack);
	RemoveGameplayTag(JunGameplayTags::State_Block_Move);
	RemoveGameplayTag(JunGameplayTags::State_Block_Jump);

	bIsBasicAttacking = false;
	bCanBufferBasicAttackComboInput = false;
	bBufferedBasicAttackComboInput = false;
	bBasicAttackComboAdvanceStateActive = false;
	BufferedBasicAttackRecoveryAction = EJunBufferedRecoveryAction::None;
	PostBasicAttackJumpDodgeBufferRemainTime = PostBasicAttackJumpDodgeBufferDuration;
	PostBasicAttackDefenseBufferRemainTime = PostBasicAttackDefenseBufferDuration;
	BasicAttackSectionElapsedTime = 0.f;
	BasicAttackComboIndex = 0;
}

void AJunPlayer::StartBasicAttack()
{
	if (!AnimInstance || !BasicAttackMontage)
	{
		return;
	}

	// 怨듦꺽 ?쒖옉 ???寃??뚯쟾 蹂댁“/?대룞 李⑤떒/肄ㅻ낫 ?곹깭瑜???踰덉뿉 ?명똿?쒕떎.
	bBufferedBasicAttackInput = false;

	TargetActor = FindBestAttackTarget();
	BeginAttackFacingAssist(TargetActor);

	AddGameplayTag(JunGameplayTags::State_Action_Attack);
	AddGameplayTag(JunGameplayTags::State_Block_Move);
	AddGameplayTag(JunGameplayTags::State_Block_Jump);

	bIsBasicAttacking = true;
	bCanBufferBasicAttackComboInput = false;
	bBufferedBasicAttackComboInput = false;
	bBasicAttackComboAdvanceStateActive = false;
	BufferedBasicAttackRecoveryAction = EJunBufferedRecoveryAction::None;
	PostBasicAttackJumpDodgeBufferRemainTime = 0.f;
	PostBasicAttackDefenseBufferRemainTime = 0.f;
	BasicAttackSectionElapsedTime = 0.f;
	BasicAttackComboIndex = 1;

	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnBasicAttackMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AJunPlayer::OnBasicAttackMontageEnded);

	AnimInstance->Montage_Play(BasicAttackMontage);
	AnimInstance->Montage_JumpToSection(FName("1"), BasicAttackMontage);
}

void AJunPlayer::TryAdvanceBasicAttackCombo()
{
	if (!bIsBasicAttacking || !bBufferedBasicAttackComboInput || !BasicAttackMontage)
	{
		return;
	}

	if (!AnimInstance)
	{
		return;
	}

	FName NextSectionName = NAME_None;

	switch (BasicAttackComboIndex)
	{
	case 1:
		NextSectionName = FName("2");
		break;
	case 2:
		NextSectionName = FName("3");
		break;
	case 3:
		NextSectionName = FName("4");
		break;
	default:
		return;
	}

	bBufferedBasicAttackComboInput = false;
	bCanBufferBasicAttackComboInput = false;
	bBasicAttackComboAdvanceStateActive = false;
	BasicAttackComboIndex++;
	BasicAttackSectionElapsedTime = 0.f;

	TargetActor = FindBestAttackTarget();
	BeginAttackFacingAssist(TargetActor);

	AnimInstance->Montage_JumpToSection(NextSectionName, BasicAttackMontage);
}

void AJunPlayer::CancelBasicAttackIntoDefense()
{
	CancelBasicAttackForRecoveryTransition();
}

void AJunPlayer::OnBasicAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != BasicAttackMontage)
	{
		return;
	}

	if (bInterrupted)
	{
		return;
	}

	ResetBasicAttackCombo();

	if (EquippedWeapon)
	{
		EquippedWeapon->StopAttackTrace();
	}
}

void AJunPlayer::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentDodgeMontage)
	{
		return;
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		FinishDodgeState();
	}
}

void AJunPlayer::OnLockOnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentLockOnTurnMontage)
	{
		return;
	}

	if (UAnimInstance* PlayerAnimInstance = GetPlayerAnimInstance())
	{
		PlayerAnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnLockOnTurnMontageEnded);
	}

	bLockOnTurnInProgress = false;
	CurrentLockOnTurnMontage = nullptr;
}

void AJunPlayer::AlignActorToDesiredMoveDirectionForDodge()
{
	if (!Controller)
	{
		return;
	}

	FVector2D DesiredMoveInput(DesiredMoveForward, DesiredMoveRight);
	if (DesiredMoveInput.IsNearlyZero())
	{
		DesiredMoveInput = FVector2D(PendingMoveForward, PendingMoveRight);
	}

	if (DesiredMoveInput.IsNearlyZero())
	{
		return;
	}

	FRotator ControlRotation = Controller->GetControlRotation();
	ControlRotation.Pitch = 0.f;
	ControlRotation.Roll = 0.f;

	const FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(ControlRotation);
	const FVector RightDirection = UKismetMathLibrary::GetRightVector(ControlRotation);

	FVector DesiredWorldDirection =
		(ForwardDirection * DesiredMoveInput.X) +
		(RightDirection * DesiredMoveInput.Y);

	DesiredWorldDirection.Z = 0.f;

	if (DesiredWorldDirection.IsNearlyZero())
	{
		return;
	}

	SetActorRotation(DesiredWorldDirection.GetSafeNormal().Rotation());
}

UAnimMontage* AJunPlayer::GetDodgeMontageToPlay() const
{
	if (!bLockOnActive)
	{
		return DodgeMontage;
	}

	FVector2D DodgeInput(
		FMath::Clamp(DesiredMoveForward, -1.f, 1.f),
		FMath::Clamp(DesiredMoveRight, -1.f, 1.f)
	);

	if (DodgeInput.IsNearlyZero())
	{
		DodgeInput = FVector2D(
			FMath::Clamp(PendingMoveForward, -1.f, 1.f),
			FMath::Clamp(PendingMoveRight, -1.f, 1.f)
		);
	}

	if (DodgeInput.IsNearlyZero())
	{
		return DodgeMontage;
	}

	const float ForwardAbs = FMath::Abs(DodgeInput.X);
	const float RightAbs = FMath::Abs(DodgeInput.Y);

	// Lock-on dodge now uses 4-direction selection only.
	// If both axes are pressed, left/right wins on ties so diagonal input prefers side dodge.
	if (RightAbs >= ForwardAbs)
	{
		return DodgeInput.Y >= 0.f
			? (LockOnDodgeRightMontage ? LockOnDodgeRightMontage.Get() : DodgeMontage.Get())
			: (LockOnDodgeLeftMontage ? LockOnDodgeLeftMontage.Get() : DodgeMontage.Get());
	}

	return DodgeInput.X >= 0.f
		? DodgeMontage.Get()
		: (LockOnDodgeBackMontage ? LockOnDodgeBackMontage.Get() : DodgeMontage.Get());
}

void AJunPlayer::UpdateDefenseInput(float DeltaTime)
{
	// Tick owns the delayed movement release after GuardEnd.
	if (GuardMoveBlendRemainTime > 0.f)
	{
		GuardMoveBlendRemainTime = FMath::Max(0.f, GuardMoveBlendRemainTime - DeltaTime);
	}

	if (GuardExitMoveReleaseRemainTime > 0.f)
	{
		GuardExitMoveReleaseRemainTime = FMath::Max(0.f, GuardExitMoveReleaseRemainTime - DeltaTime);

		if (GuardExitMoveReleaseRemainTime <= 0.f)
		{
			bDeferGuardMoveInput = false;
			DesiredMoveForward = PendingMoveForward;
			DesiredMoveRight = PendingMoveRight;
			ClearDefenseBlockTags();
		}
	}

	if (DefenseState == EJunDefenseState::Starting || DefenseState == EJunDefenseState::Ending)
	{
		DefenseTransitionElapsedTime += DeltaTime;

		if (DefenseState == EJunDefenseState::Ending &&
			BufferedDefenseTransitionCancelAction != EJunBufferedDefenseCancelAction::None)
		{
			TryExecuteBufferedDefenseTransitionCancelAction();
		}
	}
	else
	{
		DefenseTransitionElapsedTime = 0.f;
	}

	if (DefenseState == EJunDefenseState::Starting && !bDeflectResolveReceived)
	{
		if (bHoldRequestedForCurrentDeflect)
		{
			CurrentDeflectHeldDuration += DeltaTime;
		}

		DeflectResolveRemainTime = FMath::Max(0.f, DeflectResolveRemainTime - DeltaTime);

		if (DeflectResolveRemainTime <= 0.f)
		{
			ResolveCurrentDeflectAttempt();
		}
	}

	if (DefenseState == EJunDefenseState::Ending && GuardEndFinishRemainTime > 0.f)
	{
		if (GuardEndBaseReleaseRemainTime > 0.f)
		{
			GuardEndBaseReleaseRemainTime = FMath::Max(0.f, GuardEndBaseReleaseRemainTime - DeltaTime);

			if (GuardEndBaseReleaseRemainTime <= 0.f)
			{
				bKeepGuardBaseWhileEnding = false;
			}
		}

		GuardEndFinishRemainTime = FMath::Max(0.f, GuardEndFinishRemainTime - DeltaTime);

		if (GuardEndFinishRemainTime <= 0.f)
		{
			DesiredMoveForward = 0.f;
			DesiredMoveRight = 0.f;
			PendingMoveForward = 0.f;
			PendingMoveRight = 0.f;

			if (BufferedDefenseTransitionCancelAction != EJunBufferedDefenseCancelAction::None)
			{
				TryExecuteBufferedDefenseTransitionCancelAction();
			}
			else
			{
				FinishDefense();
			}

			return;
		}
	}

	if (DefenseState == EJunDefenseState::None && PostDefenseTransitionCancelBufferRemainTime > 0.f)
	{
		PostDefenseTransitionCancelBufferRemainTime = FMath::Max(0.f, PostDefenseTransitionCancelBufferRemainTime - DeltaTime);

		if (BufferedDefenseTransitionCancelAction != EJunBufferedDefenseCancelAction::None)
		{
			ExecuteBufferedDefenseTransitionCancelAction();
			return;
		}
	}

	if (bParryWindowOpen)
	{
		ParryWindowRemainTime = FMath::Max(0.f, ParryWindowRemainTime - DeltaTime);

		if (ParryWindowRemainTime <= 0.f)
		{
			bParryWindowOpen = false;
		}
	}

}

EJunPlayerHitResolveResult AJunPlayer::ResolveIncomingHitResult(EHitReactType IncomingHitType) const
{
	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		return EJunPlayerHitResolveResult::Ignored;
	}

	if (bParryWindowOpen)
	{
		return EJunPlayerHitResolveResult::ParrySuccess;
	}

	if (DefenseState == EJunDefenseState::Guarding)
	{
		return EJunPlayerHitResolveResult::GuardBlock;
	}

	if (!CanBeInterruptedBy(IncomingHitType))
	{
		return EJunPlayerHitResolveResult::Ignored;
	}

	return EJunPlayerHitResolveResult::HitReact;
}

bool AJunPlayer::CanBeInterruptedBy(EHitReactType IncomingHitType) const
{
	if (CurrentHitState == EJunPlayerHitState::None)
	{
		return true;
	}

	if (CurrentHitState == EJunPlayerHitState::ParrySuccess)
	{
		return !bParryWindowOpen;
	}

	if (CurrentHitState == EJunPlayerHitState::GuardBlock)
	{
		return DefenseState == EJunDefenseState::Guarding;
	}

	if (CurrentHitState != EJunPlayerHitState::HitReact)
	{
		return true;
	}

	if (CurrentHitReactType == EHitReactType::HeavyHit &&
		IncomingHitType == EHitReactType::LightHit)
	{
		return false;
	}

	if (IncomingHitType == EHitReactType::HeavyHit)
	{
		return true;
	}

	if (CurrentHitReactType == EHitReactType::LightHit &&
		IncomingHitType == EHitReactType::LightHit)
	{
		return true;
	}

	return CurrentHitReactType == EHitReactType::None;
}

ECharacterHitReactDirection AJunPlayer::DetermineHitReactDirection(const FVector& SwingDirection) const
{
	const FVector SafeSwingDirection = SwingDirection.GetSafeNormal();
	if (SafeSwingDirection.IsNearlyZero())
	{
		return ECharacterHitReactDirection::Front_F;
	}

	const FVector LocalSwingDirection = GetActorTransform().InverseTransformVectorNoScale(SafeSwingDirection);
	const float SideValue = LocalSwingDirection.Y;
	const float SideThreshold = 0.35f;

	if (FMath::Abs(SideValue) >= SideThreshold)
	{
		return SideValue > 0.f
			? ECharacterHitReactDirection::Front_R
			: ECharacterHitReactDirection::Front_L;
	}

	return ECharacterHitReactDirection::Front_F;
}

ECharacterKnockbackDirection AJunPlayer::DetermineKnockbackDirectionFromDamageCauser(const AActor* DamageCauser) const
{
	if (!DamageCauser)
	{
		return ECharacterKnockbackDirection::Backward;
	}

	const FVector ToAttackerWorld = DamageCauser->GetActorLocation() - GetActorLocation();
	FVector ToAttackerLocal = GetActorTransform().InverseTransformVectorNoScale(ToAttackerWorld);
	ToAttackerLocal.Z = 0.f;

	if (ToAttackerLocal.IsNearlyZero())
	{
		return ECharacterKnockbackDirection::Backward;
	}

	if (FMath::Abs(ToAttackerLocal.Y) > FMath::Abs(ToAttackerLocal.X))
	{
		return ToAttackerLocal.Y > 0.f
			? ECharacterKnockbackDirection::Left
			: ECharacterKnockbackDirection::Right;
	}

	return ToAttackerLocal.X > 0.f
		? ECharacterKnockbackDirection::Backward
		: ECharacterKnockbackDirection::Forward;
}

UAnimMontage* AJunPlayer::GetHitReactMontage(EHitReactType HitType, ECharacterHitReactDirection HitDirection) const
{
	if (HitType != EHitReactType::LightHit)
	{
		return nullptr;
	}

	switch (HitDirection)
	{
	case ECharacterHitReactDirection::Front_L:
		return LightHitFront_LMontage;
	case ECharacterHitReactDirection::Front_R:
		return LightHitFront_RMontage;
	case ECharacterHitReactDirection::Front_F:
	default:
		return LightHitFront_FMontage;
	}
}

UAnimMontage* AJunPlayer::GetParrySuccessMontage(const FVector& SwingDirection) const
{
	const FVector SafeSwingDirection = SwingDirection.GetSafeNormal();
	if (SafeSwingDirection.IsNearlyZero())
	{
		return ParrySuccessR_UpMontage ? ParrySuccessR_UpMontage : ParrySuccessL_UpMontage;
	}

	const FVector LocalSwingDirection = GetActorTransform().InverseTransformVectorNoScale(SafeSwingDirection);
	const bool bUseLeft = LocalSwingDirection.Y < 0.f;
	const bool bUseUp = LocalSwingDirection.Z >= 0.f;

	if (bUseLeft)
	{
		return bUseUp ? ParrySuccessL_UpMontage : ParrySuccessL_DownMontage;
	}

	return bUseUp ? ParrySuccessR_UpMontage : ParrySuccessR_DownMontage;
}

void AJunPlayer::StartParrySuccess()
{
	InterruptActionsForHitReaction();

	CurrentHitState = EJunPlayerHitState::ParrySuccess;
	ChainParryWindowRemainTime = ChainParryWindowDuration;
	ParrySuccessElapsedTime = 0.f;
	BufferedParrySuccessCancelAction = EJunBufferedParrySuccessCancelAction::None;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.6f, FColor::Cyan, TEXT("ParrySuccess"));
	}
	AddGameplayTag(JunGameplayTags::State_Block_Move);
	AddGameplayTag(JunGameplayTags::State_Block_Jump);
	AddGameplayTag(JunGameplayTags::State_Block_Attack);
	AddGameplayTag(JunGameplayTags::State_Block_Dodge);
	ApplyCommonKnockback(
		LastIncomingKnockbackDirection,
		ParrySuccessKnockbackStrength,
		ParrySuccessKnockbackBrakingDeceleration,
		ParrySuccessKnockbackGroundFriction,
		ParrySuccessKnockbackBrakingFrictionFactor,
		ParrySuccessKnockbackOverrideDuration
	);

	UAnimMontage* ParrySuccessMontage = GetParrySuccessMontage(LastIncomingSwingDirection);
	CurrentParrySuccessMontage = ParrySuccessMontage;
	PlayerHitStateRemainTime = ParrySuccessMontage ? ParrySuccessMontage->GetPlayLength() : ParrySuccessDuration;

	if (ParrySuccessMontage)
	{
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.f, ParrySuccessMontage);
		}

		PlayAnimMontage(ParrySuccessMontage);
	}
}

void AJunPlayer::StartGuardBlock()
{
	CurrentHitState = EJunPlayerHitState::GuardBlock;
	PlayerHitStateRemainTime = GuardBlockMontage ? GuardBlockMontage->GetPlayLength() : GuardBlockDuration;
	AddGameplayTag(JunGameplayTags::State_Block_Move);
	AddGameplayTag(JunGameplayTags::State_Block_Jump);
	AddGameplayTag(JunGameplayTags::State_Block_Attack);
	AddGameplayTag(JunGameplayTags::State_Block_Dodge);
	ApplyCommonKnockback(
		LastIncomingKnockbackDirection,
		GuardBlockKnockbackStrength,
		GuardBlockKnockbackBrakingDeceleration,
		GuardBlockKnockbackGroundFriction,
		GuardBlockKnockbackBrakingFrictionFactor,
		GuardBlockKnockbackOverrideDuration
	);

	if (GuardBlockMontage)
	{
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.f, GuardBlockMontage);
		}

		PlayAnimMontage(GuardBlockMontage);
	}
}

void AJunPlayer::StartHitReact(EHitReactType HitType, ECharacterHitReactDirection HitDirection)
{
	InterruptActionsForHitReaction();

	CurrentHitState = EJunPlayerHitState::HitReact;
	CurrentHitReactType = HitType;
	CurrentHitReactDirection = HitDirection;

	UAnimMontage* HitReactMontage = GetHitReactMontage(HitType, HitDirection);
	if (HitType == EHitReactType::LightHit)
	{
		PlayerHitStateRemainTime = LightHitReactDuration;
	}
	else
	{
		PlayerHitStateRemainTime = HitReactMontage ? HitReactMontage->GetPlayLength() : HitReactDuration;
	}

	AddGameplayTag(JunGameplayTags::State_Condition_HitReact);
	AddGameplayTag(JunGameplayTags::State_Condition_ControlLocked);

	if (HitReactMontage)
	{
		if (AnimInstance)
		{
			//AnimInstance->Montage_Stop(0.f, HitReactMontage);
		}

		PlayAnimMontage(HitReactMontage);
	}
}

void AJunPlayer::ApplyCommonKnockback(
	ECharacterKnockbackDirection KnockbackDirectionType,
	float Strength,
	float BrakingDecelerationOverride,
	float GroundFrictionOverride,
	float BrakingFrictionFactorOverride,
	float OverrideDuration
)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || Strength <= 0.f)
	{
		return;
	}

	FVector KnockbackDirection = FVector::ZeroVector;
	switch (KnockbackDirectionType)
	{
	case ECharacterKnockbackDirection::Forward:
		KnockbackDirection = GetActorForwardVector();
		break;
	case ECharacterKnockbackDirection::Backward:
		KnockbackDirection = -GetActorForwardVector();
		break;
	case ECharacterKnockbackDirection::Left:
		KnockbackDirection = -GetActorRightVector();
		break;
	case ECharacterKnockbackDirection::Right:
		KnockbackDirection = GetActorRightVector();
		break;
	default:
		KnockbackDirection = -GetActorForwardVector();
		break;
	}

	KnockbackDirection.Z = 0.f;
	KnockbackDirection = KnockbackDirection.GetSafeNormal();
	if (KnockbackDirection.IsNearlyZero())
	{
		return;
	}

	KnockbackBrakingDecelerationOverride = BrakingDecelerationOverride;
	KnockbackGroundFrictionOverride = GroundFrictionOverride;
	KnockbackBrakingFrictionFactorOverride = BrakingFrictionFactorOverride;
	KnockbackBrakingOverrideRemainTime = OverrideDuration;

	MovementComponent->AddImpulse(KnockbackDirection * Strength, true);
}

void AJunPlayer::InterruptActionsForHitReaction()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopAttackTrace();
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		FinishDodgeState();
	}

	if (bIsBasicAttacking)
	{
		CancelBasicAttackForRecoveryTransition();
	}

	if (DefenseState != EJunDefenseState::None)
	{
		if (AnimInstance)
		{
			if (GuardStartMontage)
			{
				AnimInstance->Montage_Stop(0.05f, GuardStartMontage);
			}

			if (GuardEndMontage)
			{
				AnimInstance->Montage_Stop(0.05f, GuardEndMontage);
			}
		}

		FinishDefenseForCancel();
	}

	AddGameplayTag(JunGameplayTags::State_Block_Move);
	AddGameplayTag(JunGameplayTags::State_Block_Jump);
	AddGameplayTag(JunGameplayTags::State_Block_Attack);
	AddGameplayTag(JunGameplayTags::State_Block_Dodge);
}

void AJunPlayer::UpdatePlayerHitState(float DeltaTime)
{
	if (ChainParryWindowRemainTime > 0.f)
	{
		ChainParryWindowRemainTime = FMath::Max(0.f, ChainParryWindowRemainTime - DeltaTime);
	}

	if (CurrentHitState == EJunPlayerHitState::None)
	{
		return;
	}

	if (CurrentHitState == EJunPlayerHitState::ParrySuccess)
	{
		ParrySuccessElapsedTime += DeltaTime;

		if (BufferedParrySuccessCancelAction != EJunBufferedParrySuccessCancelAction::None)
		{
			TryExecuteBufferedParrySuccessCancelAction();
			if (CurrentHitState == EJunPlayerHitState::None)
			{
				return;
			}
		}
	}

	PlayerHitStateRemainTime = FMath::Max(0.f, PlayerHitStateRemainTime - DeltaTime);

	if (PlayerHitStateRemainTime <= 0.f)
	{
		FinishPlayerHitState();
	}
}

void AJunPlayer::FinishPlayerHitState()
{
	CurrentHitState = EJunPlayerHitState::None;
	PlayerHitStateRemainTime = 0.f;
	ChainParryWindowRemainTime = 0.f;
	ParrySuccessElapsedTime = 0.f;
	CurrentHitReactType = EHitReactType::None;
	CurrentHitReactDirection = ECharacterHitReactDirection::Front_F;
	BufferedParrySuccessCancelAction = EJunBufferedParrySuccessCancelAction::None;
	CurrentParrySuccessMontage = nullptr;

	if (GetCharacterMovement())
	{
		RestoreDefaultMovementBrakingSettings();
	}

	RemoveGameplayTag(JunGameplayTags::State_Condition_HitReact);
	RemoveGameplayTag(JunGameplayTags::State_Condition_ControlLocked);
	RemoveGameplayTag(JunGameplayTags::State_Block_Move);
	RemoveGameplayTag(JunGameplayTags::State_Block_Jump);
	RemoveGameplayTag(JunGameplayTags::State_Block_Attack);
	RemoveGameplayTag(JunGameplayTags::State_Block_Dodge);

	if (DefenseState == EJunDefenseState::Guarding)
	{
		ApplyGuardOnBlockTags();
	}
	else if (DefenseState == EJunDefenseState::Starting || DefenseState == EJunDefenseState::Ending)
	{
		ApplyDefenseStartBlockTags();
	}
}

void AJunPlayer::StartDefenseSequence()
{
	// Every tap/press starts a fresh deflect attempt from frame 0 of GuardStart.
	const bool bIsImmediateRestartCycle =
		DefenseState == EJunDefenseState::Starting ||
		DefenseState == EJunDefenseState::Ending;

	bParryWindowOpen = true;
	ParryWindowRemainTime = DefaultParryWindowDuration;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DefaultParryWindowDuration, FColor::Green, TEXT("ParryEnable"));
	}
	BufferedDefenseTransitionCancelAction = EJunBufferedDefenseCancelAction::None;
	bHoldRequestedForCurrentDeflect = bDefenseButtonHeld;
	bDeflectResolveReceived = false;
	DeflectResolveRemainTime =
		(GuardStartMontage ? GuardStartMontage->GetPlayLength() : DefaultDeflectResolveTime);
	GuardEndFinishRemainTime = 0.f;
	GuardEndBaseReleaseRemainTime = 0.f;
	PostDefenseTransitionCancelBufferRemainTime = 0.f;
	CurrentDeflectHeldDuration = 0.f;
	DefenseTransitionElapsedTime = 0.f;
	bKeepGuardBaseWhileEnding = false;
	GuardExitMoveReleaseRemainTime = 0.f;
	bDeferGuardMoveInput = true;
	DesiredMoveForward = 0.f;
	DesiredMoveRight = 0.f;
	PendingMoveForward = 0.f;
	PendingMoveRight = 0.f;

	ApplyDefenseStartBlockTags();

	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardStartMontageEnded);
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardEndMontageEnded);

		if (GuardEndMontage)
		{
			AnimInstance->Montage_Stop(0.05f, GuardEndMontage);
		}

		if (GuardStartMontage)
		{
			AnimInstance->Montage_Stop(0.02f, GuardStartMontage);
		}
	}

	DefenseState = EJunDefenseState::Starting;

	if (!AnimInstance || !GuardStartMontage)
	{
		if (bDefenseButtonHeld)
		{
			EnterGuardLoop();
		}
		else
		{
			BeginGuardEnd();
		}
		return;
	}

	AnimInstance->OnMontageEnded.AddDynamic(this, &AJunPlayer::OnGuardStartMontageEnded);
	AnimInstance->Montage_Play(GuardStartMontage);
}

void AJunPlayer::BeginDefenseFromBufferedInput()
{
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		return;
	}

	StartDefenseSequence();
}

void AJunPlayer::ResolveCurrentDeflectAttempt()
{
	if (DefenseState != EJunDefenseState::Starting || bDeflectResolveReceived)
	{
		return;
	}

	bDeflectResolveReceived = true;

	const bool bShouldEnterGuardLoop =
		bHoldRequestedForCurrentDeflect &&
		CurrentDeflectHeldDuration >= GuardEnterHoldThreshold;

	if (bShouldEnterGuardLoop)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Guard"));
		}
		EnterGuardLoop();
		return;
	}
	BeginGuardEnd();
}

void AJunPlayer::ApplyDefenseStartBlockTags()
{
	AddGameplayTag(JunGameplayTags::State_Block_Move);
	AddGameplayTag(JunGameplayTags::State_Block_Jump);
	AddGameplayTag(JunGameplayTags::State_Block_Attack);
	AddGameplayTag(JunGameplayTags::State_Block_Dodge);
}

void AJunPlayer::ApplyGuardOnBlockTags()
{
	RemoveGameplayTag(JunGameplayTags::State_Block_Move);
	AddGameplayTag(JunGameplayTags::State_Block_Jump);
	AddGameplayTag(JunGameplayTags::State_Block_Attack);
	AddGameplayTag(JunGameplayTags::State_Block_Dodge);
}

void AJunPlayer::ClearDefenseBlockTags()
{
	RemoveGameplayTag(JunGameplayTags::State_Block_Move);
	RemoveGameplayTag(JunGameplayTags::State_Block_Jump);
	RemoveGameplayTag(JunGameplayTags::State_Block_Attack);
	RemoveGameplayTag(JunGameplayTags::State_Block_Dodge);
}

void AJunPlayer::EnterGuardLoop()
{
	// Guarding is the only sustained block state. Start/End remain montage-driven.
	DefenseState = EJunDefenseState::Guarding;
	bParryWindowOpen = false;
	BufferedDefenseTransitionCancelAction = EJunBufferedDefenseCancelAction::None;
	bHoldRequestedForCurrentDeflect = false;
	bDeflectResolveReceived = false;
	DeflectResolveRemainTime = 0.f;
	GuardEndFinishRemainTime = 0.f;
	GuardEndBaseReleaseRemainTime = 0.f;
	PostDefenseTransitionCancelBufferRemainTime = 0.f;
	CurrentDeflectHeldDuration = 0.f;
	DefenseTransitionElapsedTime = 0.f;
	bDeferGuardMoveInput = false;
	DesiredMoveForward = PendingMoveForward;
	DesiredMoveRight = PendingMoveRight;
	GuardMoveBlendRemainTime = GuardMoveBlendDuration;
	ApplyGuardOnBlockTags();
}

void AJunPlayer::BeginGuardEnd()
{
	// Ending keeps the guard base, but movement should already be available once
	// GuardStart has fully resolved. Only non-move defense blocks remain here.
	if (DefenseState == EJunDefenseState::None || DefenseState == EJunDefenseState::Ending)
	{
		return;
	}

	DefenseState = EJunDefenseState::Ending;
	bParryWindowOpen = false;
	ParryWindowRemainTime = 0.f;
	bHoldRequestedForCurrentDeflect = false;
	bDeflectResolveReceived = false;
	DeflectResolveRemainTime = 0.f;
	GuardEndFinishRemainTime =
		GuardEndMontage
			? FMath::Max(0.f, GuardEndMontage->GetPlayLength() - GuardEndFinishTimeOffset)
			: 0.f;
	GuardEndBaseReleaseRemainTime =
		GuardEndMontage
			? FMath::Max(0.f, GuardEndMontage->GetPlayLength() - GuardEndBaseReleaseTimeOffset)
			: 0.f;
	PostDefenseTransitionCancelBufferRemainTime = 0.f;
	CurrentDeflectHeldDuration = 0.f;
	DefenseTransitionElapsedTime = 0.f;
	bKeepGuardBaseWhileEnding = true;
	bDeferGuardMoveInput = true;
	DesiredMoveForward = 0.f;
	DesiredMoveRight = 0.f;
	GuardMoveBlendRemainTime = GuardMoveBlendDuration;
	ApplyDefenseStartBlockTags();

	if (!AnimInstance || !GuardEndMontage)
	{
		FinishDefense();
		return;
	}

	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardStartMontageEnded);
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardEndMontageEnded);
	AnimInstance->Montage_Stop(0.02f, GuardStartMontage);
	AnimInstance->OnMontageEnded.AddDynamic(this, &AJunPlayer::OnGuardEndMontageEnded);

	if (AnimInstance->Montage_Play(GuardEndMontage) <= 0.f)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardEndMontageEnded);
		FinishDefense();
		return;
	}

	if (BufferedDefenseTransitionCancelAction != EJunBufferedDefenseCancelAction::None)
	{
		TryExecuteBufferedDefenseTransitionCancelAction();
	}
}

void AJunPlayer::FinishDefense()
{
	DefenseState = EJunDefenseState::None;
	bParryWindowOpen = false;
	ParryWindowRemainTime = 0.f;
	BufferedDefenseTransitionCancelAction = EJunBufferedDefenseCancelAction::None;
	bKeepGuardBaseWhileEnding = false;
	bDeferGuardMoveInput = true;
	bHoldRequestedForCurrentDeflect = false;
	bDeflectResolveReceived = false;
	DeflectResolveRemainTime = 0.f;
	DefenseTransitionElapsedTime = 0.f;
	GuardEndFinishRemainTime = 0.f;
	GuardEndBaseReleaseRemainTime = 0.f;
	PostDefenseTransitionCancelBufferRemainTime = PostDefenseTransitionCancelBufferDuration;
	CurrentDeflectHeldDuration = 0.f;
	GuardMoveBlendRemainTime = GuardExitMoveBlendDuration;
	GuardExitMoveReleaseRemainTime = GuardExitMoveReleaseDelay;
}

void AJunPlayer::OnGuardStartMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != GuardStartMontage)
	{
		return;
	}

	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardStartMontageEnded);
	}

	if (DefenseState != EJunDefenseState::Starting)
	{
		return;
	}

	if (bInterrupted)
	{
		return;
	}

	ResolveCurrentDeflectAttempt();
}

void AJunPlayer::OnGuardEndMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != GuardEndMontage)
	{
		return;
	}

	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &AJunPlayer::OnGuardEndMontageEnded);
	}

	if (BufferedDefenseTransitionCancelAction != EJunBufferedDefenseCancelAction::None)
	{
		TryExecuteBufferedDefenseTransitionCancelAction();
		return;
	}

	if (GuardEndFinishRemainTime <= 0.f)
	{
		FinishDefense();
	}
}

void AJunPlayer::ApplyRunningStateToAnimInstance(bool bNewIsRunning)
{
	if (!AnimInstance)
	{
		AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	}

	if (!AnimInstance)
	{
		return;
	}

	if (FBoolProperty* BoolProp = FindFProperty<FBoolProperty>(AnimInstance->GetClass(), TEXT("bIsRunning")))
	{
		BoolProp->SetPropertyValue_InContainer(AnimInstance, bNewIsRunning);
	}
}

FVector AJunPlayer::GetJumpLaunchDirection(const FVector2D& MoveInput) const
{
	if (MoveInput.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const FRotator BasisRotation = GetJumpLaunchBasisRotation();
	const FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(BasisRotation);
	const FVector RightDirection = UKismetMathLibrary::GetRightVector(BasisRotation);

	FVector LaunchDirection =
		(ForwardDirection * MoveInput.X) +
		(RightDirection * MoveInput.Y);

	LaunchDirection.Z = 0.f;
	return LaunchDirection.GetSafeNormal();
}

AJunCharacter* AJunPlayer::FindBestAttackTarget()
{
	const FVector Start = GetActorLocation();
	const float Radius = 500.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<AActor*> OutActors;

	const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Start,
		Radius,
		ObjectTypes,
		AJunCharacter::StaticClass(),
		ActorsToIgnore,
		OutActors
	);

	if (!bHit)
	{
		return nullptr;
	}

	AJunCharacter* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	const FVector MyLocation = GetActorLocation();
	const FVector CameraForward = GetCameraForwardOnPlane();

	for (AActor* HitActor : OutActors)
	{
		AJunCharacter* Candidate = Cast<AJunCharacter>(HitActor);
		if (!Candidate || Candidate == this)
		{
			continue;
		}

		if (IsSameTeam(Candidate))
		{
			continue;
		}

		FVector ToTarget = Candidate->GetActorLocation() - MyLocation;
		ToTarget.Z = 0.f;

		const float Distance = ToTarget.Length();
		if (Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		ToTarget.Normalize();

		const float Dot = FVector::DotProduct(CameraForward, ToTarget);
		if (Dot < 0.2f)
		{
			continue;
		}

		const float Score = Dot * 1000.f - Distance;
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void AJunPlayer::FaceTargetForAttack(AJunCharacter* NewTarget)
{
	if (!NewTarget)
	{
		return;
	}

	FVector Direction = NewTarget->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = Direction.Rotation();
	SetActorRotation(TargetRotation);
}

void AJunPlayer::UpdateAttackFacing(float DeltaTime)
{
	if (!bUseAttackFacingAssist)
	{
		return;
	}

	if (!TargetActor)
	{
		EndAttackFacingAssist();
		return;
	}

	AttackFacingAssistRemainTime -= DeltaTime;
	if (AttackFacingAssistRemainTime <= 0.f)
	{
		EndAttackFacingAssist();
		return;
	}

	FVector Direction = TargetActor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = Direction.Rotation();

	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		AttackFacingInterpSpeed
	);

	SetActorRotation(NewRotation);
}

void AJunPlayer::BeginAttackFacingAssist(AJunCharacter* NewTarget)
{
	TargetActor = NewTarget;

	if (!TargetActor)
	{
		bUseAttackFacingAssist = false;
		AttackFacingAssistRemainTime = 0.f;
		return;
	}

	FVector Direction = TargetActor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;

	if (Direction.IsNearlyZero())
	{
		bUseAttackFacingAssist = false;
		AttackFacingAssistRemainTime = 0.f;
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = Direction.Rotation();

	const float DeltaYaw = FMath::Abs(
		FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw)
	);

	const float MaxFacingAssistAngle = 70.f;
	if (DeltaYaw > MaxFacingAssistAngle)
	{
		// ?꾩슂?섎㈃ ?뚯쟾 蹂댁젙??留됰뒗 洹쒖튃???ㅼ떆 ?쒖꽦?뷀븳??
	}

	bUseAttackFacingAssist = true;
	AttackFacingAssistRemainTime = AttackFacingAssistDuration;
}

void AJunPlayer::EndAttackFacingAssist()
{
	bUseAttackFacingAssist = false;
	AttackFacingAssistRemainTime = 0.f;
}

void AJunPlayer::UpdateJunCamera(float DeltaTime)
{
	switch (CameraMode)
	{
	case EJunCameraMode::LockOn:
		UpdateLockOnCamera(DeltaTime);
		break;
	case EJunCameraMode::Free:
	default:
		UpdateFreeCamera(DeltaTime);
		break;
	}

	FVector TargetSocketOffset = bLockOnActive ? LockOnCameraSocketOffset : FreeCameraSocketOffset;

	if (!bLockOnActive && HasGameplayTag(JunGameplayTags::State_Action_Dodge))
	{
		TargetSocketOffset += DodgeCameraSocketOffset;
	}

	const FVector NewSocketOffset = FMath::VInterpTo(
		SpringArm->SocketOffset,
		TargetSocketOffset,
		DeltaTime,
		CameraSocketOffsetInterpSpeed
	);

	SpringArm->SocketOffset = NewSocketOffset;
}

void AJunPlayer::UpdateFreeCamera(float DeltaTime)
{
	if (!Controller)
	{
		PendingCameraLookInput = FVector2D::ZeroVector;
		return;
	}

	FVector2D Input = PendingCameraLookInput;

	if (GetVelocity().SizeSquared2D() > 0.f)
	{
		Input *= MovingLookInputScale;
	}

	if (HasGameplayTag(JunGameplayTags::State_Action_Attack))
	{
		Input *= AttackLookInputScale;
	}

	TargetCameraYaw += Input.X * FreeCameraYawSpeed * DeltaTime;
	TargetCameraPitch += Input.Y * FreeCameraPitchSpeed * DeltaTime;
	TargetCameraPitch = FMath::Clamp(TargetCameraPitch, MinCameraPitch, MaxCameraPitch);

	const FRotator CurrentRot = Controller->GetControlRotation();
	const FRotator TargetRot(TargetCameraPitch, TargetCameraYaw, 0.f);

	const FRotator NewRot = FMath::RInterpTo(
		CurrentRot,
		TargetRot,
		DeltaTime,
		FreeCameraRotationInterpSpeed
	);

	Controller->SetControlRotation(NewRot);
	PendingCameraLookInput = FVector2D::ZeroVector;
}

FVector AJunPlayer::GetCameraForwardOnPlane() const
{
	if (!Controller)
	{
		return GetActorForwardVector();
	}

	FRotator ControlRot = Controller->GetControlRotation();
	ControlRot.Pitch = 0.f;
	ControlRot.Roll = 0.f;

	return ControlRot.Vector().GetSafeNormal();
}

void AJunPlayer::StartLockOn(AJunCharacter* NewTarget)
{
	if (!NewTarget)
	{
		return;
	}

	bLockOnActive = true;
	LockOnTarget = NewTarget;
	CameraMode = EJunCameraMode::LockOn;
	CachedLockOnTargetPoint = FVector::ZeroVector;

	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AJunPlayer::EndLockOn()
{
	bLockOnActive = false;
	LockOnTarget = nullptr;
	CameraMode = EJunCameraMode::Free;
	CachedLockOnTargetPoint = FVector::ZeroVector;
	CancelLockOnTurn(0.05f);

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

bool AJunPlayer::IsLockOnTargetValid() const
{
	if (!LockOnTarget)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), LockOnTarget->GetActorLocation());
	return DistSq <= FMath::Square(LockOnBreakDistance);
}

void AJunPlayer::UpdateLockOnCamera(float DeltaTime)
{
	if (!Controller || !LockOnTarget)
	{
		PendingCameraLookInput = FVector2D::ZeroVector;
		return;
	}

	FVector CameraBasePoint = CameraAnchor
		? CameraAnchor->GetComponentLocation()
		: GetActorLocation();
	CameraBasePoint.Z += 50.f;

	const FVector TargetBonePoint = GetFilteredLockOnTargetPoint();
	const FVector ToTarget = TargetBonePoint - CameraBasePoint;
	const FVector DebugSpherePoint = GetRawLockOnBonePoint() - (ToTarget.GetSafeNormal() * 20.f);

	DrawDebugSphere(GetWorld(), DebugSpherePoint, 6.f, 12, FColor::Red, false, 0.f);

	if (ToTarget.IsNearlyZero())
	{
		PendingCameraLookInput = FVector2D::ZeroVector;
		return;
	}

	const float Distance2D = FVector(ToTarget.X, ToTarget.Y, 0.f).Length();
	const float DeltaZ = ToTarget.Z;

	FRotator DesiredRot = ToTarget.Rotation();
	DesiredRot.Roll = 0.f;

	const float TargetPitchDeg = FMath::RadiansToDegrees(
		FMath::Atan2(DeltaZ, FMath::Max(Distance2D, 1.f))
	);

	DesiredRot.Pitch = LockOnPitchOffset + TargetPitchDeg;

	const FRotator CurrentRot = Controller->GetControlRotation();

	if (GetCharacterMovement()->IsFalling())
	{
		DesiredRot.Pitch = FMath::Lerp(CurrentRot.Pitch, DesiredRot.Pitch, 0.35f);
	}

	DesiredRot.Pitch = FMath::Clamp(DesiredRot.Pitch, MinCameraPitch, MaxCameraPitch);

	const FRotator NewRot = FMath::RInterpTo(
		CurrentRot,
		DesiredRot,
		DeltaTime,
		LockOnRotationInterpSpeed
	);

	Controller->SetControlRotation(NewRot);
	TargetCameraYaw = NewRot.Yaw;
	TargetCameraPitch = NewRot.Pitch;
	PendingCameraLookInput = FVector2D::ZeroVector;
}

void AJunPlayer::UpdateLockOnCharacterRotation(float DeltaTime)
{
	if (!bLockOnActive || !LockOnTarget || IsLockOnTurnPlaying())
	{
		return;
	}

	FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRot = GetActorRotation();
	const FRotator DesiredRot = ToTarget.Rotation();

	const FRotator NewRot = FMath::RInterpTo(
		CurrentRot,
		DesiredRot,
		DeltaTime,
		LockOnCharacterRotationInterpSpeed
	);

	SetActorRotation(NewRot);
}

FVector AJunPlayer::GetRawLockOnBonePoint() const
{
	if (!LockOnTarget)
	{
		return FVector::ZeroVector;
	}

	USkeletalMeshComponent* TargetMesh = LockOnTarget->GetMesh();
	if (!TargetMesh)
	{
		return LockOnTarget->GetActorLocation() + FVector(0.f, 0.f, 40.f);
	}

	static const FName LockOnBoneName(TEXT("spine_03"));

	if (TargetMesh->GetBoneIndex(LockOnBoneName) != INDEX_NONE)
	{
		// 이 본 위치는 락온 UI 그릴때 사용
		return TargetMesh->GetBoneLocation(LockOnBoneName);
	}

	return LockOnTarget->GetActorLocation() + FVector(0.f, 0.f, 40.f);
}

FVector AJunPlayer::GetRawLockOnCapsulePoint() const
{
	if (!LockOnTarget)
	{
		return FVector::ZeroVector;
	}

	return LockOnTarget->GetActorLocation() + FVector(0.f, 0.f, 40.f);
}

FVector AJunPlayer::GetFilteredLockOnTargetPoint()
{
	FVector RawPoint = GetRawLockOnCapsulePoint();
	if (LockOnTarget)
	{
		const FVector TargetLockOnPoint = LockOnTarget->GetLockOnTargetPoint();
		RawPoint.Z = TargetLockOnPoint.Z;
	}
	
	if (CachedLockOnTargetPoint.IsNearlyZero())
	{
		CachedLockOnTargetPoint = RawPoint;
		return CachedLockOnTargetPoint;
	}

	const FVector2D CachedXY(CachedLockOnTargetPoint.X, CachedLockOnTargetPoint.Y);
	const FVector2D RawXY(RawPoint.X, RawPoint.Y);

	const float XYDelta = FVector2D::Distance(CachedXY, RawXY);
	const float ZDelta = FMath::Abs(RawPoint.Z - CachedLockOnTargetPoint.Z);

	if (XYDelta >= LockOnTargetXYIgnoreThreshold)
	{
		CachedLockOnTargetPoint.X = RawPoint.X;
		CachedLockOnTargetPoint.Y = RawPoint.Y;
	}

	if (ZDelta >= LockOnTargetZIgnoreThreshold)
	{
		CachedLockOnTargetPoint.Z = RawPoint.Z;
	}

	return CachedLockOnTargetPoint;
}




