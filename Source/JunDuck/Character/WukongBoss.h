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

UENUM()
enum class EWukongPlannedAttackType : uint8
{
	None,
	BasicCombo,
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
	bool IsWithinSelectionRange(const FMonsterAttackSelection& Selection) const;
	UAnimMontage* GetBasicComboMontage() const;
	float GetTargetYawAbsDelta() const;
	void RefreshPlannedAttackType();
	EWukongPlannedAttackType ChooseRandomSpecialAttackType() const;
	void ApplyJumpAttackGravityOverride();
	void RestoreJumpAttackGravityOverride();
	void ExecuteJumpAttackLaunch();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> ChargeAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> BasicAttack1Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> BasicAttack2Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> BasicAttack3Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> BasicAttack4Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wukong|Attack")
	TObjectPtr<class UAnimMontage> JumpAttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	bool bFaceTargetDuringChargeAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	bool bTryTurnAfterChargeAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackFacingInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackFacingDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackTurnReacquireAngle = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|ChargeAttack")
	float ChargeAttackRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|BasicAttack")
	float BasicAttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMinRange = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackMaxRange = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackStartDelay = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchZVelocity = 470.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchForwardSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackGravityScale = 26.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackTargetStandOffDistance = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Recovery")
	float RecoveryDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float TurnStartAngle = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wukong|Turn")
	float IdleEntryYawTolerance = 15.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongCombatState CurrentCombatSubState = EWukongCombatState::Approach;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	float CombatSubStateElapsedTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	int32 BasicComboIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	bool bUsedChargeAttackLast = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|Combat")
	EWukongPlannedAttackType PlannedAttackType = EWukongPlannedAttackType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	bool bJumpAttackGravityOverrideActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float DefaultGravityScale = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	bool bPendingJumpAttackLaunch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wukong|JumpAttack")
	float JumpAttackLaunchDelayRemainTime = 0.f;
};
