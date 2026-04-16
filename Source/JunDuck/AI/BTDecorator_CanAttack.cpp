


#include "AI/BTDecorator_CanAttack.h"
#include "AI/JunAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Character/JunCharacter.h"


UBTDecoTargetKeyrator_CanAttack::UBTDecoTargetKeyrator_CanAttack()
{
	NodeName = TEXT("CanAttack");
}

bool UBTDecoTargetKeyrator_CanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();

	if (ControllingPawn == nullptr)
	{
		return false;
	}

	AJunCharacter* Target = Cast<AJunCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetKey.SelectedKeyName));

	if (Target == nullptr)
	{
		return false;
	}


	return Target->GetDistanceTo(ControllingPawn) <= AttackRange;
	
}
