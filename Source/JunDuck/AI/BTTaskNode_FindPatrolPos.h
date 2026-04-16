

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_FindPatrolPos.generated.h"

/**
 * 
 */
UCLASS()
class JUNDUCK_API UBTTaskNode_FindPatrolPos : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_FindPatrolPos();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 순찰 위치를 저장할 블랙보드 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolPosKey;

	// 현재 위치 기준 탐색 반경
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 800.f;

};
