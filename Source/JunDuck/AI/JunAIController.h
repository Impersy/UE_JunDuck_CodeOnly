

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "JunAIController.generated.h"

/**
 * 
 */
UCLASS()
class JUNDUCK_API AJunAIController : public AAIController
{
	GENERATED_BODY()

public:
	AJunAIController(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;



};
