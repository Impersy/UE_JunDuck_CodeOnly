


#include "AI/BTService_FindTarget.h"
#include "AI/JunAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UBTService_FindTarget::UBTService_FindTarget()
{
	NodeName = TEXT("FindTargetService");
	Interval = 0.2f;
}

void UBTService_FindTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	APawn* OwnerPawn = AICon->GetPawn();
	if (!OwnerPawn) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	const FVector MyLocation = OwnerPawn->GetActorLocation();
	const FVector Forward = OwnerPawn->GetActorForwardVector();

	// 반경 내 후보 찾기
	TArray<AActor*> OverlappedActors;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(OwnerPawn);

	UKismetSystemLibrary::SphereOverlapActors(
		OwnerPawn,
		MyLocation,
		DetectRadius,
		ObjectTypes,
		APawn::StaticClass(),
		IgnoreActors,
		OverlappedActors
	);

	// 각도 기준
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfVisionAngle));

	AActor* BestTarget = nullptr;
	float BestDistSq = MAX_flt;

	for (AActor* Candidate : OverlappedActors)
	{
		if (!Candidate) continue;

		// 방향 계산
		FVector ToTarget = Candidate->GetActorLocation() - MyLocation;

		// 수평 기준 부채꼴 (Z 제거)
		ToTarget.Z = 0.f;

		if (ToTarget.IsNearlyZero()) continue;

		ToTarget.Normalize();

		const float Dot = FVector::DotProduct(Forward, ToTarget);

		// 각도 밖이면 제외
		if (Dot < CosHalfAngle)
		{
			continue;
		}

		// LOS 체크
		if (bUseLineOfSight)
		{
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(OwnerPawn);

			const bool bBlocked = OwnerPawn->GetWorld()->LineTraceSingleByChannel(
				Hit,
				MyLocation,
				Candidate->GetActorLocation(),
				ECC_Visibility,
				Params
			);

			if (bBlocked && Hit.GetActor() != Candidate)
			{
				continue;
			}
		}

		// 거리 기준으로 가장 가까운 타겟 선택
		const float DistSq = FVector::DistSquared(MyLocation, Candidate->GetActorLocation());

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}

		// 디버그
		if (bDrawDebug)
		{
			DrawDebugLine(GetWorld(), MyLocation, Candidate->GetActorLocation(), FColor::Green, false, 0.1f, 0, 1.f);
		}
	}

	// Blackboard에 저장(타겟 없으면 초기값인 nullptr로 들어감)
	BB->SetValueAsObject(TargetKey.SelectedKeyName, BestTarget);

	// 디버그: 시야 원
	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), MyLocation, DetectRadius, 16, FColor::Blue, false, 0.1f);
	}
}
