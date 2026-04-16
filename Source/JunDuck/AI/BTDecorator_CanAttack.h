

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanAttack.generated.h"

/**
 * 
 */
UCLASS()
class JUNDUCK_API UBTDecoTargetKeyrator_CanAttack : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecoTargetKeyrator_CanAttack();


protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere)
	float AttackRange = 200.f;

};
