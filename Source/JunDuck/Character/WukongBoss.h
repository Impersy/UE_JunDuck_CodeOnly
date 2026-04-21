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
	NonAttackAction,
	Evade,
	Turn
};

UENUM(BlueprintType)
enum class EWukongActionType : uint8
{
	None,
	ComboAttack,
	NormalAttack,
	Dash,
	Dodge,
	Turn
};

UENUM(BlueprintType)
enum class EWukongPlannedActionType : uint8
{
	None,
	Attack,
	NonAttack
};

UENUM(BlueprintType)
enum class EWukongPlannedNonAttackType : uint8
{
	None,
	Strafe,
	Dash,
	Dodge,
	ComeHere,
	Sleep,
	Boring,
	Hold
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

UENUM(BlueprintType)
enum class EWukongNormalAttackType : uint8
{
	None,
	JumpAttack,
	ChargeAttack,
	DodgeAttack,
	Ambush,
	Heavy,
	Spin,
	NinjaA,
	NinjaB,
	Execution
};

USTRUCT(BlueprintType)
struct FWukongNormalAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	TObjectPtr<class UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float MaxRange = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float CandidateMaxRange = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	bool bFaceTargetDuringAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float FacingDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float FacingInterpSpeed = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	bool bTryTurnAfterAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack")
	float PostAttackTurnStartAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|NormalAttack", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SelectionWeight = 1;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|Action")
	EWukongNormalAttackType NormalAttackType = EWukongNormalAttackType::None;
};

UENUM()
enum class EWukongPlannedAttackType : uint8
{
	None,
	ComboAttack,
	NormalAttack
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
	virtual float GetHitReactDuration(EHitReactType HitType) const override;
	virtual FString GetMonsterDebugExtraText() const override;

	void SetWukongCombatState(EWukongCombatState NewState);
	void EnterApproachState();
	void EnterIdleState();
	void EnterAttackState();
	void EnterRecoveryState();
	void EnterRepositionState();
	void EnterNonAttackActionState();
	void EnterEvadeState();
	void EnterTurnState();

	void UpdateApproachState(float DeltaTime);
	void UpdateIdleState(float DeltaTime);
	void UpdateAttackState(float DeltaTime);
	void UpdateRecoveryState(float DeltaTime);
	void UpdateRepositionState(float DeltaTime);
	void UpdateNonAttackActionState(float DeltaTime);
	void UpdateEvadeState(float DeltaTime);
	void UpdateTurnState(float DeltaTime);

	bool CanStartWukongTurn() const;
	bool HasAnyAttackCandidateForCurrentDistance() const;
	bool HasValidPlannedAttack() const;
	bool HasValidPlannedMovement() const;
	bool IsWithinSelectionRange(const FMonsterAttackSelection& Selection) const;
	const FWukongNormalAttackData* GetNormalAttackData(EWukongNormalAttackType AttackType) const;
	UAnimMontage* GetPlannedComboMontage() const;
	const FWukongNormalAttackData* GetPlannedNormalAttackData() const;
	float GetPlannedComboFacingDuration() const;
	UAnimMontage* GetPlannedNonAttackMontage() const;
	float GetTargetYawAbsDelta() const;
	float GetTargetDistance2D() const;
	float GetPlannedAttackMinRange() const;
	float GetPlannedAttackMaxRange() const;
	float GetAttackCandidateMaxRange(EWukongPlannedAttackType AttackType) const;
	bool IsTargetTooFarForPlannedAttack() const;
	bool IsTargetTooCloseForPlannedAttack() const;
	void UpdateFacingTowardsTargetWithoutTurn(float DeltaTime, float InterpSpeed);
	void ResetPlannedCombatPlan();
	void EnsureCombatPlan();
	void RefreshPlannedCombatPlan();
	void RefreshPlannedAttackType();
	void RefreshPlannedNonAttackType();
	void RefreshNoAttackFallbackMovementType();
	bool TryPlanComboAttackOnly(int32 MinComboLengthExclusive = 0);
	void ApplyAttackGroundMotionOverride(float Duration);
	void RestoreAttackGroundMotionOverride();
	void ExecuteJumpAttackLaunch();
	void ExecuteHeavyAttackMove();
	void RecordAction(EWukongActionType ActionType, EWukongComboSet ComboSet = EWukongComboSet::None, int32 ComboLength = 0, EWukongNormalAttackType NormalAttackType = EWukongNormalAttackType::None);
	bool WasRecentlyUsed(EWukongActionType ActionType, int32 Depth) const;
	bool WasRecentlyUsed(EWukongComboSet ComboSet, int32 ComboLength, int32 Depth) const;
	bool WasRecentlyUsed(EWukongNormalAttackType AttackType, int32 Depth) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Approach")
	TObjectPtr<class UAnimMontage> ComeHereMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Approach")
	TObjectPtr<class UAnimMontage> SleepMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Approach")
	TObjectPtr<class UAnimMontage> BoringMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<TObjectPtr<class UAnimMontage>> ComboSetAMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<float> ComboSetAFacingDurations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<TObjectPtr<class UAnimMontage>> ComboSetBMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Attack")
	TArray<float> ComboSetBFacingDurations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData JumpAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData ChargeAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData DodgeAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData AmbushAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData HeavyAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData SpinAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData NinjaAAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData NinjaBAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack")
	FWukongNormalAttackData ExecutionAttack;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|BasicAttack")
	float BasicAttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|BasicAttack")
	float BasicAttackCandidateRange = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMoveStartTimeAtPlayRateOne = 0.14f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchForwardSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackTargetStandOffDistance = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackAirTrackInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|HeavyAttack")
	float HeavyAttackMoveStartTimeAtPlayRateOne = 0.82f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|HeavyAttack")
	float HeavyAttackMoveSpeed = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|HeavyAttack")
	float HeavyAttackMoveStandOffDistance = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|HeavyAttack")
	float HeavyAttackGroundMotionOverrideDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Recovery")
	float RecoveryDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Recovery")
	float ComboDodgeFollowUpFallbackDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Plan", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackPlanWeight = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|NormalAttack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpinAttackSelectionChance = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Approach")
	float ApproachComeHereDelay = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Approach")
	float NoAttackCandidateApproachDelay = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMinDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMoveInputStrength = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMoveSpeedScale = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeAnimInputStrength = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeMovementPlanWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float YawMismatchRepositionChance = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float YawMismatchIdleHoldDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float AttackBackOffDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float AttackBackOffMoveInputStrength = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float EvadeFallbackDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float NoAttackHoldDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float TurnStartAngle = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float MoveTurnStartAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float ApproachFacingInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float EvadeFacingInterpSpeed = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|HitReact")
	float WukongLightHitDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|HitReact")
	float WukongLargeHitDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float IdleEntryYawTolerance = 15.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongCombatState CurrentCombatSubState = EWukongCombatState::Approach;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float CombatSubStateElapsedTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float CurrentRepositionDirection = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bUseIdleHoldForYawMismatch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedActionType PlannedActionType = EWukongPlannedActionType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedAttackType PlannedAttackType = EWukongPlannedAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongNormalAttackType PlannedNormalAttackType = EWukongNormalAttackType::None;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedNonAttackType PlannedNonAttackType = EWukongPlannedNonAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedNonAttackType LastCompletedNonAttackType = EWukongPlannedNonAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedNonAttackType NextExpressiveNonAttackType = EWukongPlannedNonAttackType::ComeHere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bShouldStartNoAttackFallbackWithStrafe = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongMovementDirection PlannedMovementDirection = EWukongMovementDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float PlannedMovementDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongComboSet PlannedComboSet = EWukongComboSet::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 PlannedComboLength = 0;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongNormalAttackType CurrentAttackNormalAttackType = EWukongNormalAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bCurrentAttackStartedFromComboDodgeFollowUp = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bPendingComboDodgeFollowUp = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float PendingComboDodgeFollowUpDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 PendingComboDodgeFollowUpMinComboLengthExclusive = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bApproachComeHereTriggered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bApproachComeHereInProgress = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float ApproachComeHereRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bDiscardPlannedAttackAfterApproachComeHere = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bNoAttackCandidateApproachInProgress = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bUseNonAttackFallbackUntilAttackCandidateAppears = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float NoAttackCandidateApproachRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	bool bPendingHeavyAttackMove = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	float HeavyAttackMoveDelayRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	bool bPendingJumpAttackLaunch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	float JumpAttackLaunchDelayRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	bool bAttackGroundMotionOverrideActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	float AttackGroundMotionOverrideRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	float DefaultGroundFriction = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	float DefaultBrakingDecelerationWalking = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|AttackMotion")
	float DefaultBrakingFrictionFactor = 0.f;
};
