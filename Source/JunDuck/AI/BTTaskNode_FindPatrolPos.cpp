


#include "AI/BTTaskNode_FindPatrolPos.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"

UBTTaskNode_FindPatrolPos::UBTTaskNode_FindPatrolPos()
{
	NodeName = TEXT("Find Patrol Position");
}

EBTNodeResult::Type UBTTaskNode_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld());
	if (!NavSystem)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomLocation;
	const FVector Origin = ControlledPawn->GetActorLocation();

	const bool bFound = NavSystem->GetRandomReachablePointInRadius(
		Origin,
		PatrolRadius,
		RandomLocation
	);

	if (!bFound)
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsVector(PatrolPosKey.SelectedKeyName, RandomLocation.Location);

	return EBTNodeResult::Succeeded;
}
