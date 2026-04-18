#include "Character/WukongBoss.h"

#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	const TCHAR* GetWukongCombatStateDebugText(EWukongCombatState State)
	{
		switch (State)
		{
		case EWukongCombatState::Approach:
			return TEXT("Approach");
		case EWukongCombatState::Idle:
			return TEXT("Idle");
		case EWukongCombatState::Attack:
			return TEXT("Attack");
		case EWukongCombatState::Recovery:
			return TEXT("Recovery");
		case EWukongCombatState::Reposition:
			return TEXT("Reposition");
		case EWukongCombatState::Evade:
			return TEXT("Evade");
		case EWukongCombatState::Turn:
			return TEXT("Turn");
		default:
			return TEXT("Unknown");
		}
	}

	const TCHAR* GetWukongPlannedAttackDebugText(EWukongPlannedAttackType AttackType)
	{
		switch (AttackType)
		{
		case EWukongPlannedAttackType::ComboAttack:
			return TEXT("ComboAttack");
		case EWukongPlannedAttackType::JumpAttack:
			return TEXT("JumpAttack");
		case EWukongPlannedAttackType::ChargeAttack:
			return TEXT("ChargeAttack");
		case EWukongPlannedAttackType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* GetWukongPlannedActionDebugText(EWukongPlannedActionType ActionType)
	{
		switch (ActionType)
		{
		case EWukongPlannedActionType::Attack:
			return TEXT("Attack");
		case EWukongPlannedActionType::Movement:
			return TEXT("Movement");
		case EWukongPlannedActionType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* GetWukongPlannedMovementDebugText(EWukongPlannedMovementType MovementType)
	{
		switch (MovementType)
		{
		case EWukongPlannedMovementType::Strafe:
			return TEXT("Strafe");
		case EWukongPlannedMovementType::Dash:
			return TEXT("Dash");
		case EWukongPlannedMovementType::Dodge:
			return TEXT("Dodge");
		case EWukongPlannedMovementType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* GetWukongComboSetDebugText(EWukongComboSet ComboSet)
	{
		switch (ComboSet)
		{
		case EWukongComboSet::ComboA:
			return TEXT("ComboA");
		case EWukongComboSet::ComboB:
			return TEXT("ComboB");
		case EWukongComboSet::None:
		default:
			return TEXT("None");
		}
	}
}

AWukongBoss::AWukongBoss()
{
}

void AWukongBoss::EnterCombatState()
{
	Super::EnterCombatState();
	SetWukongCombatState(EWukongCombatState::Approach);
}

void AWukongBoss::UpdateCombat(float DeltaTime)
{
	if (!CurrentTarget)
	{
		RestoreJumpAttackGroundMotionOverride();
		SetMonsterState(EMonsterState::Patrol);
		return;
	}

	if (!CanRemainInCombat(CurrentTarget))
	{
		RestoreJumpAttackGroundMotionOverride();
		LastKnownTargetLocation = CurrentTarget->GetActorLocation();
		CurrentTarget = nullptr;
		bIsHasTarget = false;
		CombatMoveInput = FVector2D::ZeroVector;
		SetMonsterState(EMonsterState::Return);
		return;
	}

	if (GEngine)
	{
		const FMonsterAttackSelection DebugSelection = ChooseNextAttackSelection();
		const FString SelectedMontageName = DebugSelection.Montage
			? DebugSelection.Montage->GetName()
			: TEXT("None");

		const FString DebugText = FString::Printf(
			TEXT("Wukong SubState: %s\nPlannedAction: %s\nPlannedAttack: %s\nPlannedMovement: %s\nCombo: %s / %d\nSelectedMontage: %s\nCanAttack: %s"),
			GetWukongCombatStateDebugText(CurrentCombatSubState),
			GetWukongPlannedActionDebugText(PlannedActionType),
			GetWukongPlannedAttackDebugText(PlannedAttackType),
			GetWukongPlannedMovementDebugText(PlannedMovementType),
			GetWukongComboSetDebugText(PlannedComboSet),
			PlannedComboLength,
			*SelectedMontageName,
			CanAttackTarget() ? TEXT("true") : TEXT("false")
		);

		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(reinterpret_cast<UPTRINT>(this)) + 1000,
			0.f,
			FColor::Cyan,
			DebugText
		);
	}

	CombatSubStateElapsedTime += DeltaTime;

	switch (CurrentCombatSubState)
	{
	case EWukongCombatState::Approach:
		UpdateApproachState(DeltaTime);
		break;
	case EWukongCombatState::Idle:
		UpdateIdleState(DeltaTime);
		break;
	case EWukongCombatState::Attack:
		UpdateAttackState(DeltaTime);
		break;
	case EWukongCombatState::Recovery:
		UpdateRecoveryState(DeltaTime);
		break;
	case EWukongCombatState::Reposition:
		UpdateRepositionState(DeltaTime);
		break;
	case EWukongCombatState::Evade:
		UpdateEvadeState(DeltaTime);
		break;
	case EWukongCombatState::Turn:
		UpdateTurnState(DeltaTime);
		break;
	default:
		break;
	}
}

FMonsterAttackSelection AWukongBoss::ChooseNextAttackSelection() const
{
	FMonsterAttackSelection Selection = Super::ChooseNextAttackSelection();

	if (PlannedAttackType == EWukongPlannedAttackType::JumpAttack && JumpAttackMontage)
	{
		Selection.Montage = JumpAttackMontage;
		Selection.AttackRange = JumpAttackMaxRange;
		Selection.bFaceTargetDuringAttack = true;
		Selection.FacingDuration = 0.6f;
		Selection.FacingInterpSpeed = 24.f;
		Selection.bTryTurnAfterAttack = true;
		Selection.PostAttackTurnStartAngle = TurnStartAngle;
		return Selection;
	}

	if (PlannedAttackType == EWukongPlannedAttackType::ChargeAttack && ChargeAttackMontage)
	{
		Selection.Montage = ChargeAttackMontage;
		Selection.AttackRange = ChargeAttackRange;
		Selection.bFaceTargetDuringAttack = bFaceTargetDuringChargeAttack;
		Selection.FacingDuration = ChargeAttackFacingDuration;
		Selection.FacingInterpSpeed = ChargeAttackFacingInterpSpeed;
		Selection.bTryTurnAfterAttack = bTryTurnAfterChargeAttack;
		Selection.PostAttackTurnStartAngle = ChargeAttackTurnReacquireAngle;
		return Selection;
	}

	if (PlannedAttackType == EWukongPlannedAttackType::ComboAttack)
	{
		if (UAnimMontage* PlannedComboMontage = GetPlannedComboMontage())
		{
			Selection.Montage = PlannedComboMontage;
			Selection.AttackRange = BasicAttackRange;
			Selection.bFaceTargetDuringAttack = true;
			Selection.FacingDuration = GetPlannedComboFacingDuration();
			Selection.FacingInterpSpeed = AttackFacingInterpSpeed;
			Selection.bTryTurnAfterAttack = true;
			Selection.PostAttackTurnStartAngle = TurnStartAngle;
			return Selection;
		}
	}

	return Selection;
}

void AWukongBoss::SetWukongCombatState(EWukongCombatState NewState)
{
	if (CurrentCombatSubState == NewState)
	{
		return;
	}

	CurrentCombatSubState = NewState;
	CombatSubStateElapsedTime = 0.f;

	switch (CurrentCombatSubState)
	{
	case EWukongCombatState::Approach:
		EnterApproachState();
		break;
	case EWukongCombatState::Idle:
		EnterIdleState();
		break;
	case EWukongCombatState::Attack:
		EnterAttackState();
		break;
	case EWukongCombatState::Recovery:
		EnterRecoveryState();
		break;
	case EWukongCombatState::Reposition:
		EnterRepositionState();
		break;
	case EWukongCombatState::Evade:
		EnterEvadeState();
		break;
	case EWukongCombatState::Turn:
		EnterTurnState();
		break;
	default:
		break;
	}
}

void AWukongBoss::EnterApproachState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bRunLocomotionRequested = false;
}

void AWukongBoss::EnterIdleState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	bRunLocomotionRequested = false;
	RefreshPlannedCombatPlan();
}

void AWukongBoss::EnterAttackState()
{
	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();
	bPendingJumpAttackLaunch = false;
	JumpAttackLaunchDelayRemainTime = 0.f;
	CurrentAttackActionType = EWukongActionType::None;
	CurrentAttackComboSet = EWukongComboSet::None;
	CurrentAttackComboLength = 0;

	if (AttackSelection.Montage == JumpAttackMontage)
	{
		CurrentAttackActionType = EWukongActionType::JumpAttack;
	}
	else if (AttackSelection.Montage == ChargeAttackMontage)
	{
		CurrentAttackActionType = EWukongActionType::ChargeAttack;
	}
	else if (AttackSelection.Montage == GetPlannedComboMontage())
	{
		CurrentAttackActionType = EWukongActionType::ComboAttack;
		CurrentAttackComboSet = PlannedComboSet;
		CurrentAttackComboLength = PlannedComboLength;
	}

	if (AttackSelection.Montage == JumpAttackMontage)
	{
		bPendingJumpAttackLaunch = true;
		JumpAttackLaunchDelayRemainTime = JumpAttackStartDelay;
	}

	TryAttack();
}

void AWukongBoss::EnterRecoveryState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	RestoreJumpAttackGroundMotionOverride();

	if (CurrentAttackActionType != EWukongActionType::None)
	{
		RecordAction(CurrentAttackActionType, CurrentAttackComboSet, CurrentAttackComboLength);
	}

	if (DeferredAttackTypeAfterApproachDodge != EWukongPlannedAttackType::None)
	{
		PlannedActionType = EWukongPlannedActionType::Attack;
		PlannedAttackType = DeferredAttackTypeAfterApproachDodge;
		PlannedComboSet = DeferredComboSetAfterApproachDodge;
		PlannedComboLength = DeferredComboLengthAfterApproachDodge;
		DeferredAttackTypeAfterApproachDodge = EWukongPlannedAttackType::None;
		DeferredComboSetAfterApproachDodge = EWukongComboSet::None;
		DeferredComboLengthAfterApproachDodge = 0;
	}
	else
	{
		ResetPlannedCombatPlan();
	}

	CurrentAttackActionType = EWukongActionType::None;
	CurrentAttackComboSet = EWukongComboSet::None;
	CurrentAttackComboLength = 0;
}

void AWukongBoss::EnterRepositionState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	bRunLocomotionRequested = false;

	if (PlannedActionType == EWukongPlannedActionType::Movement &&
		PlannedMovementType == EWukongPlannedMovementType::Strafe &&
		PlannedMovementDirection == EWukongMovementDirection::None)
	{
		PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;
	}

	if (PlannedActionType == EWukongPlannedActionType::Movement &&
		PlannedMovementType == EWukongPlannedMovementType::Strafe &&
		PlannedMovementDuration <= 0.f)
	{
		PlannedMovementDuration = StrafeMinDuration;
	}

	CurrentRepositionDirection = PlannedMovementDirection == EWukongMovementDirection::Left ? -1.f : 1.f;
}

void AWukongBoss::EnterEvadeState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	bRunLocomotionRequested = false;

	if (CurrentTarget)
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;
		if (!ToTarget.IsNearlyZero())
		{
			SetActorRotation(ToTarget.Rotation());
		}
	}

	if (UAnimMontage* MovementMontage = GetPlannedMovementMontage())
	{
		PlayAnimMontage(MovementMontage);
		PlannedMovementDuration = FMath::Max(PlannedMovementDuration, MovementMontage->GetPlayLength());
	}
	else
	{
		PlannedMovementDuration = FMath::Max(PlannedMovementDuration, EvadeFallbackDuration);
	}
}

void AWukongBoss::EnterTurnState()
{
	bRunLocomotionRequested = false;
	TryStartCombatTurnWithThreshold(TurnStartAngle);
}

void AWukongBoss::UpdateApproachState(float DeltaTime)
{
	EnsureCombatPlan();

	if (PlannedActionType != EWukongPlannedActionType::Attack || !HasValidPlannedAttack())
	{
		SetWukongCombatState(EWukongCombatState::Idle);
		return;
	}

	if (GetTargetYawAbsDelta() >= MoveTurnStartAngle && CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	UpdateFacingTowardsTargetWithoutTurn(DeltaTime, ApproachFacingInterpSpeed);

	if (IsTargetTooFarForPlannedAttack())
	{
		bRunLocomotionRequested = CombatSubStateElapsedTime >= ApproachRunStartDelay;

		if (CombatSubStateElapsedTime >= ApproachDodgeFallbackDelay && DodgeForwardMontage)
		{
			DeferredAttackTypeAfterApproachDodge = PlannedAttackType;
			DeferredComboSetAfterApproachDodge = PlannedComboSet;
			DeferredComboLengthAfterApproachDodge = PlannedComboLength;
			PlannedActionType = EWukongPlannedActionType::Movement;
			PlannedMovementType = EWukongPlannedMovementType::Dodge;
			PlannedMovementDirection = EWukongMovementDirection::Forward;
			PlannedMovementDuration = 0.f;
			SetWukongCombatState(EWukongCombatState::Evade);
			return;
		}
	}
	else
	{
		bRunLocomotionRequested = false;
	}

	if (IsTargetTooCloseForPlannedAttack())
	{
		SetWukongCombatState(EWukongCombatState::Reposition);
		return;
	}

	if (!IsTargetTooFarForPlannedAttack())
	{
		const float TargetYawAbsDelta = GetTargetYawAbsDelta();

		if (TargetYawAbsDelta <= IdleEntryYawTolerance)
		{
			StopAIMovement();
			if (CanAttackTarget())
			{
				SetWukongCombatState(EWukongCombatState::Attack);
			}
			else
			{
				SetWukongCombatState(EWukongCombatState::Idle);
			}
			return;
		}

		SetWukongCombatState(EWukongCombatState::Reposition);
		return;
	}

	if (CurrentTarget)
	{
		MoveToTarget(CurrentTarget);
	}
}

void AWukongBoss::UpdateIdleState(float DeltaTime)
{
	EnsureCombatPlan();

	if (CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	if (GetTargetYawAbsDelta() > IdleEntryYawTolerance)
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack)
	{
		if (!HasValidPlannedAttack())
		{
			ResetPlannedCombatPlan();
			return;
		}

		if (IsTargetTooFarForPlannedAttack())
		{
			SetWukongCombatState(EWukongCombatState::Approach);
			return;
		}

		if (IsTargetTooCloseForPlannedAttack())
		{
			SetWukongCombatState(EWukongCombatState::Reposition);
			return;
		}

		if (CanAttackTarget())
		{
			SetWukongCombatState(EWukongCombatState::Attack);
			return;
		}

		SetWukongCombatState(EWukongCombatState::Reposition);
		return;
	}

	if (!HasValidPlannedMovement())
	{
		ResetPlannedCombatPlan();
		return;
	}

	SetWukongCombatState(
		PlannedMovementType == EWukongPlannedMovementType::Strafe
			? EWukongCombatState::Reposition
			: EWukongCombatState::Evade
	);
}

void AWukongBoss::UpdateAttackState(float DeltaTime)
{
	if (!bIsAttacking)
	{
		bPendingJumpAttackLaunch = false;
		JumpAttackLaunchDelayRemainTime = 0.f;
		RestoreJumpAttackGroundMotionOverride();
		SetWukongCombatState(EWukongCombatState::Recovery);
	}
}

void AWukongBoss::UpdateRecoveryState(float DeltaTime)
{
	if (CombatSubStateElapsedTime < RecoveryDuration)
	{
		return;
	}

	EnsureCombatPlan();

	if (CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	if (GetTargetYawAbsDelta() > IdleEntryYawTolerance)
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack)
	{
		if (IsTargetTooFarForPlannedAttack())
		{
			SetWukongCombatState(EWukongCombatState::Approach);
			return;
		}

		if (IsTargetTooCloseForPlannedAttack())
		{
			SetWukongCombatState(EWukongCombatState::Reposition);
			return;
		}
	}

	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateRepositionState(float DeltaTime)
{
	EnsureCombatPlan();

	if (GetTargetYawAbsDelta() >= MoveTurnStartAngle && CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	UpdateFacingTowardsTargetWithoutTurn(DeltaTime, CombatFacingInterpSpeed);

	if (PlannedActionType == EWukongPlannedActionType::Attack && IsTargetTooFarForPlannedAttack())
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return;
	}

	if (CombatSubStateElapsedTime <= DeltaTime && PlannedMovementType == EWukongPlannedMovementType::Strafe)
	{
		CurrentRepositionDirection = FMath::RandBool() ? 1.f : -1.f;
	}

	if (PlannedActionType == EWukongPlannedActionType::Movement &&
		PlannedMovementType == EWukongPlannedMovementType::Strafe)
	{
		const FVector RightDirection = GetActorRightVector();
		AddMovementInput(RightDirection, CurrentRepositionDirection * StrafeMoveInputStrength);
		SetDesiredMoveAxes(0.f, CurrentRepositionDirection * StrafeMoveInputStrength);

		if (CombatSubStateElapsedTime >= PlannedMovementDuration)
		{
			CombatMoveInput = FVector2D::ZeroVector;
			SetDesiredMoveAxes(0.f, 0.f);
			ResetPlannedCombatPlan();
			SetWukongCombatState(EWukongCombatState::Idle);
		}

		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack && IsTargetTooCloseForPlannedAttack())
	{
		const FVector BackwardDirection = -GetActorForwardVector();
		AddMovementInput(BackwardDirection, AttackBackOffMoveInputStrength);
		SetDesiredMoveAxes(-AttackBackOffMoveInputStrength, 0.f);

		if (!IsTargetTooCloseForPlannedAttack() || CombatSubStateElapsedTime >= AttackBackOffDuration)
		{
			CombatMoveInput = FVector2D::ZeroVector;
			SetDesiredMoveAxes(0.f, 0.f);
			SetWukongCombatState(EWukongCombatState::Idle);
		}

		return;
	}

	SetDesiredMoveAxes(0.f, 0.f);
	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateEvadeState(float DeltaTime)
{
	if (CombatSubStateElapsedTime < PlannedMovementDuration)
	{
		return;
	}

	switch (PlannedMovementType)
	{
	case EWukongPlannedMovementType::Dash:
		RecordAction(EWukongActionType::Dash);
		break;
	case EWukongPlannedMovementType::Dodge:
		RecordAction(EWukongActionType::Dodge);
		break;
	default:
		break;
	}

	if (DeferredAttackTypeAfterApproachDodge != EWukongPlannedAttackType::None)
	{
		PlannedActionType = EWukongPlannedActionType::Attack;
		PlannedAttackType = DeferredAttackTypeAfterApproachDodge;
		PlannedComboSet = DeferredComboSetAfterApproachDodge;
		PlannedComboLength = DeferredComboLengthAfterApproachDodge;
		DeferredAttackTypeAfterApproachDodge = EWukongPlannedAttackType::None;
		DeferredComboSetAfterApproachDodge = EWukongComboSet::None;
		DeferredComboLengthAfterApproachDodge = 0;
	}
	else
	{
		ResetPlannedCombatPlan();
	}

	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateTurnState(float DeltaTime)
{
	if (!IsCombatTurnPlaying())
	{
		SetWukongCombatState(EWukongCombatState::Idle);
	}
}

bool AWukongBoss::CanUseChargeAttack() const
{
	return ChargeAttackMontage != nullptr && CurrentTarget != nullptr;
}

bool AWukongBoss::CanUseJumpAttack() const
{
	return JumpAttackMontage != nullptr && CurrentTarget != nullptr;
}

bool AWukongBoss::CanStartWukongTurn() const
{
	if (!CanStartCombatTurn())
	{
		return false;
	}

	return GetTargetYawAbsDelta() >= TurnStartAngle;
}

bool AWukongBoss::HasValidPlannedAttack() const
{
	switch (PlannedAttackType)
	{
	case EWukongPlannedAttackType::ComboAttack:
		return GetPlannedComboMontage() != nullptr;
	case EWukongPlannedAttackType::JumpAttack:
		return JumpAttackMontage != nullptr;
	case EWukongPlannedAttackType::ChargeAttack:
		return ChargeAttackMontage != nullptr;
	case EWukongPlannedAttackType::None:
	default:
		return false;
	}
}

bool AWukongBoss::HasValidPlannedMovement() const
{
	switch (PlannedMovementType)
	{
	case EWukongPlannedMovementType::Strafe:
		return true;
	case EWukongPlannedMovementType::Dash:
	case EWukongPlannedMovementType::Dodge:
		return GetPlannedMovementMontage() != nullptr;
	case EWukongPlannedMovementType::None:
	default:
		return false;
	}
}

bool AWukongBoss::IsWithinSelectionRange(const FMonsterAttackSelection& Selection) const
{
	if (!CurrentTarget || !Selection.Montage)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared2D(GetActorLocation(), CurrentTarget->GetActorLocation());
	return DistSq <= FMath::Square(Selection.AttackRange);
}

UAnimMontage* AWukongBoss::GetPlannedComboMontage() const
{
	const TArray<TObjectPtr<UAnimMontage>>* ComboMontages = nullptr;

	switch (PlannedComboSet)
	{
	case EWukongComboSet::ComboA:
		ComboMontages = &ComboSetAMontages;
		break;
	case EWukongComboSet::ComboB:
		ComboMontages = &ComboSetBMontages;
		break;
	default:
		return nullptr;
	}

	if (!ComboMontages || PlannedComboLength <= 0)
	{
		return nullptr;
	}

	const int32 MontageIndex = PlannedComboLength - 1;
	return ComboMontages->IsValidIndex(MontageIndex)
		? (*ComboMontages)[MontageIndex].Get()
		: nullptr;
}

float AWukongBoss::GetPlannedComboFacingDuration() const
{
	const TArray<float>* FacingDurations = nullptr;

	switch (PlannedComboSet)
	{
	case EWukongComboSet::ComboA:
		FacingDurations = &ComboSetAFacingDurations;
		break;
	case EWukongComboSet::ComboB:
		FacingDurations = &ComboSetBFacingDurations;
		break;
	default:
		return -1.f;
	}

	if (!FacingDurations || PlannedComboLength <= 0)
	{
		return -1.f;
	}

	const int32 DurationIndex = PlannedComboLength - 1;
	return FacingDurations->IsValidIndex(DurationIndex)
		? (*FacingDurations)[DurationIndex]
		: -1.f;
}

UAnimMontage* AWukongBoss::GetPlannedMovementMontage() const
{
	switch (PlannedMovementType)
	{
	case EWukongPlannedMovementType::Dash:
		switch (PlannedMovementDirection)
		{
		case EWukongMovementDirection::Forward:
			return DashForwardMontage.Get();
		case EWukongMovementDirection::Backward:
			return DashBackwardMontage.Get();
		case EWukongMovementDirection::Left:
			return DashLeftMontage.Get();
		case EWukongMovementDirection::Right:
		default:
			return DashRightMontage.Get();
		}
	case EWukongPlannedMovementType::Dodge:
		switch (PlannedMovementDirection)
		{
		case EWukongMovementDirection::Forward:
			return DodgeForwardMontage.Get();
		case EWukongMovementDirection::Backward:
			return DodgeBackwardMontage.Get();
		case EWukongMovementDirection::Left:
			return DodgeLeftMontage.Get();
		case EWukongMovementDirection::Right:
		default:
			return DodgeRightMontage.Get();
		}
	case EWukongPlannedMovementType::Strafe:
	case EWukongPlannedMovementType::None:
	default:
		return nullptr;
	}
}

void AWukongBoss::RecordAction(EWukongActionType ActionType, EWukongComboSet ComboSet, int32 ComboLength)
{
	FWukongActionRecord Record;
	Record.ActionType = ActionType;
	Record.ComboSet = ComboSet;
	Record.ComboLength = ComboLength;

	RecentActionHistory.Add(Record);

	if (RecentActionHistory.Num() > MaxRecentActionHistory)
	{
		RecentActionHistory.RemoveAt(0, RecentActionHistory.Num() - MaxRecentActionHistory);
	}
}

bool AWukongBoss::WasRecentlyUsed(EWukongActionType ActionType, int32 Depth) const
{
	const int32 NumToCheck = FMath::Min(Depth, RecentActionHistory.Num());
	for (int32 Index = RecentActionHistory.Num() - 1; Index >= RecentActionHistory.Num() - NumToCheck; --Index)
	{
		if (RecentActionHistory[Index].ActionType == ActionType)
		{
			return true;
		}
	}

	return false;
}

bool AWukongBoss::WasRecentlyUsed(EWukongComboSet ComboSet, int32 ComboLength, int32 Depth) const
{
	const int32 NumToCheck = FMath::Min(Depth, RecentActionHistory.Num());
	for (int32 Index = RecentActionHistory.Num() - 1; Index >= RecentActionHistory.Num() - NumToCheck; --Index)
	{
		const FWukongActionRecord& Record = RecentActionHistory[Index];
		if (Record.ComboSet == ComboSet && Record.ComboLength == ComboLength)
		{
			return true;
		}
	}

	return false;
}

float AWukongBoss::GetTargetYawAbsDelta() const
{
	return FMath::Abs(GetCombatTargetYawDelta());
}

float AWukongBoss::GetTargetDistance2D() const
{
	if (!CurrentTarget)
	{
		return 0.f;
	}

	return FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());
}

float AWukongBoss::GetPlannedAttackMinRange() const
{
	switch (PlannedAttackType)
	{
	case EWukongPlannedAttackType::ChargeAttack:
		return ChargeAttackMinRange;
	case EWukongPlannedAttackType::JumpAttack:
		return JumpAttackMinRange;
	case EWukongPlannedAttackType::ComboAttack:
	case EWukongPlannedAttackType::None:
	default:
		return 0.f;
	}
}

float AWukongBoss::GetPlannedAttackMaxRange() const
{
	switch (PlannedAttackType)
	{
	case EWukongPlannedAttackType::ChargeAttack:
		return ChargeAttackRange;
	case EWukongPlannedAttackType::JumpAttack:
		return JumpAttackMaxRange;
	case EWukongPlannedAttackType::ComboAttack:
		return BasicAttackRange;
	case EWukongPlannedAttackType::None:
	default:
		return 0.f;
	}
}

bool AWukongBoss::IsTargetTooFarForPlannedAttack() const
{
	if (!CurrentTarget || !HasValidPlannedAttack())
	{
		return false;
	}

	return GetTargetDistance2D() > GetPlannedAttackMaxRange();
}

bool AWukongBoss::IsTargetTooCloseForPlannedAttack() const
{
	if (!CurrentTarget || !HasValidPlannedAttack())
	{
		return false;
	}

	return GetTargetDistance2D() < GetPlannedAttackMinRange();
}

void AWukongBoss::UpdateFacingTowardsTargetWithoutTurn(float DeltaTime, float InterpSpeed)
{
	if (!CurrentTarget)
	{
		return;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = ToTarget.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
	SetActorRotation(NewRotation);
}

FVector AWukongBoss::GetLockOnTargetPoint() const
{
	const FVector DefaultTargetPoint = Super::GetLockOnTargetPoint();

	if (CurrentAttackActionType != EWukongActionType::JumpAttack)
	{
		return DefaultTargetPoint;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return DefaultTargetPoint;
	}

	static const FName LockOnBoneName(TEXT("spine_03"));
	if (MeshComp->GetBoneIndex(LockOnBoneName) == INDEX_NONE)
	{
		return DefaultTargetPoint;
	}

	FVector LockOnPoint = DefaultTargetPoint;
	LockOnPoint.Z = MeshComp->GetBoneLocation(LockOnBoneName).Z;
	return LockOnPoint;
}

void AWukongBoss::ResetPlannedCombatPlan()
{
	PlannedActionType = EWukongPlannedActionType::None;
	PlannedAttackType = EWukongPlannedAttackType::None;
	PlannedMovementType = EWukongPlannedMovementType::None;
	PlannedMovementDirection = EWukongMovementDirection::None;
	PlannedMovementDuration = 0.f;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;
	DeferredAttackTypeAfterApproachDodge = EWukongPlannedAttackType::None;
	DeferredComboSetAfterApproachDodge = EWukongComboSet::None;
	DeferredComboLengthAfterApproachDodge = 0;
}

void AWukongBoss::EnsureCombatPlan()
{
	if (PlannedActionType == EWukongPlannedActionType::None)
	{
		RefreshPlannedCombatPlan();
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack && !HasValidPlannedAttack())
	{
		RefreshPlannedCombatPlan();
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Movement && !HasValidPlannedMovement())
	{
		RefreshPlannedCombatPlan();
	}
}

void AWukongBoss::OnAttackTick(float DeltaTime)
{
	Super::OnAttackTick(DeltaTime);

	if (bPendingJumpAttackLaunch)
	{
		JumpAttackLaunchDelayRemainTime = FMath::Max(0.f, JumpAttackLaunchDelayRemainTime - DeltaTime);
		if (JumpAttackLaunchDelayRemainTime <= 0.f)
		{
			bPendingJumpAttackLaunch = false;
			ExecuteJumpAttackLaunch();
		}
	}

	if (bJumpAttackGroundMotionOverrideActive)
	{
		JumpAttackGroundMotionOverrideRemainTime = FMath::Max(0.f, JumpAttackGroundMotionOverrideRemainTime - DeltaTime);

		if (!GetCharacterMovement()->IsMovingOnGround() || JumpAttackGroundMotionOverrideRemainTime <= 0.f)
		{
			RestoreJumpAttackGroundMotionOverride();
		}
	}

	if (CurrentAttackActionType == EWukongActionType::JumpAttack &&
		CurrentTarget &&
		GetCharacterMovement() &&
		GetCharacterMovement()->IsFalling())
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;

		if (!ToTarget.IsNearlyZero())
		{
			UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
			FVector Velocity = MovementComponent->Velocity;
			const float HorizontalSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
			const FVector TargetHorizontalVelocity = ToTarget.GetSafeNormal() * HorizontalSpeed;
			const FVector CurrentHorizontalVelocity(Velocity.X, Velocity.Y, 0.f);
			const FVector NewHorizontalVelocity = FMath::VInterpTo(
				CurrentHorizontalVelocity,
				TargetHorizontalVelocity,
				DeltaTime,
				JumpAttackAirTrackInterpSpeed
			);

			Velocity.X = NewHorizontalVelocity.X;
			Velocity.Y = NewHorizontalVelocity.Y;
			MovementComponent->Velocity = Velocity;
		}
	}
}

void AWukongBoss::ExecuteJumpAttackLaunch()
{
	FVector LaunchVelocity = FVector::ZeroVector;
	if (CurrentTarget && JumpAttackLaunchForwardSpeed > 0.f)
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;

		if (!ToTarget.IsNearlyZero())
		{
			const FVector LaunchDirection = ToTarget.GetSafeNormal();
			const float DistanceToTarget = ToTarget.Size();
			const float DesiredTravelDistance = FMath::Max(0.f, DistanceToTarget - JumpAttackTargetStandOffDistance);
			const float ForwardSpeed = DesiredTravelDistance > 0.f
				? JumpAttackLaunchForwardSpeed
				: 0.f;

			LaunchVelocity.X = LaunchDirection.X * ForwardSpeed;
			LaunchVelocity.Y = LaunchDirection.Y * ForwardSpeed;
		}
	}

	ApplyJumpAttackGroundMotionOverride();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		FVector NewVelocity = MovementComponent->Velocity;
		NewVelocity.X = LaunchVelocity.X;
		NewVelocity.Y = LaunchVelocity.Y;
		MovementComponent->Velocity = NewVelocity;
	}
}

void AWukongBoss::ApplyJumpAttackGroundMotionOverride()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || bJumpAttackGroundMotionOverrideActive)
	{
		return;
	}

	DefaultGroundFriction = MovementComponent->GroundFriction;
	DefaultBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
	DefaultBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;

	MovementComponent->GroundFriction = 0.f;
	MovementComponent->BrakingDecelerationWalking = 0.f;
	MovementComponent->BrakingFrictionFactor = 0.f;
	bJumpAttackGroundMotionOverrideActive = true;
	JumpAttackGroundMotionOverrideRemainTime = JumpAttackGroundMotionOverrideDuration;
}

void AWukongBoss::RestoreJumpAttackGroundMotionOverride()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !bJumpAttackGroundMotionOverrideActive)
	{
		return;
	}

	MovementComponent->GroundFriction = DefaultGroundFriction;
	MovementComponent->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
	MovementComponent->BrakingFrictionFactor = DefaultBrakingFrictionFactor;
	bJumpAttackGroundMotionOverrideActive = false;
	JumpAttackGroundMotionOverrideRemainTime = 0.f;
}

void AWukongBoss::RefreshPlannedAttackType()
{
	if (!CurrentTarget)
	{
		PlannedAttackType = EWukongPlannedAttackType::None;
		PlannedComboSet = EWukongComboSet::None;
		PlannedComboLength = 0;
		return;
	}

	TArray<EWukongPlannedAttackType> AttackCandidates;

	const bool bHasComboA = ComboSetAMontages.Num() > 0;
	const bool bHasComboB = ComboSetBMontages.Num() > 0;
	const bool bCanUseCombo = bHasComboA || bHasComboB;
	if (bCanUseCombo && !WasRecentlyUsed(EWukongActionType::ComboAttack, 1))
	{
		AttackCandidates.Add(EWukongPlannedAttackType::ComboAttack);
	}

	if (CanUseChargeAttack() && !WasRecentlyUsed(EWukongActionType::ChargeAttack, 1))
	{
		AttackCandidates.Add(EWukongPlannedAttackType::ChargeAttack);
	}

	if (CanUseJumpAttack() && !WasRecentlyUsed(EWukongActionType::JumpAttack, 1))
	{
		AttackCandidates.Add(EWukongPlannedAttackType::JumpAttack);
	}

	if (AttackCandidates.Num() == 0)
	{
		if (bCanUseCombo)
		{
			AttackCandidates.Add(EWukongPlannedAttackType::ComboAttack);
		}

		if (CanUseChargeAttack())
		{
			AttackCandidates.Add(EWukongPlannedAttackType::ChargeAttack);
		}

		if (CanUseJumpAttack())
		{
			AttackCandidates.Add(EWukongPlannedAttackType::JumpAttack);
		}
	}

	if (AttackCandidates.Num() <= 0)
	{
		ResetPlannedCombatPlan();
		return;
	}

	PlannedAttackType = AttackCandidates[FMath::RandRange(0, AttackCandidates.Num() - 1)];
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;

	if (PlannedAttackType != EWukongPlannedAttackType::ComboAttack)
	{
		return;
	}

	TArray<EWukongComboSet> ComboSetCandidates;
	if (bHasComboA)
	{
		ComboSetCandidates.Add(EWukongComboSet::ComboA);
	}
	if (bHasComboB)
	{
		ComboSetCandidates.Add(EWukongComboSet::ComboB);
	}

	if (ComboSetCandidates.Num() <= 0)
	{
		PlannedAttackType = EWukongPlannedAttackType::None;
		return;
	}

	PlannedComboSet = ComboSetCandidates[FMath::RandRange(0, ComboSetCandidates.Num() - 1)];

	const TArray<TObjectPtr<UAnimMontage>>& SelectedComboMontages =
		PlannedComboSet == EWukongComboSet::ComboA ? ComboSetAMontages : ComboSetBMontages;

	TArray<int32> ComboLengthCandidates;
	for (int32 ComboLength = 1; ComboLength <= SelectedComboMontages.Num(); ++ComboLength)
	{
		if (!WasRecentlyUsed(PlannedComboSet, ComboLength, 1))
		{
			ComboLengthCandidates.Add(ComboLength);
		}
	}

	if (ComboLengthCandidates.Num() == 0)
	{
		for (int32 ComboLength = 1; ComboLength <= SelectedComboMontages.Num(); ++ComboLength)
		{
			ComboLengthCandidates.Add(ComboLength);
		}
	}

	PlannedComboLength = ComboLengthCandidates[FMath::RandRange(0, ComboLengthCandidates.Num() - 1)];
}

void AWukongBoss::RefreshPlannedCombatPlan()
{
	ResetPlannedCombatPlan();

	if (FMath::FRand() <= AttackPlanWeight)
	{
		PlannedActionType = EWukongPlannedActionType::Attack;
		RefreshPlannedAttackType();
		return;
	}

	PlannedActionType = EWukongPlannedActionType::Movement;
	RefreshPlannedMovementType();
}

void AWukongBoss::RefreshPlannedMovementType()
{
	PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;
	PlannedMovementType = EWukongPlannedMovementType::Strafe;
	PlannedMovementDuration = StrafeMinDuration;
}
