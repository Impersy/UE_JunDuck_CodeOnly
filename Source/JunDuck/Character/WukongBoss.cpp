#include "Character/WukongBoss.h"

#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		RestoreJumpAttackGravityOverride();
		SetMonsterState(EMonsterState::Patrol);
		return;
	}

	if (!CanRemainInCombat(CurrentTarget))
	{
		RestoreJumpAttackGravityOverride();
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

	if (JumpAttackMontage)
	{
		Selection.Montage = JumpAttackMontage;
		Selection.AttackRange = JumpAttackMaxRange;
		Selection.bFaceTargetDuringAttack = true;
		Selection.FacingDuration = 0.4f;
		Selection.FacingInterpSpeed = AttackFacingInterpSpeed;
		Selection.bTryTurnAfterAttack = true;
		Selection.PostAttackTurnStartAngle = TurnStartAngle;
		return Selection;
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
}

void AWukongBoss::EnterIdleState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
	StopAIMovement();
	RefreshPlannedAttackType();
}

void AWukongBoss::EnterAttackState()
{
	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();
	bUsedChargeAttackLast = AttackSelection.Montage == ChargeAttackMontage;
	bPendingJumpAttackLaunch = false;
	JumpAttackLaunchDelayRemainTime = 0.f;

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

	if (bUsedChargeAttackLast)
	{
		bUsedChargeAttackLast = false;
	}
	else
	{
		BasicComboIndex = (BasicComboIndex + 1) % 4;
	}

	PlannedAttackType = EWukongPlannedAttackType::None;
}

void AWukongBoss::EnterRepositionState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
}

void AWukongBoss::EnterEvadeState()
{
	CombatMoveInput = FVector2D::ZeroVector;
	SetDesiredMoveAxes(0.f, 0.f);
}

void AWukongBoss::EnterTurnState()
{
	TryStartCombatTurnWithThreshold(TurnStartAngle);
}

void AWukongBoss::UpdateApproachState(float DeltaTime)
{
	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();

	UpdateCombatFacing(DeltaTime);

	if (CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	if (IsWithinSelectionRange(AttackSelection))
	{
		if (GetTargetYawAbsDelta() <= IdleEntryYawTolerance)
		{
			StopAIMovement();
			SetWukongCombatState(EWukongCombatState::Idle);
			return;
		}
	}

	if (CurrentTarget)
	{
		MoveToTarget(CurrentTarget);
	}
}

void AWukongBoss::UpdateIdleState(float DeltaTime)
{
	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();

	UpdateCombatFacing(DeltaTime);

	if (!IsWithinSelectionRange(AttackSelection))
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return;
	}

	if (CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	if (CanAttackTarget())
	{
		SetWukongCombatState(EWukongCombatState::Attack);
	}
}

void AWukongBoss::UpdateAttackState(float DeltaTime)
{
	if (!bIsAttacking)
	{
		bPendingJumpAttackLaunch = false;
		JumpAttackLaunchDelayRemainTime = 0.f;
		RestoreJumpAttackGravityOverride();
		SetWukongCombatState(EWukongCombatState::Recovery);
	}
}

void AWukongBoss::UpdateRecoveryState(float DeltaTime)
{
	UpdateCombatFacing(DeltaTime);

	if (CombatSubStateElapsedTime < RecoveryDuration)
	{
		return;
	}

	if (CanStartWukongTurn())
	{
		SetWukongCombatState(EWukongCombatState::Turn);
		return;
	}

	const FMonsterAttackSelection AttackSelection = ChooseNextAttackSelection();
	if (!IsWithinSelectionRange(AttackSelection))
	{
		SetWukongCombatState(EWukongCombatState::Approach);
		return;
	}

	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateRepositionState(float DeltaTime)
{
	SetWukongCombatState(EWukongCombatState::Idle);
}

void AWukongBoss::UpdateEvadeState(float DeltaTime)
{
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

bool AWukongBoss::IsWithinSelectionRange(const FMonsterAttackSelection& Selection) const
{
	if (!CurrentTarget || !Selection.Montage)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared2D(GetActorLocation(), CurrentTarget->GetActorLocation());
	return DistSq <= FMath::Square(Selection.AttackRange);
}

UAnimMontage* AWukongBoss::GetBasicComboMontage() const
{
	switch (BasicComboIndex)
	{
	case 0:
		return BasicAttack1Montage.Get();
	case 1:
		return BasicAttack2Montage.Get();
	case 2:
		return BasicAttack3Montage.Get();
	case 3:
	default:
		return BasicAttack4Montage.Get();
	}
}

float AWukongBoss::GetTargetYawAbsDelta() const
{
	return FMath::Abs(GetCombatTargetYawDelta());
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

	if (bJumpAttackGravityOverrideActive && !GetCharacterMovement()->IsFalling())
	{
		RestoreJumpAttackGravityOverride();
	}
}

void AWukongBoss::ExecuteJumpAttackLaunch()
{
	ApplyJumpAttackGravityOverride();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Cyan,
				FString::Printf(
					TEXT("JumpAttack GravityScale: %.2f / GravityZ: %.2f"),
					MovementComponent->GravityScale,
					MovementComponent->GetGravityZ()
				)
			);
		}
	}

	FVector LaunchVelocity = FVector(0.f, 0.f, JumpAttackLaunchZVelocity);
	if (CurrentTarget && JumpAttackLaunchForwardSpeed > 0.f)
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;

		if (!ToTarget.IsNearlyZero())
		{
			const FVector LaunchDirection = ToTarget.GetSafeNormal();
			const float DistanceToTarget = ToTarget.Size();
			const float DesiredTravelDistance = FMath::Max(0.f, DistanceToTarget - JumpAttackTargetStandOffDistance);

			float ForwardSpeed = JumpAttackLaunchForwardSpeed;
			if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
			{
				const float GravityMagnitude = FMath::Abs(MovementComponent->GetGravityZ());
				if (GravityMagnitude > KINDA_SMALL_NUMBER && JumpAttackLaunchZVelocity > 0.f)
				{
					const float EstimatedAirTime = (2.f * JumpAttackLaunchZVelocity) / GravityMagnitude;
					if (EstimatedAirTime > KINDA_SMALL_NUMBER)
					{
						ForwardSpeed = FMath::Min(
							JumpAttackLaunchForwardSpeed,
							DesiredTravelDistance / EstimatedAirTime
						);
					}
				}
			}

			LaunchVelocity.X = LaunchDirection.X * ForwardSpeed;
			LaunchVelocity.Y = LaunchDirection.Y * ForwardSpeed;
		}
	}

	LaunchCharacter(LaunchVelocity, true, true);
}

void AWukongBoss::ApplyJumpAttackGravityOverride()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (!bJumpAttackGravityOverrideActive)
	{
		DefaultGravityScale = MovementComponent->GravityScale;
	}

	MovementComponent->GravityScale = JumpAttackGravityScale;
	bJumpAttackGravityOverrideActive = true;
}

void AWukongBoss::RestoreJumpAttackGravityOverride()
{
	if (!bJumpAttackGravityOverrideActive)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->GravityScale = DefaultGravityScale;
	}

	bJumpAttackGravityOverrideActive = false;
}

void AWukongBoss::RefreshPlannedAttackType()
{
	if (CanUseChargeAttack() && CanUseJumpAttack())
	{
		PlannedAttackType = ChooseRandomSpecialAttackType();
		return;
	}

	if (CanUseChargeAttack())
	{
		PlannedAttackType = EWukongPlannedAttackType::ChargeAttack;
		return;
	}

	if (CanUseJumpAttack())
	{
		PlannedAttackType = EWukongPlannedAttackType::JumpAttack;
		return;
	}

	PlannedAttackType = EWukongPlannedAttackType::BasicCombo;
}

EWukongPlannedAttackType AWukongBoss::ChooseRandomSpecialAttackType() const
{
	return FMath::RandBool()
		? EWukongPlannedAttackType::JumpAttack
		: EWukongPlannedAttackType::ChargeAttack;
}
