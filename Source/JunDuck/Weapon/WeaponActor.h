

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

UCLASS()
class JUNDUCK_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	FORCEINLINE UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

public:
	UFUNCTION(BlueprintCallable)
	void StartAttackTrace();

	UFUNCTION(BlueprintCallable)
	void EndAttackTrace();

	UFUNCTION(BlueprintCallable)
	void StopAttackTrace();

protected:
	void UpdateAttackTrace();

	void ApplyDamageToHitCharacter(AActor* HitActor, const FVector& SwingDirection);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace")
	USceneComponent* TraceStartPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trace")
	USceneComponent* TraceEndPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	int32 TraceSampleCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel2;

	bool bTraceActive = false;

	FVector PrevTraceStart = FVector::ZeroVector;
	FVector PrevTraceEnd = FVector::ZeroVector;

	TSet<AActor*> HitActors;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BasicDamage = 1;
};
