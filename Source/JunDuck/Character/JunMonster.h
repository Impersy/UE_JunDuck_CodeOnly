

#pragma once

#include "CoreMinimal.h"
#include "Character/JunCharacter.h"
#include "JunMonster.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle,
	Patrol,
	Chase,
	Return,
	BattleStart,
	Combat,
	Dead
};

UENUM(BlueprintType)
enum class EMonsterMoveState : uint8
{
	Walk,
	Run,
	Sprint,
	Guard
};

USTRUCT(BlueprintType)
struct FMonsterAIData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float PatrolRadius = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float PatrolWaitTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float PatrolAcceptRadius = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float DetectRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float LoseTargetRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float ChaseMoveAcceptanceRadius = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float BattleStartRange = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float CombatBreakRange = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float CombatMoveInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MonsterAIData")
	float AttackRange = 250.f;
};

UCLASS()
class JUNDUCK_API AJunMonster : public AJunCharacter
{
	GENERATED_BODY()

public:
	// Engine / external entry points
	AJunMonster();
	virtual void Tick(float DeltaTime) override;

	// Called from AI/path following or weapon/damage systems.
	void HandlePatrolMoveCompleted(bool bSuccess);
	void ReceiveHit(EHitReactType HitType, float DamageAmount, AActor* DamageCauser);
	void ReceiveHit(EHitReactType HitType, float DamageAmount, AActor* DamageCauser, const FVector& SwingDirection);

	// Animation or external gameplay code queries these states.
	bool HasCombatTarget();
	bool IsGuardOn() const;
	bool IsInCombat();
	bool IsAttacking();
	bool IsRunning() const;
	bool ShouldUseRunLocomotion() const;
	EMonsterState GetCurrentState() const;
	EMonsterMoveState GetMoveState() const;
	FVector2D GetCombatMoveInput() const;
	float GetDesiredMoveForward() const;
	float GetDesiredMoveRight() const;
	float GetDesiredMaxWalkSpeed() const;
	void SetDesiredMoveAxes(float NewForward, float NewRight);

	virtual void BeginAttackTraceWindow() override;
	virtual void EndAttackTraceWindow() override;

protected:
	// Spawn-time setup: team, weapon, home position and initial top-level state.
	virtual void BeginPlay() override;

protected:
	// Top-level state machine.
	// Enter*() configures the state once, Update*() advances it every Tick.
	void SetMonsterState(EMonsterState NewState);
	void EnterIdleState();
	void EnterPatrolState();
	void EnterChaseState();
	void EnterBattleStartState();
	void EnterReturnState();
	void EnterCombatState();
	void EnterDeadState();

	void Update_State(float DeltaTime);
	void UpdateIdle(float DeltaTime);
	void UpdatePatrol(float DeltaTime);
	void UpdateChase(float DeltaTime);
	void UpdateBattleStart(float DeltaTime);
	void UpdateReturn(float DeltaTime);
	void UpdateCombat(float DeltaTime);
	void UpdateCombatFacing(float DeltaTime);
	void UpdateCombatMovement(float DeltaTime);
	void ChooseCombatMoveInput();
	void UpdateAttack(float DeltaTime);
	void FinishAttack();

protected:
	// AI / target helpers used by the state machine.
	void TryFindTarget();
	bool CanDetectTarget(AActor* Target) const;
	bool CanKeepTarget(AActor* Target) const;
	bool CanRemainInCombat(AActor* Target) const;
	bool CanAttackTarget() const;
	void ChooseNextPatrolLocation();
	void MoveToLocation(const FVector& Dest);
	void MoveToTarget(AActor* Target);
	void StartChase(AActor* NewTarget);
	void StopAIMovement();
	void TryAttack();

protected:
	// Weapon lifecycle helper.
	void SpawnAndAttachWeapon();

protected:
	// Hit react lifecycle.
	// Direction is decided from swing direction, not attacker location.
	void StartHitReact(EHitReactType NewHitReact, ECharacterHitReactDirection NewHitDirection);
	void UpdateHitReact(float DeltaTime);
	void EndHitReact();
	bool IsInHitReact() const;
	bool CanBeInterruptedBy(EHitReactType IncomingHitReact) const;
	float GetHitReactDuration(EHitReactType HitType) const;
	ECharacterHitReactDirection DetermineHitReactDirection(const FVector& SwingDirection) const;
	UAnimMontage* GetHitReactMontage(EHitReactType HitType, ECharacterHitReactDirection HitDirection) const;
	bool CanUpdateBehavior() const;

protected:
	// AI tuning / top-level state runtime
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FMonsterAIData AIData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EMonsterState CurrentState = EMonsterState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	float StateTime = 0.f; // 어떤 상태에 들어온 후 지난 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float BattleStartDuration = 0.5f;

protected:
	// Runtime references cached after spawn / detection.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<class AWeaponActor> EquippedWeapon = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<class AWeaponActor> EquippedScabbard = nullptr;

protected:
	// Patrol / return runtime data.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	FVector HomeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	FVector PatrolTargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	bool bStartWithPatrol = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	float IdleToPatrolDelay = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Return")
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Return")
	float SearchWaitTime = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Return")
	bool bIsSearching = false;

protected:
	// Combat / movement runtime flags and transient values.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsGuardOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackCooldownRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	bool bIsHasTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bSprintRequested = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CombatMoveCooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CombatFacingInterpSpeed = 8.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector2D CombatMoveInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DesiredMoveForward = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DesiredMoveRight = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkMoveSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RunMoveSpeed = 550.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintMoveSpeed = 725.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float GuardMoveSpeed = 250.f;

protected:
	// Test attack setup for simple "in range -> attack montage" behavior.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<class UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float DefaultAttackDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackCooldownDuration = 1.2f;

protected:
	// Designer-assigned weapon setup.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeaponActor> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeaponActor> DefaultScabbardClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("WeaponSocket_R");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName ScabbardSocketName = TEXT("ScabSocket");

protected:
	// Hit react runtime state, timing, and animation assets.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitReact")
	EHitReactType CurrentHitReact = EHitReactType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitReact")
	ECharacterHitReactDirection CurrentHitReactDirection = ECharacterHitReactDirection::Front_F;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitReact")
	float HitReactTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact")
	float LightHitDuration = 0.82f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact")
	float HeavyHitDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<class UAnimMontage> LightHitFront_FMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<class UAnimMontage> LightHitFront_LMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReact")
	TObjectPtr<class UAnimMontage> LightHitFront_RMontage = nullptr;
};
