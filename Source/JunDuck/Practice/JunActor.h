#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// 무조건 마지막에 나와야하는 헤더
#include "JunActor.generated.h"

class UJunObject;
class UStaticMeshComponent;

UCLASS()
class JUNDUCK_API AJunActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJunActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> Box;

	UPROPERTY(EditAnywhere, Category=Battle)
	TObjectPtr<class AActor> Target;
};
