#include "Character/WukongBoss.h"

#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	EWukongPlannedNonAttackType GetNextExpressiveNonAttackTypeInSequence(EWukongPlannedNonAttackType CurrentType)
	{
		switch (CurrentType)
		{
		case EWukongPlannedNonAttackType::ComeHere:
			return EWukongPlannedNonAttackType::Sleep;
		case EWukongPlannedNonAttackType::Sleep:
			return EWukongPlannedNonAttackType::Boring;
		case EWukongPlannedNonAttackType::Boring:
		default:
			return EWukongPlannedNonAttackType::ComeHere;
		}
	}

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
		case EWukongCombatState::NonAttackAction:
			return TEXT("NonAttackAction");
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
		case EWukongPlannedAttackType::DodgeAttack:
			return TEXT("DodgeAttack");
		case EWukongPlannedAttackType::ExtraAttack:
			return TEXT("ExtraAttack");
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
		case EWukongPlannedActionType::NonAttack:
			return TEXT("NonAttack");
		case EWukongPlannedActionType::None:
		default:
			return TEXT("None");
		}
	}

	const TCHAR* GetWukongPlannedNonAttackDebugText(EWukongPlannedNonAttackType NonAttackType)
	{
		switch (NonAttackType)
		{
		case EWukongPlannedNonAttackType::Strafe:
			return TEXT("Strafe");
		case EWukongPlannedNonAttackType::Dash:
			return TEXT("Dash");
		case EWukongPlannedNonAttackType::Dodge:
			return TEXT("Dodge");
		case EWukongPlannedNonAttackType::ComeHere:
			return TEXT("ComeHere");
		case EWukongPlannedNonAttackType::Sleep:
			return TEXT("Sleep");
		case EWukongPlannedNonAttackType::Boring:
			return TEXT("Boring");
		case EWukongPlannedNonAttackType::Hold:
			return TEXT("Hold");
		case EWukongPlannedNonAttackType::None:
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

	const TCHAR* GetWukongExtraAttackDebugText(EWukongExtraAttackType AttackType)
	{
		switch (AttackType)
		{
		case EWukongExtraAttackType::Ambush:
			return TEXT("Ambush");
		case EWukongExtraAttackType::Heavy:
			return TEXT("Heavy");
		case EWukongExtraAttackType::Spin:
			return TEXT("Spin");
		case EWukongExtraAttackType::NinjaA:
			return TEXT("NinjaA");
		case EWukongExtraAttackType::NinjaB:
			return TEXT("NinjaB");
		case EWukongExtraAttackType::Execution:
			return TEXT("Execution");
		case EWukongExtraAttackType::None:
		default:
			return TEXT("None");
		}
	}
}

AWukongBoss::AWukongBoss()
{
	WalkMoveSpeed = 200.f;
	RunMoveSpeed = 700.f;
	AmbushAttack.FacingDuration = 0.5f;
	ExecutionAttack.FacingDuration = 1.5f;
	HeavyAttack.FacingDuration = 1.5f;
	SpinAttack.bFaceTargetDuringAttack = false;
	SpinAttack.FacingDuration = 0.f;
	SpinAttack.MaxRange = 200.f;
	NinjaAAttack.FacingDuration = 1.7f;
	NinjaBAttack.FacingDuration = 1.6f;
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
			TEXT("Wukong SubState: %s\nPlannedAction: %s\nPlannedAttack: %s\nExtraAttack: %s\nPlannedNonAttack: %s\nCombo: %s / %d\nSelectedMontage: %s\nCanAttack: %s"),
			GetWukongCombatStateDebugText(CurrentCombatSubState),
			GetWukongPlannedActionDebugText(PlannedActionType),
			GetWukongPlannedAttackDebugText(PlannedAttackType),
			GetWukongExtraAttackDebugText(PlannedExtraAttackType),
			GetWukongPlannedNonAttackDebugText(PlannedNonAttackType),
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
	case EWukongCombatState::NonAttackAction:
		UpdateNonAttackActionState(DeltaTime);
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

	if (PlannedAttackType == EWukongPlannedAttackType::ChargeAttack)
	{
		if (ChargeAttackMontage)
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
	}

	if (PlannedAttackType == EWukongPlannedAttackType::DodgeAttack && DodgeAttackMontage)
	{
		Selection.Montage = DodgeAttackMontage;
		Selection.AttackRange = DodgeAttackRange;
		Selection.bFaceTargetDuringAttack = true;
		Selection.FacingDuration = DodgeAttackFacingDuration;
		Selection.FacingInterpSpeed = DodgeAttackFacingInterpSpeed;
		Selection.bTryTurnAfterAttack = true;
		Selection.PostAttackTurnStartAngle = DodgeAttackTurnReacquireAngle;
		return Selection;
	}

	if (PlannedAttackType == EWukongPlannedAttackType::ExtraAttack)
	{
		if (const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(PlannedExtraAttackType))
		{
			if (ExtraAttackData->Montage)
			{
				Selection.Montage = ExtraAttackData->Montage;
				Selection.AttackRange = ExtraAttackData->MaxRange;
				Selection.bFaceTargetDuringAttack = ExtraAttackData->bFaceTargetDuringAttack;
				Selection.FacingDuration = ExtraAttackData->FacingDuration;
				Selection.FacingInterpSpeed = ExtraAttackData->FacingInterpSpeed;
				Selection.bTryTurnAfterAttack = ExtraAttackData->bTryTurnAfterAttack;
				Selection.PostAttackTurnStartAngle = ExtraAttackData->PostAttackTurnStartAngle;
				return Selection;
			}
		}
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
	case EWukongCombatState::NonAttackAction:
		EnterNonAttackActionState();
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
	bApproachComeHereTriggered = false;
	bApproachComeHereInProgress = false;
	ApproachComeHereRemainTime = 0.f;
	bDiscardPlannedAttackAfterApproachComeHere = false;
}

void AWukongBoss::EnterIdleState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bRunLocomotionRequested = false;
	RefreshPlannedCombatPlan();
	bUseIdleHoldForYawMismatch = false;

	if (PlannedActionType == EWukongPlannedActionType::Attack &&
		HasValidPlannedAttack() &&
		!IsTargetTooFarForPlannedAttack() &&
		!IsTargetTooCloseForPlannedAttack())
	{
		const float TargetYawAbsDelta = GetTargetYawAbsDelta();
		if (TargetYawAbsDelta > IdleEntryYawTolerance && TargetYawAbsDelta < MoveTurnStartAngle)
		{
			bUseIdleHoldForYawMismatch = FMath::FRand() > YawMismatchRepositionChance;
		}
	}
}

void AWukongBoss::EnterAttackState()
{
	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();
	bPendingJumpAttackLaunch = false;
	JumpAttackLaunchDelayRemainTime = 0.f;
	CurrentAttackActionType = EWukongActionType::None;
	CurrentAttackComboSet = EWukongComboSet::None;
	CurrentAttackComboLength = 0;
	CurrentAttackExtraAttackType = EWukongExtraAttackType::None;

	if (AttackSelection.Montage == JumpAttackMontage)
	{
		CurrentAttackActionType = EWukongActionType::JumpAttack;
	}
	else if (AttackSelection.Montage == ChargeAttackMontage)
	{
		CurrentAttackActionType = EWukongActionType::ChargeAttack;
	}
	else if (AttackSelection.Montage == DodgeAttackMontage)
	{
		CurrentAttackActionType = EWukongActionType::DodgeAttack;
	}
	else if (PlannedAttackType == EWukongPlannedAttackType::ExtraAttack && AttackSelection.Montage != nullptr)
	{
		CurrentAttackActionType = EWukongActionType::ExtraAttack;
		CurrentAttackExtraAttackType = PlannedExtraAttackType;
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
	bPendingComboDashFollowUp = false;
	PendingComboDashFollowUpDuration = 0.f;
	PendingComboDashFollowUpMinComboLengthExclusive = 0;

	if (CurrentAttackActionType != EWukongActionType::None)
	{
		RecordAction(CurrentAttackActionType, CurrentAttackComboSet, CurrentAttackComboLength, CurrentAttackExtraAttackType);
	}

	const bool bCanStartComboDashFollowUp =
		CurrentAttackActionType == EWukongActionType::ComboAttack &&
		!bCurrentAttackStartedFromComboDashFollowUp &&
		(CurrentAttackComboLength == 1 || CurrentAttackComboLength == 2);

	if (bCanStartComboDashFollowUp)
	{
		TArray<TPair<UAnimMontage*, EWukongMovementDirection>> DashCandidates;
		if (DashLeftMontage)
		{
			DashCandidates.Emplace(DashLeftMontage.Get(), EWukongMovementDirection::Left);
		}
		if (DashRightMontage)
		{
			DashCandidates.Emplace(DashRightMontage.Get(), EWukongMovementDirection::Right);
		}

		if (DashCandidates.Num() > 0)
		{
			const TPair<UAnimMontage*, EWukongMovementDirection>& SelectedDash =
				DashCandidates[FMath::RandRange(0, DashCandidates.Num() - 1)];
			if (CurrentTarget)
			{
				FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
				ToTarget.Z = 0.f;
				if (!ToTarget.IsNearlyZero())
				{
					SetActorRotation(ToTarget.Rotation());
				}
			}
			PlayAnimMontage(SelectedDash.Key);
			PlannedNonAttackType = EWukongPlannedNonAttackType::Dash;
			PlannedMovementDirection = SelectedDash.Value;
			PendingComboDashFollowUpDuration = SelectedDash.Key
				? SelectedDash.Key->GetPlayLength()
				: ComboDashFollowUpFallbackDuration;
			PendingComboDashFollowUpMinComboLengthExclusive = CurrentAttackComboLength;
			bPendingComboDashFollowUp = true;
		}
	}

	ResetPlannedCombatPlan();

	CurrentAttackActionType = EWukongActionType::None;
	CurrentAttackComboSet = EWukongComboSet::None;
	CurrentAttackComboLength = 0;
	CurrentAttackExtraAttackType = EWukongExtraAttackType::None;
	bCurrentAttackStartedFromComboDashFollowUp = false;
}

void AWukongBoss::EnterRepositionState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	bRunLocomotionRequested = false;

	if (PlannedActionType == EWukongPlannedActionType::NonAttack &&
		PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe &&
		PlannedMovementDirection == EWukongMovementDirection::None)
	{
		PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;
	}

	if (PlannedActionType == EWukongPlannedActionType::NonAttack &&
		PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe &&
		PlannedMovementDuration <= 0.f)
	{
		PlannedMovementDuration = StrafeMinDuration;
	}

	if (PlannedActionType == EWukongPlannedActionType::NonAttack &&
		PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe)
	{
		CurrentRepositionDirection = PlannedMovementDirection == EWukongMovementDirection::Left ? -1.f : 1.f;
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack &&
		CurrentTarget &&
		!IsTargetTooCloseForPlannedAttack())
	{
		const FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		const float SideSign = FVector::DotProduct(GetActorRightVector(), ToTarget);
		CurrentRepositionDirection = SideSign >= 0.f ? 1.f : -1.f;
		return;
	}

	CurrentRepositionDirection = 1.f;
}

void AWukongBoss::EnterNonAttackActionState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bRunLocomotionRequested = false;

	if (UAnimMontage* NonAttackMontage = GetPlannedNonAttackMontage())
	{
		PlayAnimMontage(NonAttackMontage);
		PlannedMovementDuration = NonAttackMontage->GetPlayLength();
	}
	else if (PlannedMovementDuration <= 0.f)
	{
		PlannedMovementDuration = NoAttackHoldDuration;
	}
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

	if (UAnimMontage* NonAttackMontage = GetPlannedNonAttackMontage())
	{
		PlayAnimMontage(NonAttackMontage);
		PlannedMovementDuration = NonAttackMontage->GetPlayLength();
	}
	else if (PlannedMovementDuration <= 0.f)
	{
		PlannedMovementDuration = EvadeFallbackDuration;
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

	if (bApproachComeHereInProgress)
	{
		ApproachComeHereRemainTime = FMath::Max(0.f, ApproachComeHereRemainTime - DeltaTime);
		StopAIMovement();
		GetCharacterMovement()->StopMovementImmediately();
		bRunLocomotionRequested = false;
		SetDesiredMoveAxes(0.f, 0.f);

		if (ApproachComeHereRemainTime <= 0.f)
		{
			bApproachComeHereInProgress = false;
			if (bDiscardPlannedAttackAfterApproachComeHere)
			{
				bDiscardPlannedAttackAfterApproachComeHere = false;
				ResetPlannedCombatPlan();
				SetWukongCombatState(EWukongCombatState::Idle);
				return;
			}
		}

		return;
	}

	if (!bApproachComeHereTriggered &&
		ComeHereMontage &&
		CombatSubStateElapsedTime >= ApproachComeHereDelay)
	{
		bApproachComeHereTriggered = true;
		bApproachComeHereInProgress = true;
		bDiscardPlannedAttackAfterApproachComeHere = true;
		ApproachComeHereRemainTime = ComeHereMontage->GetPlayLength();
		StopAIMovement();
		GetCharacterMovement()->StopMovementImmediately();
		bRunLocomotionRequested = false;
		SetDesiredMoveAxes(0.f, 0.f);
		PlayAnimMontage(ComeHereMontage);
		return;
	}

	UpdateFacingTowardsTargetWithoutTurn(DeltaTime, ApproachFacingInterpSpeed);

	if (IsTargetTooFarForPlannedAttack())
	{
		bRunLocomotionRequested = false;
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

	if (PlannedActionType == EWukongPlannedActionType::NonAttack)
	{
		if (!HasValidPlannedMovement())
		{
			ResetPlannedCombatPlan();
			return;
		}

		if (GetTargetYawAbsDelta() >= MoveTurnStartAngle && CanStartWukongTurn())
		{
			SetWukongCombatState(EWukongCombatState::Turn);
			return;
		}

		if (PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe)
		{
			SetWukongCombatState(EWukongCombatState::Reposition);
			return;
		}

		SetWukongCombatState(
			(PlannedNonAttackType == EWukongPlannedNonAttackType::Dash || PlannedNonAttackType == EWukongPlannedNonAttackType::Dodge)
				? EWukongCombatState::Evade
				: EWukongCombatState::NonAttackAction
		);
		return;
	}

	if (bUseIdleHoldForYawMismatch && CombatSubStateElapsedTime < YawMismatchIdleHoldDuration)
	{
		SetDesiredMoveAxes(0.f, 0.f);
		return;
	}

	if (GetTargetYawAbsDelta() > IdleEntryYawTolerance)
	{
		if (CanStartWukongTurn())
		{
			SetWukongCombatState(EWukongCombatState::Turn);
			return;
		}

		if (PlannedActionType == EWukongPlannedActionType::Attack &&
			HasValidPlannedAttack() &&
			!IsTargetTooFarForPlannedAttack() &&
			!IsTargetTooCloseForPlannedAttack())
		{
			SetWukongCombatState(EWukongCombatState::Reposition);
			return;
		}

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
		return;
	}
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
	if (bPendingComboDashFollowUp)
	{
		if (CurrentTarget)
		{
			UpdateFacingTowardsTargetWithoutTurn(DeltaTime, EvadeFacingInterpSpeed);
		}

		if (CombatSubStateElapsedTime < PendingComboDashFollowUpDuration)
		{
			return;
		}

		bPendingComboDashFollowUp = false;
		PendingComboDashFollowUpDuration = 0.f;
		RecordAction(EWukongActionType::Dash);
		ResetPlannedCombatPlan();

		if (TryPlanComboAttackOnly(PendingComboDashFollowUpMinComboLengthExclusive))
		{
			if (!IsTargetTooFarForPlannedAttack() && !IsTargetTooCloseForPlannedAttack())
			{
				bCurrentAttackStartedFromComboDashFollowUp = true;
				SetWukongCombatState(EWukongCombatState::Attack);
				return;
			}

			SetWukongCombatState(EWukongCombatState::Approach);
			return;
		}

		SetWukongCombatState(EWukongCombatState::Idle);
		return;
	}

	if (CombatSubStateElapsedTime < RecoveryDuration)
	{
		return;
	}

	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateRepositionState(float DeltaTime)
{
	EnsureCombatPlan();
	const float ResolvedStrafeAnimInputStrength = FMath::Clamp(StrafeAnimInputStrength, 0.f, 0.3f);
	const float ResolvedStrafeMoveInputStrength = StrafeMoveInputStrength * StrafeMoveSpeedScale;

	if (GetTargetYawAbsDelta() >= MoveTurnStartAngle && CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack && IsTargetTooFarForPlannedAttack())
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::NonAttack &&
		PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe)
	{
		UpdateFacingTowardsTargetWithoutTurn(DeltaTime, CombatFacingInterpSpeed);
		const FVector RightDirection = GetActorRightVector();
		AddMovementInput(RightDirection, CurrentRepositionDirection * ResolvedStrafeMoveInputStrength);
		SetDesiredMoveAxes(0.f, CurrentRepositionDirection * ResolvedStrafeAnimInputStrength);

		if (CombatSubStateElapsedTime >= PlannedMovementDuration)
		{
			CombatMoveInput = FVector2D::ZeroVector;
			SetDesiredMoveAxes(0.f, 0.f);
			LastCompletedNonAttackType = EWukongPlannedNonAttackType::Strafe;
			ResetPlannedCombatPlan();
			SetWukongCombatState(EWukongCombatState::Idle);
		}

		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack && IsTargetTooCloseForPlannedAttack())
	{
		ResetPlannedCombatPlan();
		SetWukongCombatState(EWukongCombatState::Idle);
		return;
	}

	if (PlannedActionType == EWukongPlannedActionType::Attack)
	{
		const float TargetYawDelta = GetTargetYawAbsDelta();
		if (TargetYawDelta > IdleEntryYawTolerance)
		{
			UpdateFacingTowardsTargetWithoutTurn(DeltaTime, CombatFacingInterpSpeed);
			const FVector RightDirection = GetActorRightVector();
			AddMovementInput(RightDirection, CurrentRepositionDirection * ResolvedStrafeMoveInputStrength);
			SetDesiredMoveAxes(0.f, CurrentRepositionDirection * ResolvedStrafeAnimInputStrength);
			return;
		}

		if (CanAttackTarget())
		{
			SetWukongCombatState(EWukongCombatState::Attack);
			return;
		}

		SetWukongCombatState(EWukongCombatState::Idle);
		return;
	}

	SetDesiredMoveAxes(0.f, 0.f);
	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateNonAttackActionState(float DeltaTime)
{
	StopAIMovement();
	GetCharacterMovement()->StopMovementImmediately();
	SetDesiredMoveAxes(0.f, 0.f);
	bRunLocomotionRequested = false;

	if (CombatSubStateElapsedTime < PlannedMovementDuration)
	{
		return;
	}

	LastCompletedNonAttackType = PlannedNonAttackType;
	if (PlannedNonAttackType == EWukongPlannedNonAttackType::ComeHere ||
		PlannedNonAttackType == EWukongPlannedNonAttackType::Sleep ||
		PlannedNonAttackType == EWukongPlannedNonAttackType::Boring)
	{
		NextExpressiveNonAttackType = GetNextExpressiveNonAttackTypeInSequence(PlannedNonAttackType);
	}
	ResetPlannedCombatPlan();
	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateEvadeState(float DeltaTime)
{
	if (CurrentTarget)
	{
		UpdateFacingTowardsTargetWithoutTurn(DeltaTime, EvadeFacingInterpSpeed);
	}

	if (CombatSubStateElapsedTime < PlannedMovementDuration)
	{
		return;
	}

	switch (PlannedNonAttackType)
	{
	case EWukongPlannedNonAttackType::Dash:
		RecordAction(EWukongActionType::Dash);
		break;
	case EWukongPlannedNonAttackType::Dodge:
		RecordAction(EWukongActionType::Dodge);
		break;
	default:
		break;
	}

	ResetPlannedCombatPlan();

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

bool AWukongBoss::HasAnyAttackCandidateForCurrentDistance() const
{
	if (!CurrentTarget)
	{
		return false;
	}

	const float TargetDistance = GetTargetDistance2D();
	auto IsWithinRange = [TargetDistance](float MinRange, float MaxRange)
	{
		return TargetDistance >= MinRange && TargetDistance <= MaxRange;
	};

	const bool bCanUseCombo = ComboSetAMontages.Num() > 0 || ComboSetBMontages.Num() > 0;
	if (bCanUseCombo && IsWithinRange(0.f, BasicAttackRange))
	{
		return true;
	}

	if (CanUseChargeAttack() && IsWithinRange(ChargeAttackMinRange, ChargeAttackRange))
	{
		return true;
	}

	if (CanUseJumpAttack() && IsWithinRange(JumpAttackMinRange, JumpAttackMaxRange))
	{
		return true;
	}

	if (DodgeAttackMontage && IsWithinRange(DodgeAttackMinRange, DodgeAttackRange))
	{
		return true;
	}

	for (const EWukongExtraAttackType ExtraAttackType : {
		EWukongExtraAttackType::Ambush,
		EWukongExtraAttackType::Heavy,
		EWukongExtraAttackType::Spin,
		EWukongExtraAttackType::NinjaA,
		EWukongExtraAttackType::NinjaB,
		EWukongExtraAttackType::Execution })
	{
		const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(ExtraAttackType);
		if (ExtraAttackData &&
			ExtraAttackData->Montage &&
			IsWithinRange(ExtraAttackData->MinRange, ExtraAttackData->MaxRange))
		{
			return true;
		}
	}

	return false;
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
	case EWukongPlannedAttackType::DodgeAttack:
		return DodgeAttackMontage != nullptr;
	case EWukongPlannedAttackType::ExtraAttack:
		{
			const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(PlannedExtraAttackType);
			return ExtraAttackData && ExtraAttackData->Montage != nullptr;
		}
	case EWukongPlannedAttackType::None:
	default:
		return false;
	}
}

bool AWukongBoss::HasValidPlannedMovement() const
{
	switch (PlannedNonAttackType)
	{
	case EWukongPlannedNonAttackType::Strafe:
		return true;
	case EWukongPlannedNonAttackType::Dash:
	case EWukongPlannedNonAttackType::Dodge:
		return GetPlannedNonAttackMontage() != nullptr;
	case EWukongPlannedNonAttackType::ComeHere:
	case EWukongPlannedNonAttackType::Sleep:
	case EWukongPlannedNonAttackType::Boring:
		return GetPlannedNonAttackMontage() != nullptr;
	case EWukongPlannedNonAttackType::Hold:
		return PlannedMovementDuration > 0.f;
	case EWukongPlannedNonAttackType::None:
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

const FWukongExtraAttackData* AWukongBoss::GetExtraAttackData(EWukongExtraAttackType AttackType) const
{
	switch (AttackType)
	{
	case EWukongExtraAttackType::Ambush:
		return &AmbushAttack;
	case EWukongExtraAttackType::Heavy:
		return &HeavyAttack;
	case EWukongExtraAttackType::Spin:
		return &SpinAttack;
	case EWukongExtraAttackType::NinjaA:
		return &NinjaAAttack;
	case EWukongExtraAttackType::NinjaB:
		return &NinjaBAttack;
	case EWukongExtraAttackType::Execution:
		return &ExecutionAttack;
	case EWukongExtraAttackType::None:
	default:
		return nullptr;
	}
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

UAnimMontage* AWukongBoss::GetPlannedNonAttackMontage() const
{
	switch (PlannedNonAttackType)
	{
	case EWukongPlannedNonAttackType::ComeHere:
		return ComeHereMontage.Get();
	case EWukongPlannedNonAttackType::Sleep:
		return SleepMontage.Get();
	case EWukongPlannedNonAttackType::Boring:
		return BoringMontage.Get();
	case EWukongPlannedNonAttackType::Dash:
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
	case EWukongPlannedNonAttackType::Dodge:
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
	case EWukongPlannedNonAttackType::Hold:
	case EWukongPlannedNonAttackType::Strafe:
	case EWukongPlannedNonAttackType::None:
	default:
		return nullptr;
	}
}

void AWukongBoss::RecordAction(EWukongActionType ActionType, EWukongComboSet ComboSet, int32 ComboLength, EWukongExtraAttackType ExtraAttackType)
{
	FWukongActionRecord Record;
	Record.ActionType = ActionType;
	Record.ComboSet = ComboSet;
	Record.ComboLength = ComboLength;
	Record.ExtraAttackType = ExtraAttackType;

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

bool AWukongBoss::WasRecentlyUsed(EWukongExtraAttackType AttackType, int32 Depth) const
{
	const int32 NumToCheck = FMath::Min(Depth, RecentActionHistory.Num());
	for (int32 Index = RecentActionHistory.Num() - 1; Index >= RecentActionHistory.Num() - NumToCheck; --Index)
	{
		if (RecentActionHistory[Index].ExtraAttackType == AttackType)
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
	case EWukongPlannedAttackType::DodgeAttack:
		return DodgeAttackMinRange;
	case EWukongPlannedAttackType::ExtraAttack:
		{
			const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(PlannedExtraAttackType);
			return ExtraAttackData ? ExtraAttackData->MinRange : 0.f;
		}
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
	case EWukongPlannedAttackType::DodgeAttack:
		return DodgeAttackRange;
	case EWukongPlannedAttackType::ExtraAttack:
		{
			const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(PlannedExtraAttackType);
			return ExtraAttackData ? ExtraAttackData->MaxRange : 0.f;
		}
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

	if (CurrentCombatSubState == EWukongCombatState::Idle &&
		bUseIdleHoldForYawMismatch &&
		CombatSubStateElapsedTime < YawMismatchIdleHoldDuration)
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

bool AWukongBoss::ShouldStartHitReact(EHitReactType IncomingHitReact) const
{
	if (IncomingHitReact == EHitReactType::LightHit)
	{
		return false;
	}

	return Super::ShouldStartHitReact(IncomingHitReact);
}

void AWukongBoss::ResetPlannedCombatPlan()
{
	PlannedActionType = EWukongPlannedActionType::None;
	PlannedAttackType = EWukongPlannedAttackType::None;
	PlannedExtraAttackType = EWukongExtraAttackType::None;
	PlannedNonAttackType = EWukongPlannedNonAttackType::None;
	PlannedMovementDirection = EWukongMovementDirection::None;
	PlannedMovementDuration = 0.f;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;
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

	if (PlannedActionType == EWukongPlannedActionType::NonAttack && !HasValidPlannedMovement())
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
		PlannedExtraAttackType = EWukongExtraAttackType::None;
		PlannedComboSet = EWukongComboSet::None;
		PlannedComboLength = 0;
		return;
	}

	TArray<EWukongPlannedAttackType> AttackCandidates;
	const float TargetDistance = GetTargetDistance2D();
	auto AddWeightedAttackCandidate = [&AttackCandidates](EWukongPlannedAttackType AttackType, int32 Weight)
	{
		const int32 ClampedWeight = FMath::Max(1, Weight);
		for (int32 Index = 0; Index < ClampedWeight; ++Index)
		{
			AttackCandidates.Add(AttackType);
		}
	};
	auto IsWithinRange = [TargetDistance](float MinRange, float MaxRange)
	{
		return TargetDistance >= MinRange && TargetDistance <= MaxRange;
	};

	const bool bHasComboA = ComboSetAMontages.Num() > 0;
	const bool bHasComboB = ComboSetBMontages.Num() > 0;
	const bool bCanUseCombo = bHasComboA || bHasComboB;
	if (bCanUseCombo && IsWithinRange(0.f, BasicAttackRange) && !WasRecentlyUsed(EWukongActionType::ComboAttack, 1))
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::ComboAttack, 1);
	}

	if (CanUseChargeAttack() &&
		IsWithinRange(ChargeAttackMinRange, ChargeAttackRange) &&
		!WasRecentlyUsed(EWukongActionType::ChargeAttack, 1))
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::ChargeAttack, 1);
	}

	if (CanUseJumpAttack() &&
		IsWithinRange(JumpAttackMinRange, JumpAttackMaxRange) &&
		!WasRecentlyUsed(EWukongActionType::JumpAttack, 1))
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::JumpAttack, 1);
	}

	if (DodgeAttackMontage &&
		IsWithinRange(DodgeAttackMinRange, DodgeAttackRange) &&
		!WasRecentlyUsed(EWukongActionType::DodgeAttack, 1))
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::DodgeAttack, DodgeAttackSelectionWeight);
	}

	TArray<EWukongExtraAttackType> ExtraAttackCandidates;
	for (const EWukongExtraAttackType ExtraAttackType : {
		EWukongExtraAttackType::Ambush,
		EWukongExtraAttackType::Heavy,
		EWukongExtraAttackType::Spin,
		EWukongExtraAttackType::NinjaA,
		EWukongExtraAttackType::NinjaB,
		EWukongExtraAttackType::Execution })
	{
		const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(ExtraAttackType);
		if (ExtraAttackData &&
			ExtraAttackData->Montage &&
			IsWithinRange(ExtraAttackData->MinRange, ExtraAttackData->MaxRange) &&
			!WasRecentlyUsed(ExtraAttackType, 1))
		{
			ExtraAttackCandidates.Add(ExtraAttackType);
		}
	}

	if (ExtraAttackCandidates.Num() > 0)
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::ExtraAttack, 1);
	}

	if (AttackCandidates.Num() == 0)
	{
		if (bCanUseCombo && IsWithinRange(0.f, BasicAttackRange))
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::ComboAttack, 1);
		}

		if (CanUseChargeAttack() && IsWithinRange(ChargeAttackMinRange, ChargeAttackRange))
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::ChargeAttack, 1);
		}

		if (CanUseJumpAttack() && IsWithinRange(JumpAttackMinRange, JumpAttackMaxRange))
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::JumpAttack, 1);
		}

		if (DodgeAttackMontage && IsWithinRange(DodgeAttackMinRange, DodgeAttackRange))
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::DodgeAttack, DodgeAttackSelectionWeight);
		}

		for (const EWukongExtraAttackType ExtraAttackType : {
			EWukongExtraAttackType::Ambush,
			EWukongExtraAttackType::Heavy,
			EWukongExtraAttackType::Spin,
			EWukongExtraAttackType::NinjaA,
			EWukongExtraAttackType::NinjaB,
			EWukongExtraAttackType::Execution })
		{
			const FWukongExtraAttackData* ExtraAttackData = GetExtraAttackData(ExtraAttackType);
			if (ExtraAttackData &&
				ExtraAttackData->Montage &&
				IsWithinRange(ExtraAttackData->MinRange, ExtraAttackData->MaxRange))
			{
				ExtraAttackCandidates.AddUnique(ExtraAttackType);
			}
		}

		if (ExtraAttackCandidates.Num() > 0)
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::ExtraAttack, 1);
		}
	}

	if (AttackCandidates.Num() <= 0)
	{
		ResetPlannedCombatPlan();
		return;
	}

	PlannedAttackType = AttackCandidates[FMath::RandRange(0, AttackCandidates.Num() - 1)];
	PlannedExtraAttackType = EWukongExtraAttackType::None;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;

	if (PlannedAttackType == EWukongPlannedAttackType::ExtraAttack)
	{
		if (ExtraAttackCandidates.Num() <= 0)
		{
			PlannedAttackType = EWukongPlannedAttackType::None;
			return;
		}

		PlannedExtraAttackType = ExtraAttackCandidates[FMath::RandRange(0, ExtraAttackCandidates.Num() - 1)];
		return;
	}

	if (PlannedAttackType == EWukongPlannedAttackType::ChargeAttack)
	{
		return;
	}

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

	if (!HasAnyAttackCandidateForCurrentDistance())
	{
		PlannedActionType = EWukongPlannedActionType::NonAttack;
		RefreshNoAttackFallbackMovementType();
		return;
	}

	bShouldStartNoAttackFallbackWithStrafe = true;

	if (FMath::FRand() <= AttackPlanWeight)
	{
		PlannedActionType = EWukongPlannedActionType::Attack;
		RefreshPlannedAttackType();
		if (PlannedAttackType == EWukongPlannedAttackType::None)
		{
			PlannedActionType = EWukongPlannedActionType::NonAttack;
			RefreshNoAttackFallbackMovementType();
		}
		return;
	}

	PlannedActionType = EWukongPlannedActionType::NonAttack;
	RefreshPlannedNonAttackType();
}

void AWukongBoss::RefreshPlannedNonAttackType()
{
	PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;
	const bool bUsedDodgeRecently = WasRecentlyUsed(EWukongActionType::Dodge, 1);

	const bool bCanUseSideDodge =
		(PlannedMovementDirection == EWukongMovementDirection::Left && DodgeLeftMontage) ||
		(PlannedMovementDirection == EWukongMovementDirection::Right && DodgeRightMontage);

	if (!bUsedDodgeRecently && bCanUseSideDodge && FMath::FRand() <= DodgeMovementPlanWeight)
	{
		PlannedNonAttackType = EWukongPlannedNonAttackType::Dodge;
		PlannedMovementDuration = 0.f;
		return;
	}

	PlannedNonAttackType = EWukongPlannedNonAttackType::Strafe;
	PlannedMovementDuration = StrafeMinDuration;
}

void AWukongBoss::RefreshNoAttackFallbackMovementType()
{
	if (bShouldStartNoAttackFallbackWithStrafe)
	{
		bShouldStartNoAttackFallbackWithStrafe = false;
		PlannedNonAttackType = EWukongPlannedNonAttackType::Strafe;
		PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;
		PlannedMovementDuration = StrafeMinDuration;
		return;
	}

	TArray<EWukongPlannedNonAttackType> NonAttackCandidates;
	NonAttackCandidates.Add(EWukongPlannedNonAttackType::Strafe);
	const bool bLastWasExpressiveNonAttack =
		LastCompletedNonAttackType == EWukongPlannedNonAttackType::ComeHere ||
		LastCompletedNonAttackType == EWukongPlannedNonAttackType::Sleep ||
		LastCompletedNonAttackType == EWukongPlannedNonAttackType::Boring;

	if (!bLastWasExpressiveNonAttack)
	{
		auto TryAddExpressiveCandidate = [this, &NonAttackCandidates](EWukongPlannedNonAttackType CandidateType)
		{
			switch (CandidateType)
			{
			case EWukongPlannedNonAttackType::ComeHere:
				if (ComeHereMontage)
				{
					NonAttackCandidates.Add(CandidateType);
					return true;
				}
				break;
			case EWukongPlannedNonAttackType::Sleep:
				if (SleepMontage)
				{
					NonAttackCandidates.Add(CandidateType);
					return true;
				}
				break;
			case EWukongPlannedNonAttackType::Boring:
				if (BoringMontage)
				{
					NonAttackCandidates.Add(CandidateType);
					return true;
				}
				break;
			default:
				break;
			}

			return false;
		};

		EWukongPlannedNonAttackType CandidateType = NextExpressiveNonAttackType;
		for (int32 Attempt = 0; Attempt < 3; ++Attempt)
		{
			if (TryAddExpressiveCandidate(CandidateType))
			{
				break;
			}

			CandidateType = GetNextExpressiveNonAttackTypeInSequence(CandidateType);
		}
	}
	NonAttackCandidates.Add(EWukongPlannedNonAttackType::Hold);

	if (NonAttackCandidates.Num() <= 0)
	{
		PlannedNonAttackType = EWukongPlannedNonAttackType::Hold;
		PlannedMovementDuration = NoAttackHoldDuration;
		return;
	}

	PlannedNonAttackType = NonAttackCandidates[FMath::RandRange(0, NonAttackCandidates.Num() - 1)];
	PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;

	if (PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe)
	{
		PlannedMovementDuration = StrafeMinDuration;
		return;
	}

	if (PlannedNonAttackType == EWukongPlannedNonAttackType::Hold)
	{
		PlannedMovementDuration = NoAttackHoldDuration;
		return;
	}

	if (UAnimMontage* NonAttackMontage = GetPlannedNonAttackMontage())
	{
		PlannedMovementDuration = NonAttackMontage->GetPlayLength();
		return;
	}

	PlannedNonAttackType = EWukongPlannedNonAttackType::Hold;
	PlannedMovementDuration = NoAttackHoldDuration;
}

bool AWukongBoss::TryPlanComboAttackOnly(int32 MinComboLengthExclusive)
{
	const bool bHasComboA = ComboSetAMontages.Num() > 0;
	const bool bHasComboB = ComboSetBMontages.Num() > 0;
	if (!bHasComboA && !bHasComboB)
	{
		return false;
	}

	PlannedActionType = EWukongPlannedActionType::Attack;
	PlannedAttackType = EWukongPlannedAttackType::ComboAttack;
	PlannedExtraAttackType = EWukongExtraAttackType::None;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;

	TArray<EWukongComboSet> ComboSetCandidates;
	if (bHasComboA)
	{
		ComboSetCandidates.Add(EWukongComboSet::ComboA);
	}
	if (bHasComboB)
	{
		ComboSetCandidates.Add(EWukongComboSet::ComboB);
	}

	PlannedComboSet = ComboSetCandidates[FMath::RandRange(0, ComboSetCandidates.Num() - 1)];

	const TArray<TObjectPtr<UAnimMontage>>& SelectedComboMontages =
		PlannedComboSet == EWukongComboSet::ComboA ? ComboSetAMontages : ComboSetBMontages;

	if (SelectedComboMontages.Num() <= 0)
	{
		return false;
	}

	TArray<int32> ComboLengthCandidates;
	for (int32 ComboLength = 1; ComboLength <= SelectedComboMontages.Num(); ++ComboLength)
	{
		if (ComboLength <= MinComboLengthExclusive)
		{
			continue;
		}

		if (!WasRecentlyUsed(PlannedComboSet, ComboLength, 1))
		{
			ComboLengthCandidates.Add(ComboLength);
		}
	}

	if (ComboLengthCandidates.Num() == 0)
	{
		for (int32 ComboLength = 1; ComboLength <= SelectedComboMontages.Num(); ++ComboLength)
		{
			if (ComboLength <= MinComboLengthExclusive)
			{
				continue;
			}

			ComboLengthCandidates.Add(ComboLength);
		}
	}

	if (ComboLengthCandidates.Num() == 0)
	{
		return false;
	}

	PlannedComboLength = ComboLengthCandidates[FMath::RandRange(0, ComboLengthCandidates.Num() - 1)];
	return true;
}
