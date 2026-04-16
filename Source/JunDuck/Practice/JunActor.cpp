#include "JunActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/Gameplaystatics.h"

// Sets default values
AJunActor::AJunActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> FindMesh(TEXT("/Script/Engine.StaticMesh'/Game/_JunDuck/SM_Cube.SM_Cube'"));
	if (FindMesh.Succeeded())
	{
		Box->SetStaticMesh(FindMesh.Object);
	}*/
}

// Called when the game starts or when spawned
void AJunActor::BeginPlay()
{
	Super::BeginPlay();
	
	//UGameplayStatics::GetActorOfClass(GetWorld(), AJunActor::StaticClass());

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Enemy"), Actors);

	// Empty() 는 STL vector 기준 Clear() 임
	if (Actors.Num() > 0) 
	{
		Target = Actors[0];
	}

}

// Called every frame
void AJunActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Target != nullptr)
	{
		float Speed = 100.f;
		float Distance = Speed * DeltaTime;

		FVector Location = GetActorLocation();

		FVector Dir_Vec = Target->GetActorLocation() - Location;
		Dir_Vec.Normalize();

		/*FVector NewLocation = Location + Dir_Vec * Distance;
		SetActorLocation(NewLocation);*/

		// 방향벡터만 넘겨주면 이동시키는 함수
		AddActorWorldOffset(Dir_Vec * Distance);
	}
	
	
}

