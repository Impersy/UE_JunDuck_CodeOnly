#pragma once

#include "CoreMinimal.h"
#include "Character/JunMonster.h"
#include "WukongBoss.generated.h"

UENUM(BlueprintType)
enum class EWukongCombatState : uint8
{
	Approach,
	Idle,
	Attack,
	Recovery,
	Reposition,
	Evade,
	Turn
};

UENUM(BlueprintType)
enum class EWukongActionType : uint8
{
	None,
	ComboAttack,
	JumpAttack,
	ChargeAttack,
	Dash,
	Dodge,
	Turn
};

UENUM(BlueprintType)
enum class EWukongPlannedActionType : uint8
{
	None,
	Attack,
	Movement
};

UENUM(BlueprintType)
enum class EWukongPlannedMovementType : uint8
{
	None,
	Strafe,
	Dash,
	Dodge
};

UENUM(BlueprintType)
enum class EWukongMovementDirection : uint8
{
	None,
	Forward,
	Backward,
	Left,
	Right
};

UENUM(BlueprintType)
enum class EWukongComboSet : uint8
{
	None,
	ComboA,
	ComboB
};

USTRUCT(BlueprintType)
struct FWukongActionRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|Action")
	EWukongActionType ActionType = EWukongActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|Action")
	EWukongComboSet ComboSet = EWukongComboSet::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|Action")
	int32 ComboLength = 0;
};

UENUM()
enum class EWukongPlannedAttackType : uint8
{
	None,
	ComboAttack,
	JumpAttack,
	ChargeAttack
};

UCLASS()
class JUNDUCK_API AWukongBoss : public AJunMonster
{
	GENERATED_BODY()

public:
	AWukongBoss();

protected:
	virtual void EnterCombatState() override;
	virtual void UpdateCombat(float DeltaTime) override;
	virtual FMonsterAttackSelection ChooseNextAttackSelection() const override;
	virtual void OnAttackTick(float DeltaTime) override;
	virtual FVector GetLockOnTargetPoint() const override;

	void SetWukongCombatState(EWukongCombatState NewState);
	void EnterApproachState();
	void EnterIdleState();
	void EnterAttackState();
	void EnterRecoveryState();
	void EnterRepositionState();
	void EnterEvadeState();
	void EnterTurnState();

	void UpdateApproachState(float DeltaTime);
	void UpdateIdleState(float DeltaTime);
	void UpdateAttackState(float DeltaTime);
	void UpdateRecoveryState(float DeltaTime);
	void UpdateRepositionState(float DeltaTime);
	void UpdateEvadeState(float DeltaTime);
	void UpdateTurnState(float DeltaTime);

	bool CanUseChargeAttack() const;
	bool CanUseJumpAttack() const;
	bool CanStartWukongTurn() const;
	bool HasValidPlannedAttack() const;
	bool HasValidPlannedMovement() const;
	bool IsWithinSelectionRange(const FMonsterAttackSelection& Selection) const;
	UAnimMontage* GetPlannedComboMontage() const;
	float GetPlannedComboFacingDuration() const;
	UAnimMontage* GetPlannedMovementMontage() const;
	float GetTargetYawAbsDelta() const;
	float GetTargetDistance2D() const;
	float GetPlannedAttackMinRange() const;
	float GetPlannedAttackMaxRange() const;
	bool IsTargetTooFarForPlannedAttack() const;
	bool IsTargetTooCloseForPlannedAttack() const;
	void UpdateFacingTowardsTargetWithoutTurn(float DeltaTime, float InterpSpeed);
	void ResetPlannedCombatPlan();
	void EnsureCombatPlan();
	void RefreshPlannedCombatPlan();
	void RefreshPlannedAttackType();
	void RefreshPlannedMovementType();
	void ApplyJumpAttackGroundMotionOverride();
	void RestoreJumpAttackGroundMotionOverride();
	void ExecuteJumpAttackLaunch();
	void RecordAction(EWukongActionType ActionType, EWukongComboSet ComboSet = EWukongComboSet::None, int32 ComboLength = 0);
	bool WasRecentlyUsed(EWukongActionType ActionType, int32 Depth) const;
	bool WasRecentlyUsed(EWukongComboSet ComboSet, int32 ComboLength, int32 Depth) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> ChargeAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<TObjectPtr<class UAnimMontage>> ComboSetAMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<float> ComboSetAFacingDurations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<TObjectPtr<class UAnimMontage>> ComboSetBMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<float> ComboSetBFacingDurations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> JumpAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dash")
	TObjectPtr<class UAnimMontage> DashForwardMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dash")
	TObjectPtr<class UAnimMontage> DashBackwardMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dash")
	TObjectPtr<class UAnimMontage> DashLeftMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dash")
	TObjectPtr<class UAnimMontage> DashRightMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dodge")
	TObjectPtr<class UAnimMontage> DodgeForwardMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dodge")
	TObjectPtr<class UAnimMontage> DodgeBackwardMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dodge")
	TObjectPtr<class UAnimMontage> DodgeLeftMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Dodge")
	TObjectPtr<class UAnimMontage> DodgeRightMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	bool bFaceTargetDuringChargeAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	bool bTryTurnAfterChargeAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackFacingInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackFacingDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackTurnReacquireAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackMinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|BasicAttack")
	float BasicAttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMaxRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float ApproachRunStartDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float ApproachDodgeFallbackDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackStartDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchForwardSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackTargetStandOffDistance = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackAirTrackInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Recovery")
	float RecoveryDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Plan", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackPlanWeight = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMinDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMoveInputStrength = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float AttackBackOffDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float AttackBackOffMoveInputStrength = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float EvadeFallbackDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float TurnStartAngle = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float MoveTurnStartAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float ApproachFacingInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float IdleEntryYawTolerance = 15.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongCombatState CurrentCombatSubState = EWukongCombatState::Approach;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float CombatSubStateElapsedTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float CurrentRepositionDirection = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedActionType PlannedActionType = EWukongPlannedActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedAttackType PlannedAttackType = EWukongPlannedAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedAttackType DeferredAttackTypeAfterApproachDodge = EWukongPlannedAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedMovementType PlannedMovementType = EWukongPlannedMovementType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongMovementDirection PlannedMovementDirection = EWukongMovementDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float PlannedMovementDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongComboSet PlannedComboSet = EWukongComboSet::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongComboSet DeferredComboSetAfterApproachDodge = EWukongComboSet::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 PlannedComboLength = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 DeferredComboLengthAfterApproachDodge = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	TArray<FWukongActionRecord> RecentActionHistory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 MaxRecentActionHistory = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongActionType CurrentAttackActionType = EWukongActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongComboSet CurrentAttackComboSet = EWukongComboSet::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 CurrentAttackComboLength = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	bool bPendingJumpAttackLaunch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchDelayRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	bool bJumpAttackGroundMotionOverrideActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackGroundMotionOverrideRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float DefaultGroundFriction = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float DefaultBrakingDecelerationWalking = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float DefaultBrakingFrictionFactor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackGroundMotionOverrideDuration = 0.8f;
};
