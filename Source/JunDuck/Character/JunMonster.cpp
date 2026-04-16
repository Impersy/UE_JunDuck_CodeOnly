#include "Character/JunMonster.h"
#include "AI/JunAIController.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "JunGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Weapon/WeaponActor.h"

// Engine lifecycle / external API

AJunMonster::AJunMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐 충돌 프리셋
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("EnemyCapsule"));

	// 메쉬 기본 위치 및 회전 조정
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -88.f), FRotator(0.f, -90.f, 0.f));

	AIControllerClass = AJunAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 컨트롤러에 기록된 회전을 따라서 캐릭터가 같이 회전되는 옵션 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 몸체가 이동 방향에 맞춰서 회전하게끔함
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
}

void AJunMonster::BeginPlay()
{
	Super::BeginPlay();

	// 팀 설정
	SetTeamTag(JunGameplayTags::Team_Enemy);

	SpawnAndAttachWeapon();

	HomeLocation = GetActorLocation();

	if (bStartWithPatrol)
	{
		SetMonsterState(EMonsterState::Patrol);
	}
	else
	{
		SetMonsterState(EMonsterState::Idle);
	}
}

void AJunMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetCharacterMovement()->MaxWalkSpeed = GetDesiredMaxWalkSpeed();
	bIsRunning = ShouldUseRunLocomotion();
	UpdateAttack(DeltaTime);

	// 몬스터는 "상위 상태 업데이트"와 "피격 리액션 타이머"를 매 프레임 병렬로 관리한다.
	StateTime += DeltaTime;
	Update_State(DeltaTime);
	UpdateHitReact(DeltaTime);
}

void AJunMonster::HandlePatrolMoveCompleted(bool bSuccess)
{
	if (CurrentState == EMonsterState::Return)
	{
		bIsSearching = true;

		// 수색 시간 시작
		SetMonsterState(EMonsterState::Idle);
		return;
	}

	if (CurrentState != EMonsterState::Patrol)
	{
		return;
	}

	// 플레이어 발견 우선
	TryFindTarget();

	if (CurrentTarget)
	{
		SetMonsterState(EMonsterState::Chase);
		return;
	}

	// 성공/실패 상관없이 다음 순찰점 갱신
	ChooseNextPatrolLocation();
	MoveToLocation(PatrolTargetLocation);
}

void AJunMonster::ReceiveHit(EHitReactType HitType, float DamageAmount, AActor* DamageCauser)
{
	// 기존 호출부 호환용 경로.
	// 방향 정보가 없으면 공격자 위치 기반의 대체 방향을 만들어 아래 오버로드로 넘긴다.
	const FVector FallbackSwingDirection = DamageCauser
		? (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal()
		: GetActorForwardVector();

	ReceiveHit(HitType, DamageAmount, DamageCauser, FallbackSwingDirection);
}

void AJunMonster::ReceiveHit(EHitReactType HitType, float DamageAmount, AActor* DamageCauser, const FVector& SwingDirection)
{
	if (CurrentState == EMonsterState::Dead)
	{
		return;
	}

	// 데미지는 먼저 적용하고, 그 다음 "새 피격 리액션으로 갈아탈 수 있는지"를 판단한다.
	AJunCharacter* AttackerCharacter = Cast<AJunCharacter>(DamageCauser);

	// 데미지 적용 (기존 시스템 활용)
	OnDamaged(FMath::RoundToInt(DamageAmount), AttackerCharacter);

	// 사망 체크
	if (Is_Dead())
	{
		CurrentHitReact = EHitReactType::Dead;
		SetMonsterState(EMonsterState::Dead);
		return;
	}

	// 인터럽트 가능 여부
	if (!CanBeInterruptedBy(HitType))
	{
		return;
	}

	// 이미 HitReact 중일 때 우선순위 간단 처리
	if (IsInHitReact())
	{
		// Heavy > Light 이건 임시 처리
		if (CurrentHitReact == EHitReactType::HeavyHit && HitType == EHitReactType::LightHit)
		{
			return;
		}
	}

	// 피격 반응 선택은 "누가 때렸는가"보다 "무기가 어느 방향으로 지나갔는가"가 더 중요하다.
	// 그래서 데미지 적용 뒤에 SwingDirection 기반으로 좌/우/정면 반응을 다시 판정한다.
	StartHitReact(HitType, DetermineHitReactDirection(SwingDirection));
}

bool AJunMonster::HasCombatTarget()
{
	return bIsHasTarget;
}

bool AJunMonster::IsGuardOn() const
{
	return bIsGuardOn;
}

bool AJunMonster::IsInCombat()
{
	return CurrentState == EMonsterState::Combat;
}

bool AJunMonster::IsAttacking()
{
	return bIsAttacking;
}

bool AJunMonster::IsRunning() const
{
	return bIsRunning;
}

bool AJunMonster::ShouldUseRunLocomotion() const
{
	const EMonsterMoveState MoveState = GetMoveState();
	return MoveState == EMonsterMoveState::Run || MoveState == EMonsterMoveState::Sprint;
}

EMonsterState AJunMonster::GetCurrentState() const
{
	return CurrentState;
}

EMonsterMoveState AJunMonster::GetMoveState() const
{
	if (bIsGuardOn)
	{
		return EMonsterMoveState::Guard;
	}

	if (bSprintRequested)
	{
		return EMonsterMoveState::Sprint;
	}

	return EMonsterMoveState::Walk;
}

FVector2D AJunMonster::GetCombatMoveInput() const
{
	return CombatMoveInput;
}

float AJunMonster::GetDesiredMoveForward() const
{
	return DesiredMoveForward;
}

float AJunMonster::GetDesiredMoveRight() const
{
	return DesiredMoveRight;
}

float AJunMonster::GetDesiredMaxWalkSpeed() const
{
	switch (GetMoveState())
	{
	case EMonsterMoveState::Guard:
		return GuardMoveSpeed;
	case EMonsterMoveState::Sprint:
		return SprintMoveSpeed;
	case EMonsterMoveState::Run:
		return RunMoveSpeed;
	case EMonsterMoveState::Walk:
	default:
		return WalkMoveSpeed;
	}
}

void AJunMonster::SetDesiredMoveAxes(float NewForward, float NewRight)
{
	DesiredMoveForward = FMath::Clamp(NewForward, -1.f, 1.f);
	DesiredMoveRight = FMath::Clamp(NewRight, -1.f, 1.f);
}

void AJunMonster::BeginAttackTraceWindow()
{
	if (!EquippedWeapon)
	{
		return;
	}

	EquippedWeapon->StartAttackTrace();
}

void AJunMonster::EndAttackTraceWindow()
{
	if (!EquippedWeapon)
	{
		return;
	}

	EquippedWeapon->EndAttackTrace();
}

// Top-level state machine

void AJunMonster::SetMonsterState(EMonsterState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Monster State Change: %d -> %d"),
		(int32)CurrentState, (int32)NewState);

	CurrentState = NewState;
	StateTime = 0.f;

	switch (CurrentState)
	{
	case EMonsterState::Idle:
		EnterIdleState();
		break;
	case EMonsterState::Patrol:
		EnterPatrolState();
		break;
	case EMonsterState::Chase:
		EnterChaseState();
		break;
	case EMonsterState::BattleStart:
		EnterBattleStartState();
		break;
	case EMonsterState::Combat:
		EnterCombatState();
		break;
	case EMonsterState::Dead:
		EnterDeadState();
		break;
	case EMonsterState::Return:
		EnterReturnState();
		break;
	default:
		break;
	}
}

void AJunMonster::EnterIdleState()
{
	bIsHasTarget = false;
	bSprintRequested = false;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	StopAIMovement();
	SetDesiredMoveAxes(0.f, 0.f);
}

void AJunMonster::EnterPatrolState()
{
	bIsHasTarget = false;
	bSprintRequested = false;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	ChooseNextPatrolLocation();
	MoveToLocation(PatrolTargetLocation);
}

void AJunMonster::EnterChaseState()
{
	bIsHasTarget = false;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bSprintRequested = false;

	if (CurrentTarget)
	{
		MoveToTarget(CurrentTarget);
	}
}

void AJunMonster::EnterBattleStartState()
{
	bIsHasTarget = (CurrentTarget != nullptr);
	bSprintRequested = false;
	CombatMoveCooldown = 0.f;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	StopAIMovement();
	SetDesiredMoveAxes(0.f, 0.f);
}

void AJunMonster::EnterReturnState()
{
	bIsHasTarget = false;
	bSprintRequested = false;
	CombatMoveCooldown = 0.f;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	MoveToLocation(LastKnownTargetLocation);
}

void AJunMonster::EnterCombatState()
{
	bIsHasTarget = (CurrentTarget != nullptr);
	bSprintRequested = false;
	CombatMoveCooldown = 0.f;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	StopAIMovement();
}

void AJunMonster::EnterDeadState()
{
	bIsHasTarget = false;
	bSprintRequested = false;
	CombatMoveCooldown = 0.f;
	CombatMoveInput = FVector2D::ZeroVector;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	AddGameplayTag(JunGameplayTags::State_Condition_Dead);
	StopAIMovement();
	SetDesiredMoveAxes(0.f, 0.f);

	// 필요하면 죽음 몽타주 / collision 조정 / AI 비활성화
}

void AJunMonster::Update_State(float DeltaTime)
{
	// ControlLocked/Dead 같은 상위 조건이 걸리면 일반 AI 상태 갱신은 멈춘다.
	if (Is_Dead() && CurrentState != EMonsterState::Dead)
	{
		SetMonsterState(EMonsterState::Dead);
		return;
	}

	if (!CanUpdateBehavior())
	{
		return;
	}

	switch (CurrentState)
	{
	case EMonsterState::Idle:
		UpdateIdle(DeltaTime);
		break;
	case EMonsterState::Patrol:
		UpdatePatrol(DeltaTime);
		break;
	case EMonsterState::Chase:
		UpdateChase(DeltaTime);
		break;
	case EMonsterState::BattleStart:
		UpdateBattleStart(DeltaTime);
		break;
	case EMonsterState::Combat:
		UpdateCombat(DeltaTime);
		break;
	case EMonsterState::Return:
		UpdateReturn(DeltaTime);
		break;
	case EMonsterState::Dead:
	default:
		break;
	}
}

void AJunMonster::UpdateIdle(float DeltaTime)
{
	TryFindTarget();

	if (CurrentTarget)
	{
		bIsSearching = false;
		SetMonsterState(EMonsterState::Chase);
		return;
	}

	if (bIsSearching)
	{
		if (StateTime >= SearchWaitTime)
		{
			bIsSearching = false;
			SetMonsterState(EMonsterState::Patrol);
		}
	}
	else
	{
		if (bStartWithPatrol && StateTime >= IdleToPatrolDelay)
		{
			SetMonsterState(EMonsterState::Patrol);
		}
	}
}

void AJunMonster::UpdatePatrol(float DeltaTime)
{
	TryFindTarget();

	if (CurrentTarget)
	{
		SetMonsterState(EMonsterState::Chase);
		return;
	}
}

void AJunMonster::UpdateChase(float DeltaTime)
{
	if (!CurrentTarget)
	{
		SetMonsterState(EMonsterState::Patrol);
		return;
	}

	// 유지 범위 체크 (DetectRange 아님)
	if (!CanKeepTarget(CurrentTarget))
	{
		LastKnownTargetLocation = CurrentTarget->GetActorLocation();
		CurrentTarget = nullptr;
		bIsHasTarget = false;

		SetMonsterState(EMonsterState::Return);
		return;
	}

	const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

	// Chase는 BattleStart 범위까지 확실히 붙는 역할만 담당한다.
	// 전투 시작 연출이나 전투 이동은 여기서 하지 않고, BattleStart / Combat으로 넘긴다.
	if (DistToTarget <= AIData.BattleStartRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enter BattleStart Range"));
		StopAIMovement();
		SetMonsterState(EMonsterState::BattleStart);
		return;
	}

	// 추적 유지 (너무 자주 호출 방지용 조건 추가)
	const float MoveThreshold = 50.f;

	if (FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) > MoveThreshold)
	{
		MoveToTarget(CurrentTarget);
	}
}

void AJunMonster::UpdateBattleStart(float DeltaTime)
{
	if (!CurrentTarget)
	{
		SetMonsterState(EMonsterState::Patrol);
		return;
	}

	if (!CanRemainInCombat(CurrentTarget))
	{
		LastKnownTargetLocation = CurrentTarget->GetActorLocation();
		CurrentTarget = nullptr;
		bIsHasTarget = false;
		SetMonsterState(EMonsterState::Return);
		return;
	}

	// BattleStart는 전투 시작 연출/준비용 짧은 중간 상태다.
	// 실제 공격/전투 이동은 아직 하지 않고, 바라보기와 대기만 담당한다.
	UpdateCombatFacing(DeltaTime);

	if (StateTime >= BattleStartDuration)
	{
		SetMonsterState(EMonsterState::Combat);
	}
}

void AJunMonster::UpdateReturn(float DeltaTime)
{
	// 이동 중에도 타겟 발견 가능
	TryFindTarget();

	if (CurrentTarget)
	{
		SetMonsterState(EMonsterState::Chase);
		return;
	}
}

void AJunMonster::UpdateCombat(float DeltaTime)
{
	if (!CurrentTarget)
	{
		SetMonsterState(EMonsterState::Patrol);
		return;
	}

	if (!CanRemainInCombat(CurrentTarget))
	{
		LastKnownTargetLocation = CurrentTarget->GetActorLocation();
		CurrentTarget = nullptr;
		bIsHasTarget = false;
		CombatMoveInput = FVector2D::ZeroVector;
		SetMonsterState(EMonsterState::Return);
		return;
	}

	// Combat는 타겟 유지, 전투용 회전, 전투용 이동만 담당한다.
	// 공격 여부 결정은 TryAttack 같은 별도 단계로 분리해서 붙이기 쉽게 둔다.
	UpdateCombatFacing(DeltaTime);

	if (CanAttackTarget())
	{
		TryAttack();
		return;
	}

	UpdateCombatMovement(DeltaTime);
}

// Combat helpers

void AJunMonster::UpdateCombatFacing(float DeltaTime)
{
	if (!CurrentTarget)
	{
		return;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator TargetRotation = ToTarget.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		CombatFacingInterpSpeed
	);

	SetActorRotation(NewRotation);
}

void AJunMonster::UpdateCombatMovement(float DeltaTime)
{
	CombatMoveCooldown -= DeltaTime;
	if (CombatMoveCooldown <= 0.f)
	{
		ChooseCombatMoveInput();
		CombatMoveCooldown = AIData.CombatMoveInterval;
	}

	float ForwardMoveInput = CombatMoveInput.X;
	if (CurrentTarget && ForwardMoveInput > 0.f)
	{
		const float DistToTarget = FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());
		if (DistToTarget <= AIData.AttackRange)
		{
			ForwardMoveInput = 0.f;
		}
	}

	if (ForwardMoveInput != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), ForwardMoveInput);
	}

	if (CombatMoveInput.Y != 0.f)
	{
		AddMovementInput(GetActorRightVector(), CombatMoveInput.Y);
	}
}

void AJunMonster::ChooseCombatMoveInput()
{
	static const TArray<FVector2D> CandidateInputs =
	{
		FVector2D(1.f, 0.f),
		FVector2D(-1.f, 0.f),
		FVector2D(0.f, 1.f),
		FVector2D(0.f, -1.f)
	};

	const int32 Index = FMath::RandRange(0, CandidateInputs.Num() - 1);
	CombatMoveInput = CandidateInputs[Index];
}

// AI / target helpers

void AJunMonster::TryFindTarget()
{
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);

	if (!PlayerActor)
	{
		return;
	}

	AJunCharacter* TargetCharacter = Cast<AJunCharacter>(PlayerActor);

	if (!TargetCharacter)
	{
		return;
	}

	const bool bEnemy = IsEnemyTo(TargetCharacter);
	if (!bEnemy)
	{
		return;
	}

	const bool bDetect = CanDetectTarget(TargetCharacter);
	if (!bDetect)
	{
		return;
	}

	CurrentTarget = TargetCharacter;
}

bool AJunMonster::CanDetectTarget(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	return DistSq <= FMath::Square(AIData.DetectRange);
}

bool AJunMonster::CanKeepTarget(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	return DistSq <= FMath::Square(AIData.LoseTargetRange);
}

bool AJunMonster::CanRemainInCombat(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	const AJunCharacter* TargetCharacter = Cast<AJunCharacter>(Target);
	if (TargetCharacter && TargetCharacter->Is_Dead())
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	return DistSq <= FMath::Square(AIData.CombatBreakRange);
}

bool AJunMonster::CanAttackTarget() const
{
	if (CurrentState != EMonsterState::Combat)
	{
		return false;
	}

	if (!CurrentTarget || !AttackMontage)
	{
		return false;
	}

	if (bIsAttacking || IsInHitReact() || Is_Dead())
	{
		return false;
	}

	if (AttackCooldownRemainTime > 0.f)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared2D(GetActorLocation(), CurrentTarget->GetActorLocation());
	return DistSq <= FMath::Square(AIData.AttackRange);
}

void AJunMonster::ChooseNextPatrolLocation()
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon)
	{
		PatrolTargetLocation = HomeLocation;
		return;
	}

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn)
	{
		PatrolTargetLocation = HomeLocation;
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld());
	if (!NavSystem)
	{
		PatrolTargetLocation = HomeLocation;
		return;
	}

	FNavLocation RandomLocation;
	const bool bFound = NavSystem->GetRandomReachablePointInRadius(
		HomeLocation,
		AIData.PatrolRadius,
		RandomLocation
	);

	if (!bFound)
	{
		PatrolTargetLocation = HomeLocation;
		return;
	}

	FNavLocation ProjectedLocation;
	const bool bProjected = NavSystem->ProjectPointToNavigation(
		RandomLocation.Location,
		ProjectedLocation,
		FVector(100.f, 100.f, 300.f)
	);

	PatrolTargetLocation = bProjected ? ProjectedLocation.Location : RandomLocation.Location;
}

void AJunMonster::MoveToLocation(const FVector& Dest)
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon)
	{
		return;
	}

	AICon->MoveToLocation(Dest, AIData.PatrolAcceptRadius);
}

void AJunMonster::MoveToTarget(AActor* Target)
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon || !Target)
	{
		return;
	}

	AICon->MoveToActor(Target, AIData.ChaseMoveAcceptanceRadius);
}

void AJunMonster::StartChase(AActor* NewTarget)
{
	if (!NewTarget)
	{
		return;
	}

	// 이미 같은 타겟이면 중복 전환 방지
	if (CurrentTarget == NewTarget && CurrentState == EMonsterState::Chase)
	{
		return;
	}

	CurrentTarget = NewTarget;
	bIsHasTarget = false;
	SetMonsterState(EMonsterState::Chase);
}

void AJunMonster::StopAIMovement()
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon)
	{
		return;
	}

	AICon->StopMovement();
}

void AJunMonster::TryAttack()
{
	if (!CanAttackTarget())
	{
		return;
	}

	bIsAttacking = true;
	AttackTime = AttackMontage ? AttackMontage->GetPlayLength() : DefaultAttackDuration;
	CombatMoveInput = FVector2D::ZeroVector;

	AddGameplayTag(JunGameplayTags::State_Condition_ControlLocked);
	AddGameplayTag(JunGameplayTags::State_Block_Move);

	StopAIMovement();
	SetDesiredMoveAxes(0.f, 0.f);

	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void AJunMonster::UpdateAttack(float DeltaTime)
{
	if (AttackCooldownRemainTime > 0.f)
	{
		AttackCooldownRemainTime = FMath::Max(0.f, AttackCooldownRemainTime - DeltaTime);
	}

	if (!bIsAttacking)
	{
		return;
	}

	// 공격 몽타주 중에도 타겟을 계속 바라보게 해서 추적감이 끊기지 않게 한다.
	if (CurrentTarget)
	{
		FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;

		if (!ToTarget.IsNearlyZero())
		{
			const FRotator CurrentRotation = GetActorRotation();
			const FRotator TargetRotation = ToTarget.Rotation();
			const FRotator NewRotation = FMath::RInterpTo(
				CurrentRotation,
				TargetRotation,
				DeltaTime,
				CombatFacingInterpSpeed
			);

			SetActorRotation(NewRotation);
		}
	}

	AttackTime -= DeltaTime;
	if (AttackTime <= 0.f)
	{
		FinishAttack();
	}
}

void AJunMonster::FinishAttack()
{
	if (!bIsAttacking)
	{
		return;
	}

	bIsAttacking = false;
	AttackTime = 0.f;
	AttackCooldownRemainTime = AttackCooldownDuration;

	RemoveGameplayTag(JunGameplayTags::State_Condition_ControlLocked);
	RemoveGameplayTag(JunGameplayTags::State_Block_Move);
}

// Weapon

void AJunMonster::SpawnAndAttachWeapon()
{
	// 몬스터 무기는 BeginPlay에서 한 번 스폰해서 소켓에 붙이고,
	// 공격 trace window 때만 활성화한다.
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (DefaultWeaponClass)
	{
		EquippedWeapon = World->SpawnActor<AWeaponActor>(
			DefaultWeaponClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (!EquippedWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn monster weapon."));
		}
		else
		{
			EquippedWeapon->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				WeaponSocketName
			);
		}
	}

	if (DefaultScabbardClass)
	{
		EquippedScabbard = World->SpawnActor<AWeaponActor>(
			DefaultScabbardClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (!EquippedScabbard)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn monster scabbard."));
		}
		else
		{
			EquippedScabbard->StopAttackTrace();
			EquippedScabbard->SetActorTickEnabled(false);
			EquippedScabbard->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				ScabbardSocketName
			);
		}
	}
}

// Hit react

void AJunMonster::StartHitReact(EHitReactType NewHitReact, ECharacterHitReactDirection NewHitDirection)
{
	// 피격 리액션은 "상태 태그 잠금 + AI 이동 정지 + 방향별 몽타주 재생"으로 시작한다.
	// 이 동안 일반 상태머신은 멈추고, Duration이 끝나면 다시 원래 AI 상태 갱신으로 돌아간다.
	CurrentHitReact = NewHitReact;
	CurrentHitReactDirection = NewHitDirection;
	HitReactTime = 0.f;

	AddGameplayTag(JunGameplayTags::State_Condition_HitReact);
	AddGameplayTag(JunGameplayTags::State_Condition_ControlLocked);

	StopAIMovement();

	if (UAnimMontage* HitReactMontage = GetHitReactMontage(NewHitReact, NewHitDirection))
	{
		PlayAnimMontage(HitReactMontage);
	}
}

void AJunMonster::UpdateHitReact(float DeltaTime)
{
	if (!IsInHitReact())
	{
		return;
	}

	// 현재는 몽타주 종료가 아니라 Duration 기준으로 HitReact 상태를 끝낸다.
	HitReactTime += DeltaTime;

	const float Duration = GetHitReactDuration(CurrentHitReact);
	if (HitReactTime >= Duration)
	{
		EndHitReact();
	}
}

void AJunMonster::EndHitReact()
{
	// HitReact 종료는 태그 해제만 담당하고,
	// 이후 AI 상태 갱신은 CanUpdateBehavior()가 다시 허용한다.
	CurrentHitReact = EHitReactType::None;
	CurrentHitReactDirection = ECharacterHitReactDirection::Front_F;
	HitReactTime = 0.f;

	RemoveGameplayTag(JunGameplayTags::State_Condition_HitReact);
	RemoveGameplayTag(JunGameplayTags::State_Condition_ControlLocked);
}

bool AJunMonster::IsInHitReact() const
{
	return CurrentHitReact != EHitReactType::None;
}

bool AJunMonster::CanBeInterruptedBy(EHitReactType IncomingHitReact) const
{
	if (CurrentState == EMonsterState::Dead)
	{
		return false;
	}

	if (HasGameplayTag(JunGameplayTags::State_Condition_SuperArmor))
	{
		return IncomingHitReact == EHitReactType::HeavyHit
			|| IncomingHitReact == EHitReactType::Airborne
			|| IncomingHitReact == EHitReactType::Knockdown;
	}

	return true;
}

float AJunMonster::GetHitReactDuration(EHitReactType HitType) const
{
	switch (HitType)
	{
	case EHitReactType::LightHit:
		return LightHitDuration;
	case EHitReactType::HeavyHit:
		return HeavyHitDuration;
	default:
		return 0.f;
	}
}

ECharacterHitReactDirection AJunMonster::DetermineHitReactDirection(const FVector& SwingDirection) const
{
	// 공격자 위치가 아니라 "칼이 지나간 방향"을 기준으로 좌/우/정면 반응을 고른다.
	// 정면에서 맞더라도 스윙이 좌->우인지 우->좌인지에 따라 다른 피격 애니를 고르기 위한 판정이다.
	const FVector SafeSwingDirection = SwingDirection.GetSafeNormal();
	if (SafeSwingDirection.IsNearlyZero())
	{
		return ECharacterHitReactDirection::Front_F;
	}

	const FVector LocalSwingDirection = GetActorTransform().InverseTransformVectorNoScale(SafeSwingDirection);
	const float SideValue = LocalSwingDirection.Y;
	const float ForwardValue = LocalSwingDirection.X;
	const float SideThreshold = 0.35f;

	if (FMath::Abs(SideValue) >= SideThreshold && FMath::Abs(SideValue) > FMath::Abs(ForwardValue))
	{
		return SideValue > 0.f
			? ECharacterHitReactDirection::Front_R
			: ECharacterHitReactDirection::Front_L;
	}

	return ECharacterHitReactDirection::Front_F;
}

UAnimMontage* AJunMonster::GetHitReactMontage(EHitReactType HitType, ECharacterHitReactDirection HitDirection) const
{
	// 테스트 단계에선 LightHit만 방향별 3개 몽타주를 사용한다.
	if (HitType != EHitReactType::LightHit)
	{
		return nullptr;
	}

	switch (HitDirection)
	{
	case ECharacterHitReactDirection::Front_L:
		return LightHitFront_LMontage;
	case ECharacterHitReactDirection::Front_R:
		return LightHitFront_RMontage;
	case ECharacterHitReactDirection::Front_F:
	default:
		return LightHitFront_FMontage;
	}
}

bool AJunMonster::CanUpdateBehavior() const
{
	if (CurrentState == EMonsterState::Dead)
	{
		return false;
	}

	if (HasGameplayTag(JunGameplayTags::State_Condition_ControlLocked))
	{
		return false;
	}

	return true;
}
