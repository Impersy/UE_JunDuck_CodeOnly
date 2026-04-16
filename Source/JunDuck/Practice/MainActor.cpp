


#include "MainActor.h"
#include "JunActor.h"

// Sets default values
AMainActor::AMainActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//// 블루프린트 클래스는 경로 입력시 마지막에 _C 를 붙여줘야함
	//ConstructorHelpers::FClassFinder<AJunActor> FindClass(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/BP_JunActor.BP_JunActor_C'"));
	//if (FindClass.Succeeded())
	//{
	//	ActorClass = FindClass.Class;
	//}

}

// Called when the game starts or when spawned
void AMainActor::BeginPlay()
{
	Super::BeginPlay();
	
	FVector Location(0,0,0);
	FRotator Rotation(0,0,0);

	//기본 클래스 엑터 스폰/파괴 함수
	//Actor = GetWorld()->SpawnActor<AJunActor>(Location,Rotation);
	//GetWorld()->DestroyActor(Actor);

	// Cast를 활용해서 대입
	Actor = Cast<AJunActor>(GetWorld()->SpawnActor(ActorClass));
	//Actor->SetLifeSpan(5.0f);
}

// Called every frame
void AMainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



}

