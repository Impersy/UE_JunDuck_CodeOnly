
#pragma once

#include "CoreMinimal.h"
#include "Animation/JunAnimInstance.h"
#include "JunPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class JUNDUCK_API UJunPlayerAnimInstance : public UJunAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	virtual void UpdateMovementDirectionData(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bIsLockOn = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Movement")
	bool bHasMoveInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Movement")
	bool bUseRunLocomotion = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Movement")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Movement")
	bool bShouldPlayLockOnForwardRunStart = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Jump")
	bool bJumpStartTriggered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Guard")
	bool bUseGuardBase = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Guard")
	float GuardMovementDirectionInterpSpeed = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement")
	float LockOnForwardRunStartForwardThreshold = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement")
	float LockOnForwardRunStartSideTolerance = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement")
	float LockOnForwardRunStartTriggerDuration = 0.1f;

private:
	float LockOnForwardRunStartTriggerRemainTime = 0.f;
	bool bHadMoveInputLastFrame = false;
};
