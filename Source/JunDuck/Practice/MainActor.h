

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainActor.generated.h"

UCLASS()
class JUNDUCK_API AMainActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMainActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY()
	TObjectPtr<class AJunActor> Actor;

	// BP로 만들어진 서브클래스 같은게 올수있음
	UPROPERTY()
	TSubclassOf<AJunActor> ActorClass; 
};
