


#include "Animation/JunMonsterAnimInstance.h"
#include "Character/JunMonster.h"

void UJunMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AJunMonster* Monster = Cast<AJunMonster>(Character);
	if (!Monster)
	{
		bHasTarget = false;
		bIsGuard = false;
		bUseGuardBase = false;
		bIsInCombat = false;
		bIsAttacking = false;
		bIsRunning = false;
		bUseRunLocomotion = false;
		return;
	}

	bHasTarget = Monster->HasCombatTarget();
	bIsGuard = Monster->IsGuardOn();
	bUseGuardBase = bIsGuard;
	bIsInCombat = Monster->IsInCombat();
	bIsAttacking = Monster->IsAttacking();
	bIsRunning = Monster->IsRunning();
	bUseRunLocomotion = Monster->ShouldUseRunLocomotion();
}
