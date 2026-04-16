


#include "Animation/JunPlayerAnimInstance.h"
#include "Character/JunPlayer.h"

void UJunPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AJunPlayer* Player = Cast<AJunPlayer>(Character);
	if (!Player)
	{
		bIsLockOn = false;
		bHasMoveInput = false;
		bUseRunLocomotion = false;
		bIsSprinting = false;
		bShouldPlayLockOnForwardRunStart = false;
		bJumpStartTriggered = false;
		bUseGuardBase = false;
		LockOnForwardRunStartTriggerRemainTime = 0.f;
		bHadMoveInputLastFrame = false;
		return;
	}

	bIsLockOn = Player->IsLockOn();
	const bool bHasMoveInputThisFrame =
		!FMath::IsNearlyZero(Player->GetDesiredMoveForward()) ||
		!FMath::IsNearlyZero(Player->GetDesiredMoveRight());
	bHasMoveInput = bHasMoveInputThisFrame;
	bUseRunLocomotion = Player->ShouldUseRunningAnim();
	bIsSprinting = Player->IsSprinting();
	bJumpStartTriggered = Player->IsJumpStartAnimTriggered();
	bUseGuardBase = Player->ShouldUseGuardBase();

	const bool bStartedMovingThisFrame = !bHadMoveInputLastFrame && bHasMoveInputThisFrame;
	const bool bShouldTriggerLockOnForwardRunStart =
		bStartedMovingThisFrame &&
		bIsLockOn &&
		Player->ShouldUseRunningAnim() &&
		Player->GetDesiredMoveForward() >= LockOnForwardRunStartForwardThreshold &&
		FMath::Abs(Player->GetDesiredMoveRight()) <= LockOnForwardRunStartSideTolerance;

	if (bShouldTriggerLockOnForwardRunStart)
	{
		LockOnForwardRunStartTriggerRemainTime = LockOnForwardRunStartTriggerDuration;
	}
	else if (LockOnForwardRunStartTriggerRemainTime > 0.f)
	{
		LockOnForwardRunStartTriggerRemainTime = FMath::Max(
			0.f,
			LockOnForwardRunStartTriggerRemainTime - DeltaSeconds
		);
	}

	bShouldPlayLockOnForwardRunStart = LockOnForwardRunStartTriggerRemainTime > 0.f;
	bHadMoveInputLastFrame = bHasMoveInputThisFrame;
}

void UJunPlayerAnimInstance::UpdateMovementDirectionData(float DeltaSeconds)
{
	AJunPlayer* Player = Cast<AJunPlayer>(Character);

	if (!Player)
	{
		Super::UpdateMovementDirectionData(DeltaSeconds);
		return;
	}

	const float OriginalInterpSpeed = MovementDirectionInterpSpeed;

	if (Player->GetGuardMoveBlendRemainTime() > 0.f)
	{
		MovementDirectionInterpSpeed = GuardMovementDirectionInterpSpeed;
	}

	Super::UpdateMovementDirectionData(DeltaSeconds);

	MovementDirectionInterpSpeed = OriginalInterpSpeed;
}
