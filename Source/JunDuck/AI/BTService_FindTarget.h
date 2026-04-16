

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindTarget.generated.h"

/**
 * 
 */
UCLASS()
class JUNDUCK_API UBTService_FindTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_FindTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetKey;

	// 탐지 반경
	UPROPERTY(EditAnywhere, Category = "Search")
	float DetectRadius = 500.f;

	// 시야 반각 (ex: 45도면 총 90도 시야)
	UPROPERTY(EditAnywhere, Category = "Search")
	float HalfVisionAngle = 45.f;

	// LOS 체크 여부
	UPROPERTY(EditAnywhere, Category = "Search")
	bool bUseLineOfSight = true;

	// 디버그
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = true;

};
