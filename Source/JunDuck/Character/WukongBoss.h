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
	JumpAttack,
	ChargeAttack,
	DodgeAttack,
	ExtraAttack,
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
enum class EWukongExtraAttackType : uint8
{
	None,
	Ambush,
	Heavy,
	Spin,
	NinjaA,
	NinjaB,
	Execution
};

USTRUCT(BlueprintType)
struct FWukongExtraAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	TObjectPtr<class UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	float MaxRange = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	bool bFaceTargetDuringAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	float FacingDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	float FacingInterpSpeed = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	bool bTryTurnAfterAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wukong|ExtraAttack")
	float PostAttackTurnStartAngle = 45.f;
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
	EWukongExtraAttackType ExtraAttackType = EWukongExtraAttackType::None;
};

UENUM()
enum class EWukongPlannedAttackType : uint8
{
	None,
	ComboAttack,
	JumpAttack,
	ChargeAttack,
	DodgeAttack,
	ExtraAttack
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
	virtual bool ShouldStartHitReact(EHitReactType IncomingHitReact) const override;

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

	bool CanUseChargeAttack() const;
	bool CanUseJumpAttack() const;
	bool CanStartWukongTurn() const;
	bool HasAnyAttackCandidateForCurrentDistance() const;
	bool HasValidPlannedAttack() const;
	bool HasValidPlannedMovement() const;
	bool IsWithinSelectionRange(const FMonsterAttackSelection& Selection) const;
	const FWukongExtraAttackData* GetExtraAttackData(EWukongExtraAttackType AttackType) const;
	UAnimMontage* GetPlannedComboMontage() const;
	float GetPlannedComboFacingDuration() const;
	UAnimMontage* GetPlannedNonAttackMontage() const;
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
	void RefreshPlannedNonAttackType();
	void RefreshNoAttackFallbackMovementType();
	bool TryPlanComboAttackOnly(int32 MinComboLengthExclusive = 0);
	void ApplyJumpAttackGroundMotionOverride();
	void RestoreJumpAttackGroundMotionOverride();
	void ExecuteJumpAttackLaunch();
	void RecordAction(EWukongActionType ActionType, EWukongComboSet ComboSet = EWukongComboSet::None, int32 ComboLength = 0, EWukongExtraAttackType ExtraAttackType = EWukongExtraAttackType::None);
	bool WasRecentlyUsed(EWukongActionType ActionType, int32 Depth) const;
	bool WasRecentlyUsed(EWukongComboSet ComboSet, int32 ComboLength, int32 Depth) const;
	bool WasRecentlyUsed(EWukongExtraAttackType AttackType, int32 Depth) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> ChargeAttackMontage = nullptr;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> JumpAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> DodgeAttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ExtraAttack")
	FWukongExtraAttackData AmbushAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ExtraAttack")
	FWukongExtraAttackData HeavyAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ExtraAttack")
	FWukongExtraAttackData SpinAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ExtraAttack")
	FWukongExtraAttackData NinjaAAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ExtraAttack")
	FWukongExtraAttackData NinjaBAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ExtraAttack")
	FWukongExtraAttackData ExecutionAttack;

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
	float ChargeAttackRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackMinRange = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|BasicAttack")
	float BasicAttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMaxRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackStartDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchForwardSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackTargetStandOffDistance = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackAirTrackInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|DodgeAttack")
	float DodgeAttackMinRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|DodgeAttack")
	float DodgeAttackRange = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|DodgeAttack")
	float DodgeAttackFacingDuration = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|DodgeAttack")
	float DodgeAttackFacingInterpSpeed = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|DodgeAttack")
	float DodgeAttackTurnReacquireAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|DodgeAttack", meta = (ClampMin = "1", ClampMax = "10"))
	int32 DodgeAttackSelectionWeight = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Recovery")
	float RecoveryDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Recovery")
	float ComboDashFollowUpFallbackDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Plan", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackPlanWeight = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Approach")
	float ApproachComeHereDelay = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMinDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMoveInputStrength = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeMoveSpeedScale = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Movement")
	float StrafeAnimInputStrength = 0.3f;

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
	EWukongExtraAttackType PlannedExtraAttackType = EWukongExtraAttackType::None;


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
	EWukongExtraAttackType CurrentAttackExtraAttackType = EWukongExtraAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bCurrentAttackStartedFromComboDashFollowUp = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bPendingComboDashFollowUp = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float PendingComboDashFollowUpDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 PendingComboDashFollowUpMinComboLengthExclusive = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bApproachComeHereTriggered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bApproachComeHereInProgress = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float ApproachComeHereRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bDiscardPlannedAttackAfterApproachComeHere = false;

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
