#include "Character/WukongBoss.h"

#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "JunGameplayTags.h"

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
		case EWukongPlannedAttackType::NormalAttack:
			return TEXT("NormalAttack");
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

	const TCHAR* GetWukongNormalAttackDebugText(EWukongNormalAttackType AttackType)
	{
		switch (AttackType)
		{
		case EWukongNormalAttackType::JumpAttack:
			return TEXT("JumpAttack");
		case EWukongNormalAttackType::ChargeAttack:
			return TEXT("ChargeAttack");
		case EWukongNormalAttackType::DodgeAttack:
			return TEXT("DodgeAttack");
		case EWukongNormalAttackType::Ambush:
			return TEXT("Ambush");
		case EWukongNormalAttackType::Heavy:
			return TEXT("Heavy");
		case EWukongNormalAttackType::Spin:
			return TEXT("Spin");
		case EWukongNormalAttackType::NinjaA:
			return TEXT("NinjaA");
		case EWukongNormalAttackType::NinjaB:
			return TEXT("NinjaB");
		case EWukongNormalAttackType::Execution:
			return TEXT("Execution");
		case EWukongNormalAttackType::None:
		default:
			return TEXT("None");
		}
	}

	constexpr EWukongNormalAttackType AllWukongNormalAttackTypes[] = {
		EWukongNormalAttackType::JumpAttack,
		EWukongNormalAttackType::ChargeAttack,
		EWukongNormalAttackType::DodgeAttack,
		EWukongNormalAttackType::Ambush,
		EWukongNormalAttackType::Heavy,
		EWukongNormalAttackType::Spin,
		EWukongNormalAttackType::NinjaA,
		EWukongNormalAttackType::NinjaB,
		EWukongNormalAttackType::Execution
	};
}

AWukongBoss::AWukongBoss()
{
	WalkMoveSpeed = 200.f;
	RunMoveSpeed = 700.f;
	JumpAttack.MinRange = 0.f;
	JumpAttack.MaxRange = 700.f;
	JumpAttack.CandidateMaxRange = 900.f;
	JumpAttack.bFaceTargetDuringAttack = true;
	JumpAttack.FacingDuration = (0.7f / 1.4f);
	JumpAttack.FacingInterpSpeed = 24.f;
	JumpAttack.PlayRate = 1.4f;
	JumpAttack.MoveStartTimeAtPlayRateOne = 0.14f;
	JumpAttack.MoveSpeed = 1200.f;
	JumpAttack.MoveStandOffDistance = 20.f;
	JumpAttack.GroundMotionOverrideDuration = 0.8f;
	JumpAttack.AirTrackInterpSpeed = 8.f;
	JumpAttack.bTryTurnAfterAttack = true;
	JumpAttack.PostAttackTurnStartAngle = TurnStartAngle;

	ChargeAttack.MinRange = 400.f;
	ChargeAttack.MaxRange = 700.f;
	ChargeAttack.CandidateMaxRange = 900.f;
	ChargeAttack.bFaceTargetDuringAttack = true;
	ChargeAttack.FacingDuration = 0.8f;
	ChargeAttack.FacingInterpSpeed = 20.f;
	ChargeAttack.bTryTurnAfterAttack = true;
	ChargeAttack.PostAttackTurnStartAngle = 45.f;

	DodgeAttack.MinRange = 500.f;
	DodgeAttack.MaxRange = 700.f;
	DodgeAttack.CandidateMaxRange = 900.f;
	DodgeAttack.bFaceTargetDuringAttack = true;
	DodgeAttack.FacingDuration = 1.4f;
	DodgeAttack.FacingInterpSpeed = 16.f;
	DodgeAttack.bTryTurnAfterAttack = true;
	DodgeAttack.PostAttackTurnStartAngle = 45.f;
	DodgeAttack.SelectionWeight = 1;

	AmbushAttack.FacingDuration = 0.5f;
	ExecutionAttack.FacingDuration = 1.5f;
	HeavyAttack.FacingDuration = 1.2f;
	HeavyAttack.PlayRate = 0.8f;
	HeavyAttack.MoveStartTimeAtPlayRateOne = 0.82f;
	HeavyAttack.MoveSpeed = 2000.f;
	HeavyAttack.MoveStandOffDistance = 30.f;
	HeavyAttack.GroundMotionOverrideDuration = 0.35f;
	SpinAttack.bFaceTargetDuringAttack = false;
	SpinAttack.FacingDuration = 0.f;
	SpinAttack.MaxRange = 200.f;
	NinjaAAttack.FacingDuration = 1.7f;
	NinjaBAttack.FacingDuration = 1.4f;
}

void AWukongBoss::EnterCombatState()
{
	Super::EnterCombatState();
	ResetReactiveActionState();
	ResetActiveReactiveActionState();
	ResetReactiveEvadePressure();
	ClearNoAttackCandidateApproach();
	bUseNonAttackFallbackUntilAttackCandidateAppears = false;
	SetWukongCombatState(EWukongCombatState::Approach);
}

void AWukongBoss::UpdateCombat(float DeltaTime)
{
	UpdateTimedMontageSuperArmor(DeltaTime);

	if (PostHeavyHitSuperArmorRemainTime > 0.f)
	{
		PostHeavyHitSuperArmorRemainTime = FMath::Max(0.f, PostHeavyHitSuperArmorRemainTime - DeltaTime);
		if (PostHeavyHitSuperArmorRemainTime <= 0.f && !bIsAttacking)
		{
			RemoveGameplayTag(JunGameplayTags::State_Condition_PostHitArmor);
		}
	}

	if (!CurrentTarget)
	{
		RestoreAttackGroundMotionOverride();
		SetMonsterState(EMonsterState::Patrol);
		return;
	}

	if (!CanRemainInCombat(CurrentTarget))
	{
		RestoreAttackGroundMotionOverride();
		LastKnownTargetLocation = CurrentTarget->GetActorLocation();
		CurrentTarget = nullptr;
		bIsHasTarget = false;
		CombatMoveInput = FVector2D::ZeroVector;
		SetMonsterState(EMonsterState::Return);
		return;
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
	case EWukongCombatState::Hit:
		UpdateHitState(DeltaTime);
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

	if (PlannedAttackType == EWukongPlannedAttackType::NormalAttack)
	{
		if (const FWukongNormalAttackData* NormalAttackData = GetPlannedNormalAttackData())
		{
			if (NormalAttackData->Montage)
			{
				Selection.Montage = NormalAttackData->Montage;
				Selection.AttackRange = NormalAttackData->MaxRange;
				Selection.bFaceTargetDuringAttack = NormalAttackData->bFaceTargetDuringAttack;
				Selection.FacingDuration = NormalAttackData->FacingDuration;
				Selection.FacingInterpSpeed = NormalAttackData->FacingInterpSpeed;
				Selection.PlayRate = NormalAttackData->PlayRate;
				Selection.bTryTurnAfterAttack = NormalAttackData->bTryTurnAfterAttack;
				Selection.PostAttackTurnStartAngle = NormalAttackData->PostAttackTurnStartAngle;
				return Selection;
			}
		}
	}

	if (PlannedAttackType == EWukongPlannedAttackType::ComboAttack)
	{
		if (UAnimMontage* PlannedComboMontage = GetPlannedComboMontage())
		{
			const FWukongComboAttackData* ComboAttackData = GetComboAttackData(PlannedComboSet);
			Selection.Montage = PlannedComboMontage;
			Selection.AttackRange = ComboAttackData ? ComboAttackData->AttackRange : 250.f;
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
	case EWukongCombatState::Hit:
		EnterHitState();
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
	ResetApproachRuntimeState();
}

void AWukongBoss::EnterIdleState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bRunLocomotionRequested = false;
	bUseIdleHoldForYawMismatch = false;

	if (!HasQueuedReactiveAction())
	{
		RefreshPlannedCombatPlan();
	}

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

void AWukongBoss::EnterHitState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	RestoreAttackGroundMotionOverride();
	ClearTimedMontageSuperArmor();
	RemoveGameplayTag(JunGameplayTags::State_Condition_PostHitArmor);
	ResetReactiveActionState();
	ResetActiveReactiveActionState();
	ResetPendingAttackMotionState();
	ResetComboDodgeFollowUpState();
	ResetCurrentAttackRuntimeState();
	ResetPlannedCombatPlan();
}

void AWukongBoss::EnterAttackState()
{
	ResetReactiveEvadePressure();
	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();
	const FWukongNormalAttackData* CurrentNormalAttackData =
		(PlannedAttackType == EWukongPlannedAttackType::NormalAttack)
			? GetPlannedNormalAttackData()
			: nullptr;
	const float CurrentNormalAttackPlayRate = CurrentNormalAttackData
		? FMath::Max(CurrentNormalAttackData->PlayRate, KINDA_SMALL_NUMBER)
		: 1.f;

	ResetPendingAttackMotionState();
	ResetCurrentAttackRuntimeState();

	if (PlannedAttackType == EWukongPlannedAttackType::NormalAttack && AttackSelection.Montage != nullptr)
	{
		CurrentAttackActionType = EWukongActionType::NormalAttack;
		CurrentAttackNormalAttackType = PlannedNormalAttackType;
	}
	else if (AttackSelection.Montage == GetPlannedComboMontage())
	{
		CurrentAttackActionType = EWukongActionType::ComboAttack;
		CurrentAttackComboSet = PlannedComboSet;
		CurrentAttackComboLength = PlannedComboLength;
	}

	if (CurrentAttackActionType == EWukongActionType::NormalAttack &&
		CurrentAttackNormalAttackType == EWukongNormalAttackType::JumpAttack)
	{
		const float JumpMoveStartTime = CurrentNormalAttackData
			? CurrentNormalAttackData->MoveStartTimeAtPlayRateOne
			: 0.f;
		bPendingJumpAttackLaunch = true;
		JumpAttackLaunchDelayRemainTime = JumpMoveStartTime / CurrentNormalAttackPlayRate;
	}

	if (CurrentAttackActionType == EWukongActionType::NormalAttack &&
		CurrentAttackNormalAttackType == EWukongNormalAttackType::Heavy)
	{
		const float HeavyMoveStartTime = CurrentNormalAttackData
			? CurrentNormalAttackData->MoveStartTimeAtPlayRateOne
			: 0.f;
		bPendingHeavyAttackMove = true;
		HeavyAttackMoveDelayRemainTime = HeavyMoveStartTime / CurrentNormalAttackPlayRate;
	}

	const float AttackMontageDuration = AttackSelection.Montage
		? AttackSelection.Montage->GetPlayLength() / FMath::Max(AttackSelection.PlayRate, KINDA_SMALL_NUMBER)
		: 0.f;
	ApplyTimedMontageSuperArmor(AttackMontageDuration);

	TryAttack();
}

void AWukongBoss::EnterRecoveryState()
{
	ResetReactiveEvadePressure();
	ClearTimedMontageSuperArmor();
	ResetActiveReactiveActionState();
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	RestoreAttackGroundMotionOverride();
	ResetComboDodgeFollowUpState();

	if (!bAttackInterruptedByHitReact && CurrentAttackActionType != EWukongActionType::None)
	{
		RecordAction(CurrentAttackActionType, CurrentAttackComboSet, CurrentAttackComboLength, CurrentAttackNormalAttackType);
	}

	const bool bCanStartComboDodgeFollowUp =
		!bAttackInterruptedByHitReact &&
		CurrentAttackActionType == EWukongActionType::ComboAttack &&
		!bCurrentAttackStartedFromComboDodgeFollowUp &&
		(CurrentAttackComboLength == 1 || CurrentAttackComboLength == 2);

	if (bCanStartComboDodgeFollowUp)
	{
		TArray<TPair<UAnimMontage*, EWukongMovementDirection>> DodgeCandidates;
		if (DodgeLeftMontage)
		{
			DodgeCandidates.Emplace(DodgeLeftMontage.Get(), EWukongMovementDirection::Left);
		}
		if (DodgeRightMontage)
		{
			DodgeCandidates.Emplace(DodgeRightMontage.Get(), EWukongMovementDirection::Right);
		}

		if (DodgeCandidates.Num() > 0)
		{
			const TPair<UAnimMontage*, EWukongMovementDirection>& SelectedDodge =
				DodgeCandidates[FMath::RandRange(0, DodgeCandidates.Num() - 1)];
			if (CurrentTarget)
			{
				FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
				ToTarget.Z = 0.f;
				if (!ToTarget.IsNearlyZero())
				{
					SetActorRotation(ToTarget.Rotation());
				}
			}
			PlayAnimMontage(SelectedDodge.Key);
			PlannedNonAttackType = EWukongPlannedNonAttackType::Dodge;
			PlannedMovementDirection = SelectedDodge.Value;
			PendingComboDodgeFollowUpDuration = SelectedDodge.Key
				? SelectedDodge.Key->GetPlayLength()
				: ComboDodgeFollowUpFallbackDuration;
			PendingComboDodgeFollowUpMinComboLengthExclusive = CurrentAttackComboLength;
			bPendingComboDodgeFollowUp = true;
		}
	}

	ResetPlannedCombatPlan();

	ResetCurrentAttackRuntimeState();
	bAttackInterruptedByHitReact = false;
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
		if (CurrentTarget && GetTargetDistance2D() < StrafeMinDistanceToTarget)
		{
			PlannedMovementDirection = EWukongMovementDirection::Backward;
		}
		else
		{
			PlannedMovementDirection = FMath::RandBool() ? EWukongMovementDirection::Left : EWukongMovementDirection::Right;
		}
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
		switch (PlannedMovementDirection)
		{
		case EWukongMovementDirection::Left:
			CurrentRepositionDirection = -1.f;
			break;
		case EWukongMovementDirection::Backward:
			CurrentRepositionDirection = 0.f;
			break;
		case EWukongMovementDirection::Right:
		default:
			CurrentRepositionDirection = 1.f;
			break;
		}
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
	ResetReactiveEvadePressure();
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
	ResetReactiveEvadePressure();
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	bRunLocomotionRequested = false;
	ClearTimedMontageSuperArmor();
	const bool bUsingReactiveEvade = ActiveReactiveActionData.Type != EWukongReactiveActionType::None;

	if (CurrentTarget)
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;
		if (!ToTarget.IsNearlyZero())
		{
			SetActorRotation(ToTarget.Rotation());
		}
	}

	if (UAnimMontage* NonAttackMontage = bUsingReactiveEvade ? GetReactiveActionMontage() : GetPlannedNonAttackMontage())
	{
		const float PlayedMontageLength = PlayAnimMontage(NonAttackMontage);
		if (PlayedMontageLength > 0.f)
		{
			ApplyTimedMontageSuperArmor(PlayedMontageLength, 1.f);
			if (bUsingReactiveEvade)
			{
				ActiveReactiveActionData.Duration = PlayedMontageLength;
			}
			else
			{
				PlannedMovementDuration = PlayedMontageLength;
			}
		}
		else
		{
			ApplyTimedMontageSuperArmor(NonAttackMontage->GetPlayLength(), 1.f);
			if (bUsingReactiveEvade)
			{
				ActiveReactiveActionData.Duration = NonAttackMontage->GetPlayLength();
			}
			else
			{
				PlannedMovementDuration = NonAttackMontage->GetPlayLength();
			}
		}
	}
	else if (bUsingReactiveEvade)
	{
		if (ActiveReactiveActionData.Duration <= 0.f)
		{
			ActiveReactiveActionData.Duration = EvadeFallbackDuration;
		}
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

	if (bNoAttackCandidateApproachInProgress)
	{
		if (HasAnyAttackCandidateForCurrentDistance())
		{
			ClearNoAttackCandidateApproach();
			bUseNonAttackFallbackUntilAttackCandidateAppears = false;
			bShouldStartNoAttackFallbackWithStrafe = true;
			ResetPlannedCombatPlan();
			SetWukongCombatState(EWukongCombatState::Idle);
			return;
		}

		if (GetTargetYawAbsDelta() >= MoveTurnStartAngle && CanStartWukongTurn())
		{
			SetWukongCombatState(EWukongCombatState::Turn);
			return;
		}

		NoAttackCandidateApproachRemainTime = FMath::Max(0.f, NoAttackCandidateApproachRemainTime - DeltaTime);
		UpdateFacingTowardsTargetWithoutTurn(DeltaTime, ApproachFacingInterpSpeed);
		bRunLocomotionRequested = false;

		if (CurrentTarget)
		{
			MoveToTarget(CurrentTarget);
		}

		if (NoAttackCandidateApproachRemainTime <= 0.f)
		{
			ClearNoAttackCandidateApproach();
			bUseNonAttackFallbackUntilAttackCandidateAppears = true;
			ResetPlannedCombatPlan();
			SetWukongCombatState(EWukongCombatState::Idle);
		}

		return;
	}

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
	if (TryEnterReactiveStateFromIdle())
	{
		return;
	}

	EnsureCombatPlan();
	TryEnterPlannedStateFromIdle();
}

void AWukongBoss::UpdateAttackState(float DeltaTime)
{
	if (!bIsAttacking)
	{
		ResetPendingAttackMotionState();
		RestoreAttackGroundMotionOverride();
		SetWukongCombatState(EWukongCombatState::Recovery);
	}
}

void AWukongBoss::UpdateHitState(float DeltaTime)
{
	StopAIMovement();
	SetDesiredMoveAxes(0.f, 0.f);

	if (!IsInHitReact())
	{
		SetWukongCombatState(EWukongCombatState::Idle);
	}
}

void AWukongBoss::UpdateRecoveryState(float DeltaTime)
{
	if (bPendingComboDodgeFollowUp)
	{
		if (CurrentTarget)
		{
			UpdateFacingTowardsTargetWithoutTurn(DeltaTime, EvadeFacingInterpSpeed);
		}

		if (CombatSubStateElapsedTime < PendingComboDodgeFollowUpDuration)
		{
			return;
		}

		bPendingComboDodgeFollowUp = false;
		PendingComboDodgeFollowUpDuration = 0.f;
		RecordAction(EWukongActionType::Dodge);
		ResetPlannedCombatPlan();

		if (TryPlanComboAttackOnly(PendingComboDodgeFollowUpMinComboLengthExclusive))
		{
			if (!IsTargetTooFarForPlannedAttack() && !IsTargetTooCloseForPlannedAttack())
			{
				bCurrentAttackStartedFromComboDodgeFollowUp = true;
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
	const float ResolvedStrafeAnimInputStrength = FMath::Clamp(StrafeAnimInputStrength, 0.f, 0.4f);
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
		if (CurrentTarget && GetTargetDistance2D() < StrafeMinDistanceToTarget)
		{
			if (DashBackwardMontage)
			{
				PlannedNonAttackType = EWukongPlannedNonAttackType::Dash;
				PlannedMovementDirection = EWukongMovementDirection::Backward;
				SetWukongCombatState(EWukongCombatState::Evade);
				return;
			}

			PlannedMovementDirection = EWukongMovementDirection::Backward;
		}

		if (PlannedMovementDirection == EWukongMovementDirection::Backward)
		{
			AddMovementInput(-GetActorForwardVector(), StrafeMoveInputStrength);
			SetDesiredMoveAxes(-StrafeAnimInputStrength, 0.f);
		}
		else
		{
			const FVector RightDirection = GetActorRightVector();
			AddMovementInput(RightDirection, CurrentRepositionDirection * ResolvedStrafeMoveInputStrength);
			SetDesiredMoveAxes(0.f, CurrentRepositionDirection * ResolvedStrafeAnimInputStrength);
		}

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
	if (IsExpressiveNonAttackType(PlannedNonAttackType) &&
		PlannedNonAttackType != EWukongPlannedNonAttackType::Hold)
	{
		NextExpressiveNonAttackType = GetNextExpressiveNonAttackTypeInSequence(PlannedNonAttackType);
	}
	ResetPlannedCombatPlan();
	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateEvadeState(float DeltaTime)
{
	const bool bUsingReactiveEvade = ActiveReactiveActionData.Type != EWukongReactiveActionType::None;
	const EWukongPlannedNonAttackType ResolvedEvadeType =
		bUsingReactiveEvade ? ActiveReactiveActionData.NonAttackType : PlannedNonAttackType;
	const EWukongMovementDirection ResolvedEvadeDirection =
		bUsingReactiveEvade ? ActiveReactiveActionData.MovementDirection : PlannedMovementDirection;
	const float ResolvedEvadeDuration =
		bUsingReactiveEvade ? ActiveReactiveActionData.Duration : PlannedMovementDuration;
	const bool bIsBackwardDodgeEvade =
		ResolvedEvadeType == EWukongPlannedNonAttackType::Dodge &&
		ResolvedEvadeDirection == EWukongMovementDirection::Backward;

	if (CurrentTarget && !bIsBackwardDodgeEvade)
	{
		UpdateFacingTowardsTargetWithoutTurn(DeltaTime, EvadeFacingInterpSpeed);
	}

	if (CombatSubStateElapsedTime < ResolvedEvadeDuration)
	{
		return;
	}

	switch (ResolvedEvadeType)
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

	if (bUsingReactiveEvade)
	{
		ResetActiveReactiveActionState();
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

	const bool bCanUseCombo = HasAnyComboMontage();
	const float MaxComboCandidateRange = FMath::Max(ComboAttackA.CandidateRange, ComboAttackB.CandidateRange);
	if (bCanUseCombo && IsWithinRange(0.f, MaxComboCandidateRange))
	{
		return true;
	}

	for (const EWukongNormalAttackType NormalAttackType : {
		EWukongNormalAttackType::JumpAttack,
		EWukongNormalAttackType::ChargeAttack,
		EWukongNormalAttackType::DodgeAttack,
		EWukongNormalAttackType::Ambush,
		EWukongNormalAttackType::Heavy,
		EWukongNormalAttackType::Spin,
		EWukongNormalAttackType::NinjaA,
		EWukongNormalAttackType::NinjaB,
		EWukongNormalAttackType::Execution })
	{
		const FWukongNormalAttackData* NormalAttackData = GetNormalAttackData(NormalAttackType);
		if (NormalAttackData &&
			NormalAttackData->Montage &&
			IsWithinRange(NormalAttackData->MinRange, NormalAttackData->CandidateMaxRange))
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
	case EWukongPlannedAttackType::NormalAttack:
		{
			const FWukongNormalAttackData* NormalAttackData = GetPlannedNormalAttackData();
			return NormalAttackData && NormalAttackData->Montage != nullptr;
		}
	case EWukongPlannedAttackType::None:
	default:
		return false;
	}
}

bool AWukongBoss::HasValidPlannedMovement() const
{
	if (PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe)
	{
		return true;
	}

	if (PlannedNonAttackType == EWukongPlannedNonAttackType::Hold)
	{
		return PlannedMovementDuration > 0.f;
	}

	if (IsMobilityNonAttackType(PlannedNonAttackType) || IsExpressiveNonAttackType(PlannedNonAttackType))
	{
		return GetPlannedNonAttackMontage() != nullptr;
	}

	return false;
}

bool AWukongBoss::IsMobilityNonAttackType(EWukongPlannedNonAttackType NonAttackType) const
{
	return NonAttackType == EWukongPlannedNonAttackType::Strafe ||
		NonAttackType == EWukongPlannedNonAttackType::Dash ||
		NonAttackType == EWukongPlannedNonAttackType::Dodge;
}

bool AWukongBoss::IsExpressiveNonAttackType(EWukongPlannedNonAttackType NonAttackType) const
{
	return NonAttackType == EWukongPlannedNonAttackType::ComeHere ||
		NonAttackType == EWukongPlannedNonAttackType::Sleep ||
		NonAttackType == EWukongPlannedNonAttackType::Boring ||
		NonAttackType == EWukongPlannedNonAttackType::Hold;
}

void AWukongBoss::CollectNormalAttackCandidates(float TargetDistance, bool bIgnoreRepeat, TArray<EWukongNormalAttackType>& OutCandidates) const
{
	auto IsWithinRange = [TargetDistance](float MinRange, float MaxRange)
	{
		return TargetDistance >= MinRange && TargetDistance <= MaxRange;
	};

	for (const EWukongNormalAttackType NormalAttackType : AllWukongNormalAttackTypes)
	{
		const FWukongNormalAttackData* NormalAttackData = GetNormalAttackData(NormalAttackType);
		if (!NormalAttackData || !NormalAttackData->Montage)
		{
			continue;
		}

		if (!IsWithinRange(NormalAttackData->MinRange, NormalAttackData->CandidateMaxRange))
		{
			continue;
		}

		if (!bIgnoreRepeat && WasRecentlyUsed(NormalAttackType, 1))
		{
			continue;
		}

		if (NormalAttackType == EWukongNormalAttackType::Spin &&
			!bIgnoreRepeat &&
			FMath::FRand() > SpinAttackSelectionChance)
		{
			continue;
		}

		for (int32 WeightIndex = 0; WeightIndex < FMath::Max(1, NormalAttackData->SelectionWeight); ++WeightIndex)
		{
			OutCandidates.Add(NormalAttackType);
		}
	}
}

int32 AWukongBoss::GetComboAttackSelectionWeight() const
{
	int32 ComboAttackSelectionWeight = 0;
	if (ComboAttackA.Montages.Num() > 0)
	{
		ComboAttackSelectionWeight += FMath::Max(1, ComboAttackA.SelectionWeight);
	}

	if (ComboAttackB.Montages.Num() > 0)
	{
		ComboAttackSelectionWeight += FMath::Max(1, ComboAttackB.SelectionWeight);
	}

	return ComboAttackSelectionWeight;
}

bool AWukongBoss::HasAnyComboMontage() const
{
	return ComboAttackA.Montages.Num() > 0 || ComboAttackB.Montages.Num() > 0;
}

const FWukongComboAttackData* AWukongBoss::GetComboAttackData(EWukongComboSet ComboSet) const
{
	switch (ComboSet)
	{
	case EWukongComboSet::ComboA:
		return &ComboAttackA;
	case EWukongComboSet::ComboB:
		return &ComboAttackB;
	case EWukongComboSet::None:
	default:
		return nullptr;
	}
}

const FWukongReactiveActionTuningData* AWukongBoss::GetReactiveActionTuningData(EWukongReactiveActionType ReactiveActionType) const
{
	switch (ReactiveActionType)
	{
	case EWukongReactiveActionType::EvadeBackward:
		return &ReactiveBackwardEvade;
	case EWukongReactiveActionType::None:
	default:
		return nullptr;
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

const FWukongNormalAttackData* AWukongBoss::GetNormalAttackData(EWukongNormalAttackType AttackType) const
{
	switch (AttackType)
	{
	case EWukongNormalAttackType::JumpAttack:
		return &JumpAttack;
	case EWukongNormalAttackType::ChargeAttack:
		return &ChargeAttack;
	case EWukongNormalAttackType::DodgeAttack:
		return &DodgeAttack;
	case EWukongNormalAttackType::Ambush:
		return &AmbushAttack;
	case EWukongNormalAttackType::Heavy:
		return &HeavyAttack;
	case EWukongNormalAttackType::Spin:
		return &SpinAttack;
	case EWukongNormalAttackType::NinjaA:
		return &NinjaAAttack;
	case EWukongNormalAttackType::NinjaB:
		return &NinjaBAttack;
	case EWukongNormalAttackType::Execution:
		return &ExecutionAttack;
	case EWukongNormalAttackType::None:
	default:
		return nullptr;
	}
}

const FWukongNormalAttackData* AWukongBoss::GetPlannedNormalAttackData() const
{
	return GetNormalAttackData(PlannedNormalAttackType);
}

UAnimMontage* AWukongBoss::GetPlannedComboMontage() const
{
	const FWukongComboAttackData* ComboAttackData = GetComboAttackData(PlannedComboSet);
	const TArray<TObjectPtr<UAnimMontage>>* ComboMontages = ComboAttackData ? &ComboAttackData->Montages : nullptr;

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
	const FWukongComboAttackData* ComboAttackData = GetComboAttackData(PlannedComboSet);
	const TArray<float>* FacingDurations = ComboAttackData ? &ComboAttackData->FacingDurations : nullptr;

	if (!FacingDurations || PlannedComboLength <= 0)
	{
		return -1.f;
	}

	const int32 DurationIndex = PlannedComboLength - 1;
	return FacingDurations->IsValidIndex(DurationIndex)
		? (*FacingDurations)[DurationIndex]
		: -1.f;
}

UAnimMontage* AWukongBoss::GetExpressiveNonAttackMontage(EWukongPlannedNonAttackType NonAttackType) const
{
	switch (NonAttackType)
	{
	case EWukongPlannedNonAttackType::ComeHere:
		return ComeHereMontage.Get();
	case EWukongPlannedNonAttackType::Sleep:
		return SleepMontage.Get();
	case EWukongPlannedNonAttackType::Boring:
		return BoringMontage.Get();
	case EWukongPlannedNonAttackType::Hold:
	case EWukongPlannedNonAttackType::None:
	default:
		return nullptr;
	}
}

UAnimMontage* AWukongBoss::GetMobilityNonAttackMontage(EWukongPlannedNonAttackType NonAttackType, EWukongMovementDirection MovementDirection) const
{
	switch (NonAttackType)
	{
	case EWukongPlannedNonAttackType::Dash:
		switch (MovementDirection)
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
		switch (MovementDirection)
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
	case EWukongPlannedNonAttackType::Strafe:
	case EWukongPlannedNonAttackType::Hold:
	case EWukongPlannedNonAttackType::ComeHere:
	case EWukongPlannedNonAttackType::Sleep:
	case EWukongPlannedNonAttackType::Boring:
	case EWukongPlannedNonAttackType::None:
	default:
		return nullptr;
	}
}

UAnimMontage* AWukongBoss::GetPlannedNonAttackMontage() const
{
	if (IsExpressiveNonAttackType(PlannedNonAttackType))
	{
		return GetExpressiveNonAttackMontage(PlannedNonAttackType);
	}

	if (IsMobilityNonAttackType(PlannedNonAttackType))
	{
		return GetMobilityNonAttackMontage(PlannedNonAttackType, PlannedMovementDirection);
	}

	return nullptr;
}

UAnimMontage* AWukongBoss::GetReactiveActionMontage() const
{
	return GetMobilityNonAttackMontage(ActiveReactiveActionData.NonAttackType, ActiveReactiveActionData.MovementDirection);
}

EWukongCombatState AWukongBoss::GetReactiveActionTargetState(EWukongReactiveActionType ReactiveActionType) const
{
	switch (ReactiveActionType)
	{
	case EWukongReactiveActionType::EvadeBackward:
		return EWukongCombatState::Evade;
	case EWukongReactiveActionType::None:
	default:
		return EWukongCombatState::Idle;
	}
}

void AWukongBoss::RecordAction(EWukongActionType ActionType, EWukongComboSet ComboSet, int32 ComboLength, EWukongNormalAttackType NormalAttackType)
{
	FWukongActionRecord Record;
	Record.ActionType = ActionType;
	Record.ComboSet = ComboSet;
	Record.ComboLength = ComboLength;
	Record.NormalAttackType = NormalAttackType;

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

bool AWukongBoss::WasRecentlyUsed(EWukongNormalAttackType AttackType, int32 Depth) const
{
	const int32 NumToCheck = FMath::Min(Depth, RecentActionHistory.Num());
	for (int32 Index = RecentActionHistory.Num() - 1; Index >= RecentActionHistory.Num() - NumToCheck; --Index)
	{
		if (RecentActionHistory[Index].NormalAttackType == AttackType)
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
	case EWukongPlannedAttackType::NormalAttack:
		{
			const FWukongNormalAttackData* NormalAttackData = GetPlannedNormalAttackData();
			return NormalAttackData ? NormalAttackData->MinRange : 0.f;
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
	case EWukongPlannedAttackType::NormalAttack:
		{
			const FWukongNormalAttackData* NormalAttackData = GetPlannedNormalAttackData();
			return NormalAttackData ? NormalAttackData->MaxRange : 0.f;
		}
	case EWukongPlannedAttackType::ComboAttack:
		{
			const FWukongComboAttackData* ComboAttackData = GetComboAttackData(PlannedComboSet);
			return ComboAttackData ? ComboAttackData->AttackRange : 250.f;
		}
	case EWukongPlannedAttackType::None:
	default:
		return 0.f;
	}
}

float AWukongBoss::GetAttackCandidateMaxRange(EWukongPlannedAttackType AttackType) const
{
	switch (AttackType)
	{
	case EWukongPlannedAttackType::NormalAttack:
		{
			const FWukongNormalAttackData* NormalAttackData = GetPlannedNormalAttackData();
			return NormalAttackData ? NormalAttackData->CandidateMaxRange : 0.f;
		}
	case EWukongPlannedAttackType::ComboAttack:
		{
			const FWukongComboAttackData* ComboAttackData = GetComboAttackData(PlannedComboSet);
			if (ComboAttackData)
			{
				return ComboAttackData->CandidateRange;
			}

			return FMath::Max(ComboAttackA.CandidateRange, ComboAttackB.CandidateRange);
		}
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

void AWukongBoss::ResetReactiveActionState()
{
	CurrentReactiveAction = EWukongReactiveActionType::None;
}

void AWukongBoss::ResetActiveReactiveActionState()
{
	ActiveReactiveActionData = FWukongReactiveActionExecutionData();
}

void AWukongBoss::ResetReactiveEvadePressure()
{
	ReactiveBackwardEvadeCloseHitCount = 0;
	ReactiveBackwardEvadeAccumulatedTriggerChance = 0.f;
}

void AWukongBoss::ResetPendingAttackMotionState()
{
	bPendingJumpAttackLaunch = false;
	JumpAttackLaunchDelayRemainTime = 0.f;
	bPendingHeavyAttackMove = false;
	HeavyAttackMoveDelayRemainTime = 0.f;
}

void AWukongBoss::ResetComboDodgeFollowUpState()
{
	bPendingComboDodgeFollowUp = false;
	PendingComboDodgeFollowUpDuration = 0.f;
	PendingComboDodgeFollowUpMinComboLengthExclusive = 0;
}

void AWukongBoss::ResetCurrentAttackRuntimeState()
{
	CurrentAttackActionType = EWukongActionType::None;
	CurrentAttackComboSet = EWukongComboSet::None;
	CurrentAttackComboLength = 0;
	CurrentAttackNormalAttackType = EWukongNormalAttackType::None;
	bCurrentAttackStartedFromComboDodgeFollowUp = false;
}

void AWukongBoss::ApplyTimedMontageSuperArmor(float MontageDuration, float DurationScale)
{
	const float ResolvedDurationScale = DurationScale >= 0.f ? DurationScale : MontageSuperArmorDurationScale;
	const float SuperArmorDuration = FMath::Max(0.f, MontageDuration * ResolvedDurationScale);
	TimedMontageSuperArmorRemainTime = SuperArmorDuration;

	if (TimedMontageSuperArmorRemainTime > 0.f)
	{
		AddGameplayTag(JunGameplayTags::State_Condition_SuperArmor);
	}
}

void AWukongBoss::UpdateTimedMontageSuperArmor(float DeltaTime)
{
	if (TimedMontageSuperArmorRemainTime <= 0.f)
	{
		return;
	}

	TimedMontageSuperArmorRemainTime = FMath::Max(0.f, TimedMontageSuperArmorRemainTime - DeltaTime);
	if (TimedMontageSuperArmorRemainTime <= 0.f)
	{
		RemoveGameplayTag(JunGameplayTags::State_Condition_SuperArmor);
	}
}

void AWukongBoss::ClearTimedMontageSuperArmor()
{
	TimedMontageSuperArmorRemainTime = 0.f;
	RemoveGameplayTag(JunGameplayTags::State_Condition_SuperArmor);
}

bool AWukongBoss::HasQueuedReactiveAction() const
{
	return CurrentReactiveAction != EWukongReactiveActionType::None;
}

bool AWukongBoss::TryEnterReactiveStateFromIdle()
{
	return TryConsumeReactiveAction();
}

bool AWukongBoss::TryEnterPlannedStateFromIdle()
{
	if (bNoAttackCandidateApproachInProgress)
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return true;
	}

	switch (PlannedActionType)
	{
	case EWukongPlannedActionType::Attack:
		return TryEnterPlannedAttackStateFromIdle();
	case EWukongPlannedActionType::NonAttack:
		return TryEnterPlannedNonAttackStateFromIdle();
	case EWukongPlannedActionType::None:
	default:
		return false;
	}
}

bool AWukongBoss::TryEnterPlannedAttackStateFromIdle()
{
	if (bUseIdleHoldForYawMismatch && CombatSubStateElapsedTime < YawMismatchIdleHoldDuration)
	{
		SetDesiredMoveAxes(0.f, 0.f);
		return true;
	}

	if (!HasValidPlannedAttack())
	{
		ResetPlannedCombatPlan();
		return true;
	}

	const float TargetYawAbsDelta = GetTargetYawAbsDelta();
	if (TargetYawAbsDelta > IdleEntryYawTolerance)
	{
		if (CanStartWukongTurn())
		{
			SetWukongCombatState(EWukongCombatState::Turn);
			return true;
		}

		if (!IsTargetTooFarForPlannedAttack() && !IsTargetTooCloseForPlannedAttack())
		{
			SetWukongCombatState(EWukongCombatState::Reposition);
			return true;
		}

		SetWukongCombatState(EWukongCombatState::Approach);
		return true;
	}

	if (IsTargetTooFarForPlannedAttack())
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return true;
	}

	if (IsTargetTooCloseForPlannedAttack())
	{
		SetWukongCombatState(EWukongCombatState::Reposition);
		return true;
	}

	if (CanAttackTarget())
	{
		SetWukongCombatState(EWukongCombatState::Attack);
		return true;
	}

	return true;
}

bool AWukongBoss::TryEnterPlannedNonAttackStateFromIdle()
{
	if (!HasValidPlannedMovement())
	{
		ResetPlannedCombatPlan();
		return true;
	}

	if (GetTargetYawAbsDelta() >= MoveTurnStartAngle && CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return true;
	}

	if (PlannedNonAttackType == EWukongPlannedNonAttackType::Strafe)
	{
		if (CurrentTarget && GetTargetDistance2D() < StrafeMinDistanceToTarget)
		{
			if (DashBackwardMontage)
			{
				PlannedNonAttackType = EWukongPlannedNonAttackType::Dash;
				PlannedMovementDirection = EWukongMovementDirection::Backward;
				SetWukongCombatState(EWukongCombatState::Evade);
				return true;
			}

			PlannedMovementDirection = EWukongMovementDirection::Backward;
		}

		SetWukongCombatState(EWukongCombatState::Reposition);
		return true;
	}

	SetWukongCombatState(IsMobilityNonAttackType(PlannedNonAttackType)
		? EWukongCombatState::Evade
		: EWukongCombatState::NonAttackAction);
	return true;
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

	if (CurrentAttackActionType != EWukongActionType::NormalAttack ||
		CurrentAttackNormalAttackType != EWukongNormalAttackType::JumpAttack)
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

float AWukongBoss::GetHitReactDuration(EHitReactType HitType) const
{
	switch (HitType)
	{
	case EHitReactType::LightHit:
		return WukongLightHitDuration;
	case EHitReactType::LargeHit:
		return WukongLargeHitDuration;
	default:
		return Super::GetHitReactDuration(HitType);
	}
}

float AWukongBoss::GetHitReactControlLockDuration(EHitReactType HitType) const
{
	switch (HitType)
	{
	case EHitReactType::HeavyHit_A:
	case EHitReactType::HeavyHit_B:
	case EHitReactType::HeavyHit_C:
		return WukongHeavyHitControlLockDuration;
	default:
		return Super::GetHitReactControlLockDuration(HitType);
	}
}

void AWukongBoss::OnHitReactStarted(EHitReactType NewHitReact, ECharacterHitReactDirection NewHitDirection)
{
	Super::OnHitReactStarted(NewHitReact, NewHitDirection);

	if (CurrentState == EMonsterState::Combat)
	{
		SetWukongCombatState(EWukongCombatState::Hit);
	}
}

bool AWukongBoss::ShouldStartHitReact(EHitReactType IncomingHitReact) const
{
	const bool bIsIncomingHeavy =
		IncomingHitReact == EHitReactType::HeavyHit_A ||
		IncomingHitReact == EHitReactType::HeavyHit_B ||
		IncomingHitReact == EHitReactType::HeavyHit_C;

	if (PostHeavyHitSuperArmorRemainTime > 0.f && bIsIncomingHeavy)
	{
		return false;
	}

	return Super::ShouldStartHitReact(IncomingHitReact);
}

void AWukongBoss::OnHitReactEnded(EHitReactType EndedHitReact)
{
	Super::OnHitReactEnded(EndedHitReact);

	const bool bWasHeavy =
		EndedHitReact == EHitReactType::HeavyHit_A ||
		EndedHitReact == EHitReactType::HeavyHit_B ||
		EndedHitReact == EHitReactType::HeavyHit_C;

	if (bWasHeavy)
	{
		PostHeavyHitSuperArmorRemainTime = WukongPostHeavyHitSuperArmorDuration;
		AddGameplayTag(JunGameplayTags::State_Condition_PostHitArmor);
	}

	if (CurrentState == EMonsterState::Combat &&
		CurrentCombatSubState == EWukongCombatState::Hit)
	{
		ResetPlannedCombatPlan();

		if (TryAccumulateReactiveBackwardEvadePressure())
		{
			SetWukongCombatState(EWukongCombatState::Idle);
			return;
		}

		SetWukongCombatState(EWukongCombatState::Idle);
	}
}

FString AWukongBoss::GetMonsterDebugExtraText() const
{
	if (IsInHitReact())
	{
		return TEXT("Wukong State: HitReact");
	}

	return FString::Printf(
		TEXT("Wukong State: %s"),
		GetWukongCombatStateDebugText(CurrentCombatSubState)
	);
}

void AWukongBoss::ResetPlannedCombatPlan()
{
	PlannedActionType = EWukongPlannedActionType::None;
	PlannedAttackType = EWukongPlannedAttackType::None;
	PlannedNormalAttackType = EWukongNormalAttackType::None;
	PlannedNonAttackType = EWukongPlannedNonAttackType::None;
	PlannedMovementDirection = EWukongMovementDirection::None;
	PlannedMovementDuration = 0.f;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;
}

void AWukongBoss::ResetApproachRuntimeState()
{
	bApproachComeHereTriggered = false;
	bApproachComeHereInProgress = false;
	ApproachComeHereRemainTime = 0.f;
	bDiscardPlannedAttackAfterApproachComeHere = false;
}

void AWukongBoss::BeginNoAttackCandidateApproach()
{
	if (bNoAttackCandidateApproachInProgress)
	{
		return;
	}

	bNoAttackCandidateApproachInProgress = true;
	NoAttackCandidateApproachRemainTime = NoAttackCandidateApproachDelay;
}

void AWukongBoss::ClearNoAttackCandidateApproach()
{
	bNoAttackCandidateApproachInProgress = false;
	NoAttackCandidateApproachRemainTime = 0.f;
}

bool AWukongBoss::TryQueueReactiveAction(EWukongReactiveActionType ReactiveActionType)
{
	if (ReactiveActionType == EWukongReactiveActionType::None)
	{
		return false;
	}

	CurrentReactiveAction = ReactiveActionType;
	return true;
}

bool AWukongBoss::TryQueueReactiveBackwardEvade()
{
	if (!DashBackwardMontage && !DodgeBackwardMontage)
	{
		return false;
	}

	return TryQueueReactiveAction(EWukongReactiveActionType::EvadeBackward);
}

bool AWukongBoss::TryAccumulateReactiveBackwardEvadePressure()
{
	const FWukongReactiveActionTuningData* ReactiveEvadeTuningData =
		GetReactiveActionTuningData(EWukongReactiveActionType::EvadeBackward);

	if (!CurrentTarget || !ReactiveEvadeTuningData)
	{
		ResetReactiveEvadePressure();
		return false;
	}

	if (GetTargetDistance2D() > ReactiveEvadeTuningData->TriggerRange)
	{
		ResetReactiveEvadePressure();
		return false;
	}

	++ReactiveBackwardEvadeCloseHitCount;
	ReactiveBackwardEvadeAccumulatedTriggerChance = FMath::Clamp(
		ReactiveBackwardEvadeAccumulatedTriggerChance + ReactiveEvadeTuningData->TriggerChancePerHit,
		0.f,
		ReactiveEvadeTuningData->MaxTriggerChance
	);

	if (FMath::FRand() > ReactiveBackwardEvadeAccumulatedTriggerChance)
	{
		return false;
	}

	if (!TryQueueReactiveBackwardEvade())
	{
		return false;
	}

	ResetReactiveEvadePressure();
	return true;
}

bool AWukongBoss::TryActivateReactiveAction(EWukongReactiveActionType ReactiveActionType)
{
	switch (ReactiveActionType)
	{
	case EWukongReactiveActionType::EvadeBackward:
		if (TryPlanReactiveBackwardEvade())
		{
			ActiveReactiveActionData.Type = ReactiveActionType;
			return true;
		}
		break;
	case EWukongReactiveActionType::None:
	default:
		break;
	}

	return false;
}

bool AWukongBoss::TryConsumeReactiveAction()
{
	if (!HasQueuedReactiveAction())
	{
		return false;
	}

	const EWukongReactiveActionType ReactiveActionToConsume = CurrentReactiveAction;
	ResetReactiveActionState();
	ResetPlannedCombatPlan();

	if (!TryActivateReactiveAction(ReactiveActionToConsume))
	{
		return false;
	}

	SetWukongCombatState(GetReactiveActionTargetState(ReactiveActionToConsume));
	return true;
}

void AWukongBoss::EnsureCombatPlan()
{
	if (bNoAttackCandidateApproachInProgress)
	{
		return;
	}

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

	if (bPendingHeavyAttackMove)
	{
		HeavyAttackMoveDelayRemainTime = FMath::Max(0.f, HeavyAttackMoveDelayRemainTime - DeltaTime);
		if (HeavyAttackMoveDelayRemainTime <= 0.f)
		{
			bPendingHeavyAttackMove = false;
			ExecuteHeavyAttackMove();
		}
	}

	if (bAttackGroundMotionOverrideActive)
	{
		AttackGroundMotionOverrideRemainTime = FMath::Max(0.f, AttackGroundMotionOverrideRemainTime - DeltaTime);

		if (!GetCharacterMovement()->IsMovingOnGround() || AttackGroundMotionOverrideRemainTime <= 0.f)
		{
			RestoreAttackGroundMotionOverride();
		}
	}

	if (CurrentAttackActionType == EWukongActionType::NormalAttack &&
		CurrentAttackNormalAttackType == EWukongNormalAttackType::JumpAttack &&
		CurrentTarget &&
		GetCharacterMovement() &&
		GetCharacterMovement()->IsFalling())
	{
		const FWukongNormalAttackData* JumpAttackData = GetNormalAttackData(EWukongNormalAttackType::JumpAttack);
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
				JumpAttackData ? JumpAttackData->AirTrackInterpSpeed : 0.f
			);

			Velocity.X = NewHorizontalVelocity.X;
			Velocity.Y = NewHorizontalVelocity.Y;
			MovementComponent->Velocity = Velocity;
		}
	}
}

void AWukongBoss::ExecuteJumpAttackLaunch()
{
	const FWukongNormalAttackData* JumpAttackData = GetNormalAttackData(EWukongNormalAttackType::JumpAttack);
	FVector LaunchVelocity = FVector::ZeroVector;
	if (CurrentTarget && JumpAttackData && JumpAttackData->MoveSpeed > 0.f)
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;

		if (!ToTarget.IsNearlyZero())
		{
			const FVector LaunchDirection = ToTarget.GetSafeNormal();
			const float DistanceToTarget = ToTarget.Size();
			const float DesiredTravelDistance = FMath::Max(0.f, DistanceToTarget - JumpAttackData->MoveStandOffDistance);
			const float ForwardSpeed = DesiredTravelDistance > 0.f
				? JumpAttackData->MoveSpeed
				: 0.f;

			LaunchVelocity.X = LaunchDirection.X * ForwardSpeed;
			LaunchVelocity.Y = LaunchDirection.Y * ForwardSpeed;
		}
	}

	ApplyAttackGroundMotionOverride(JumpAttackData ? JumpAttackData->GroundMotionOverrideDuration : 0.f);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		FVector NewVelocity = MovementComponent->Velocity;
		NewVelocity.X = LaunchVelocity.X;
		NewVelocity.Y = LaunchVelocity.Y;
		MovementComponent->Velocity = NewVelocity;
	}
}

void AWukongBoss::ExecuteHeavyAttackMove()
{
	const FWukongNormalAttackData* HeavyAttackData = GetNormalAttackData(EWukongNormalAttackType::Heavy);
	if (!CurrentTarget || !HeavyAttackData || HeavyAttackData->MoveSpeed <= 0.f)
	{
		return;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FVector MoveDirection = ToTarget.GetSafeNormal();
	const float DistanceToTarget = ToTarget.Size();
	const float DesiredTravelDistance = FMath::Max(0.f, DistanceToTarget - HeavyAttackData->MoveStandOffDistance);
	const float MoveSpeed = DesiredTravelDistance > 0.f ? HeavyAttackData->MoveSpeed : 0.f;

	ApplyAttackGroundMotionOverride(HeavyAttackData->GroundMotionOverrideDuration);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		FVector NewVelocity = MovementComponent->Velocity;
		NewVelocity.X = MoveDirection.X * MoveSpeed;
		NewVelocity.Y = MoveDirection.Y * MoveSpeed;
		MovementComponent->Velocity = NewVelocity;
	}
}

void AWukongBoss::ApplyAttackGroundMotionOverride(float Duration)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || bAttackGroundMotionOverrideActive)
	{
		return;
	}

	DefaultGroundFriction = MovementComponent->GroundFriction;
	DefaultBrakingDecelerationWalking = MovementComponent->BrakingDecelerationWalking;
	DefaultBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;

	MovementComponent->GroundFriction = 0.f;
	MovementComponent->BrakingDecelerationWalking = 0.f;
	MovementComponent->BrakingFrictionFactor = 0.f;
	bAttackGroundMotionOverrideActive = true;
	AttackGroundMotionOverrideRemainTime = Duration;
}

void AWukongBoss::RestoreAttackGroundMotionOverride()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || !bAttackGroundMotionOverrideActive)
	{
		return;
	}

	MovementComponent->GroundFriction = DefaultGroundFriction;
	MovementComponent->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
	MovementComponent->BrakingFrictionFactor = DefaultBrakingFrictionFactor;
	bAttackGroundMotionOverrideActive = false;
	AttackGroundMotionOverrideRemainTime = 0.f;
}

void AWukongBoss::RefreshPlannedAttackType()
{
	if (!CurrentTarget)
	{
		PlannedAttackType = EWukongPlannedAttackType::None;
		PlannedNormalAttackType = EWukongNormalAttackType::None;
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

	const bool bCanUseCombo = HasAnyComboMontage();
	const int32 ComboAttackSelectionWeight = GetComboAttackSelectionWeight();
	const float MaxComboCandidateRange = FMath::Max(ComboAttackA.CandidateRange, ComboAttackB.CandidateRange);
	if (bCanUseCombo && IsWithinRange(0.f, MaxComboCandidateRange) && !WasRecentlyUsed(EWukongActionType::ComboAttack, 1))
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::ComboAttack, ComboAttackSelectionWeight);
	}

	TArray<EWukongNormalAttackType> NormalAttackCandidates;
	CollectNormalAttackCandidates(TargetDistance, false, NormalAttackCandidates);

	if (NormalAttackCandidates.Num() > 0)
	{
		AddWeightedAttackCandidate(EWukongPlannedAttackType::NormalAttack, NormalAttackCandidates.Num());
	}

	if (AttackCandidates.Num() == 0)
	{
		if (bCanUseCombo && IsWithinRange(0.f, MaxComboCandidateRange))
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::ComboAttack, ComboAttackSelectionWeight);
		}

		NormalAttackCandidates.Reset();
		CollectNormalAttackCandidates(TargetDistance, true, NormalAttackCandidates);

		if (NormalAttackCandidates.Num() > 0)
		{
			AddWeightedAttackCandidate(EWukongPlannedAttackType::NormalAttack, NormalAttackCandidates.Num());
		}
	}

	if (AttackCandidates.Num() <= 0)
	{
		ResetPlannedCombatPlan();
		return;
	}

	PlannedAttackType = AttackCandidates[FMath::RandRange(0, AttackCandidates.Num() - 1)];
	PlannedNormalAttackType = EWukongNormalAttackType::None;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;

	if (PlannedAttackType == EWukongPlannedAttackType::NormalAttack)
	{
		if (NormalAttackCandidates.Num() <= 0)
		{
			PlannedAttackType = EWukongPlannedAttackType::None;
			return;
		}

		PlannedNormalAttackType = NormalAttackCandidates[FMath::RandRange(0, NormalAttackCandidates.Num() - 1)];
		return;
	}

	if (PlannedAttackType != EWukongPlannedAttackType::ComboAttack)
	{
		return;
	}

	TArray<EWukongComboSet> ComboSetCandidates;
	if (ComboAttackA.Montages.Num() > 0)
	{
		for (int32 WeightIndex = 0; WeightIndex < FMath::Max(1, ComboAttackA.SelectionWeight); ++WeightIndex)
		{
			ComboSetCandidates.Add(EWukongComboSet::ComboA);
		}
	}
	if (ComboAttackB.Montages.Num() > 0)
	{
		for (int32 WeightIndex = 0; WeightIndex < FMath::Max(1, ComboAttackB.SelectionWeight); ++WeightIndex)
		{
			ComboSetCandidates.Add(EWukongComboSet::ComboB);
		}
	}

	if (ComboSetCandidates.Num() <= 0)
	{
		PlannedAttackType = EWukongPlannedAttackType::None;
		return;
	}

	PlannedComboSet = ComboSetCandidates[FMath::RandRange(0, ComboSetCandidates.Num() - 1)];

	const FWukongComboAttackData* SelectedComboAttackData = GetComboAttackData(PlannedComboSet);
	if (!SelectedComboAttackData)
	{
		PlannedAttackType = EWukongPlannedAttackType::None;
		PlannedComboSet = EWukongComboSet::None;
		return;
	}

	const TArray<TObjectPtr<UAnimMontage>>& SelectedComboMontages = SelectedComboAttackData->Montages;

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
		if (bUseNonAttackFallbackUntilAttackCandidateAppears)
		{
			PlannedActionType = EWukongPlannedActionType::NonAttack;
			RefreshNoAttackFallbackMovementType();
			return;
		}

		BeginNoAttackCandidateApproach();
		PlannedActionType = EWukongPlannedActionType::None;
		return;
	}

	ClearNoAttackCandidateApproach();
	bUseNonAttackFallbackUntilAttackCandidateAppears = false;
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
	const bool bWithinSideDodgeRange =
		!CurrentTarget || GetTargetDistance2D() <= SideDodgeMaxDistanceToTarget;

	const bool bCanUseSideDodge =
		(PlannedMovementDirection == EWukongMovementDirection::Left && DodgeLeftMontage) ||
		(PlannedMovementDirection == EWukongMovementDirection::Right && DodgeRightMontage);

	if (!bUsedDodgeRecently && bWithinSideDodgeRange && bCanUseSideDodge && FMath::FRand() <= DodgeMovementPlanWeight)
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
		IsExpressiveNonAttackType(LastCompletedNonAttackType) &&
		LastCompletedNonAttackType != EWukongPlannedNonAttackType::Hold;

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

bool AWukongBoss::TryPlanReactiveBackwardEvade()
{
	const FWukongReactiveActionTuningData* ReactiveEvadeTuningData =
		GetReactiveActionTuningData(EWukongReactiveActionType::EvadeBackward);
	if (!ReactiveEvadeTuningData)
	{
		return false;
	}

	const bool bUsedDashRecently = WasRecentlyUsed(EWukongActionType::Dash, 1);
	const bool bUsedDodgeRecently = WasRecentlyUsed(EWukongActionType::Dodge, 1);

	const bool bCanUseDash = DashBackwardMontage && !bUsedDashRecently;
	const bool bCanUseDodge = DodgeBackwardMontage && !bUsedDodgeRecently;
	const bool bCanFallbackDash = DashBackwardMontage != nullptr;
	const bool bCanFallbackDodge = DodgeBackwardMontage != nullptr;

	EWukongPlannedNonAttackType SelectedEvadeType = EWukongPlannedNonAttackType::None;
	const EWukongPlannedNonAttackType PrimaryNonAttackType = ReactiveEvadeTuningData->PrimaryNonAttackType;
	const EWukongPlannedNonAttackType SecondaryNonAttackType = ReactiveEvadeTuningData->SecondaryNonAttackType;
	const bool bPreferPrimaryType = FMath::FRand() <= ReactiveEvadeTuningData->PrimaryNonAttackTypeChance;
	const EWukongPlannedNonAttackType PreferredType = bPreferPrimaryType ? PrimaryNonAttackType : SecondaryNonAttackType;
	const EWukongPlannedNonAttackType FallbackType = bPreferPrimaryType ? SecondaryNonAttackType : PrimaryNonAttackType;

	auto CanUseNonAttackType = [bCanUseDash, bCanUseDodge](EWukongPlannedNonAttackType NonAttackType)
	{
		switch (NonAttackType)
		{
		case EWukongPlannedNonAttackType::Dash:
			return bCanUseDash;
		case EWukongPlannedNonAttackType::Dodge:
			return bCanUseDodge;
		default:
			return false;
		}
	};

	auto CanFallbackNonAttackType = [bCanFallbackDash, bCanFallbackDodge](EWukongPlannedNonAttackType NonAttackType)
	{
		switch (NonAttackType)
		{
		case EWukongPlannedNonAttackType::Dash:
			return bCanFallbackDash;
		case EWukongPlannedNonAttackType::Dodge:
			return bCanFallbackDodge;
		default:
			return false;
		}
	};

	if (bCanUseDash && bCanUseDodge)
	{
		SelectedEvadeType = PreferredType;
	}
	else if (CanUseNonAttackType(PreferredType))
	{
		SelectedEvadeType = PreferredType;
	}
	else if (CanUseNonAttackType(FallbackType))
	{
		SelectedEvadeType = FallbackType;
	}
	else if (bCanFallbackDash && bCanFallbackDodge)
	{
		SelectedEvadeType = PreferredType;
	}
	else if (CanFallbackNonAttackType(PreferredType))
	{
		SelectedEvadeType = PreferredType;
	}
	else if (CanFallbackNonAttackType(FallbackType))
	{
		SelectedEvadeType = FallbackType;
	}

	if (SelectedEvadeType == EWukongPlannedNonAttackType::None)
	{
		return false;
	}

	ActiveReactiveActionData.NonAttackType = SelectedEvadeType;
	ActiveReactiveActionData.MovementDirection = ReactiveEvadeTuningData->MovementDirection;
	ActiveReactiveActionData.Duration = 0.f;
	return true;
}

bool AWukongBoss::TryPlanComboAttackOnly(int32 MinComboLengthExclusive)
{
	const bool bHasComboA = ComboAttackA.Montages.Num() > 0;
	const bool bHasComboB = ComboAttackB.Montages.Num() > 0;
	if (!bHasComboA && !bHasComboB)
	{
		return false;
	}

	PlannedActionType = EWukongPlannedActionType::Attack;
	PlannedAttackType = EWukongPlannedAttackType::ComboAttack;
	PlannedNormalAttackType = EWukongNormalAttackType::None;
	PlannedComboSet = EWukongComboSet::None;
	PlannedComboLength = 0;

	TArray<EWukongComboSet> ComboSetCandidates;
	if (bHasComboA)
	{
		for (int32 WeightIndex = 0; WeightIndex < FMath::Max(1, ComboAttackA.SelectionWeight); ++WeightIndex)
		{
			ComboSetCandidates.Add(EWukongComboSet::ComboA);
		}
	}
	if (bHasComboB)
	{
		for (int32 WeightIndex = 0; WeightIndex < FMath::Max(1, ComboAttackB.SelectionWeight); ++WeightIndex)
		{
			ComboSetCandidates.Add(EWukongComboSet::ComboB);
		}
	}

	PlannedComboSet = ComboSetCandidates[FMath::RandRange(0, ComboSetCandidates.Num() - 1)];

	const FWukongComboAttackData* SelectedComboAttackData = GetComboAttackData(PlannedComboSet);
	if (!SelectedComboAttackData)
	{
		return false;
	}

	const TArray<TObjectPtr<UAnimMontage>>& SelectedComboMontages = SelectedComboAttackData->Montages;

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
