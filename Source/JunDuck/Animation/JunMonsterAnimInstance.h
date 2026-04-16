

#pragma once

#include "CoreMinimal.h"
#include "Animation/JunAnimInstance.h"
#include "JunMonsterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class JUNDUCK_API UJunMonsterAnimInstance : public UJunAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	bool bHasTarget = false;

	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	bool bIsGuard = false;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|Guard")
	bool bUseGuardBase = false;

	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	bool bIsInCombat = false;

	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|Movement")
	bool bUseRunLocomotion = false;
};
