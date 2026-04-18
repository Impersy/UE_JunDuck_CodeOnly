#pragma once

#include "CoreMinimal.h"
#include "Character/JunCharacter.h"
#include "JunPlayer.generated.h"

UENUM(BlueprintType)
enum class EJunCameraMode : uint8
{
	Free,
	LockOn
};

UENUM(BlueprintType)
enum class EJunDefenseState : uint8
{
	None = 0,
	Starting = 1,
	Guarding = 3,
	Ending = 4
};

UENUM(BlueprintType)
enum class EJunBufferedRecoveryAction : uint8
{
	None,
	Dodge,
	Jump,
	Defense
};

UENUM(BlueprintType)
enum class EJunBufferedDefenseCancelAction : uint8
{
	None,
	BasicAttack,
	Jump,
	Dodge,
	Move
};

UENUM(BlueprintType)
enum class EJunBufferedParrySuccessCancelAction : uint8
{
	None,
	BasicAttack,
	Jump,
	Dodge,
	Move
};

UENUM(BlueprintType)
enum class EJunMoveState : uint8
{
	Walk,
	Run,
	Sprint,
	Guard
};

UENUM(BlueprintType)
enum class EJunPlayerHitResolveResult : uint8
{
	Ignored,
	ParrySuccess,
	GuardBlock,
	HitReact
};

UENUM(BlueprintType)
enum class EJunPlayerHitState : uint8
{
	None,
	ParrySuccess,
	GuardBlock,
	HitReact
};

UCLASS()
class JUNDUCK_API AJunPlayer : public AJunCharacter
{
	GENERATED_BODY()

public:
	AJunPlayer();

public: // Engine / Character Overrides
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void HandleGameplayEventNotify(FGameplayTag EventTag) override;
	virtual void BeginAttackTraceWindow() override;
	virtual void EndAttackTraceWindow() override;

public: // External Gameplay API
	void BasicAttack();
	void StartDodge();
	void OnDodgeInputReleased();
	void OnDefenseStarted();
	void OnDefenseReleased();
	void ReceiveHit(EHitReactType HitType, float DamageAmount, AActor* DamageCauser, const FVector& SwingDirection);
	void AddCameraLookInput(const FVector2D& Input);
	void ToggleLockOn();

public: // Query / State API
	bool GetPlayerIsFalling();
	bool IsLockOn();
	bool IsGuardOn();
	bool IsGuardPoseActive();
	bool IsBasicAttacking() const;
	bool IsWalking() const;
	bool IsSprinting() const;
	bool IsInParrySuccess() const;
	bool IsJumpStartAnimTriggered() const;
	EJunMoveState GetMoveState() const;
	bool CanCancelBasicAttackIntoRecoveryAction(EJunBufferedRecoveryAction Action) const;
	bool CanBufferDefenseTransitionCancel() const;
	bool CanCancelDefenseTransitionIntoMove() const;
	bool CanBufferParrySuccessCancel(EJunBufferedParrySuccessCancelAction Action) const;
	bool ShouldUseGuardBase() const;
	EJunDefenseState GetDefenseState() const;
	bool IsParryWindowOpen();
	float GetDesiredMoveForward() const;
	float GetDesiredMoveRight() const;
	float GetGuardMoveBlendRemainTime() const;
	bool ShouldDeferGuardMoveInput() const;
	float GetDesiredMaxWalkSpeed() const;
	bool ShouldUseRunningAnim() const;

	void ToggleWalkingState();
	void SetDesiredMoveAxes(float NewForward, float NewRight);
	void OnMoveInputReleased();
	void BufferBasicAttackRecoveryAction(EJunBufferedRecoveryAction Action);
	void BufferDefenseTransitionCancelAction(EJunBufferedDefenseCancelAction Action);
	void BufferParrySuccessCancelAction(EJunBufferedParrySuccessCancelAction Action);
	void OnBasicAttackComboAdvanceStateBegin();
	void OnBasicAttackComboAdvanceStateEnd();
	bool TryCancelBasicAttackIntoMove();

protected: // BasicAttack
	void OnBasicAttackComboWindowBegin();

	UFUNCTION(BlueprintCallable, Category = "BasicAttack")
	void ResetBasicAttackCombo();

	void StartBasicAttack();
	void TryAdvanceBasicAttackCombo();
	void TryExecuteBufferedBasicAttackRecoveryAction();
	bool CanCancelBasicAttackIntoMove() const;
	void FinishBasicAttack();
	void CancelBasicAttackForRecoveryTransition(float BlendOutTime = 0.05f);
	void CancelBasicAttackIntoDefense();

	UFUNCTION()
	void OnBasicAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected: // Dodge
	UFUNCTION()
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void UpdateDodgeState(float DeltaTime);
	void FinishDodgeState();
	void AlignActorToDesiredMoveDirectionForDodge();
	class UAnimMontage* GetDodgeMontageToPlay() const;

protected: // Hit
	EJunPlayerHitResolveResult ResolveIncomingHitResult(EHitReactType IncomingHitType) const;
	bool CanBeInterruptedBy(EHitReactType IncomingHitType) const;
	ECharacterHitReactDirection DetermineHitReactDirection(const FVector& SwingDirection) const;
	ECharacterKnockbackDirection DetermineKnockbackDirectionFromDamageCauser(const AActor* DamageCauser) const;
	UAnimMontage* GetHitReactMontage(EHitReactType HitType, ECharacterHitReactDirection HitDirection) const;
	UAnimMontage* GetParrySuccessMontage(const FVector& SwingDirection) const;
	void StartParrySuccess();
	void CancelParrySuccessForCancelTransition(float BlendOutTime);
	void ExecuteBufferedParrySuccessCancelAction();
	void TryExecuteBufferedParrySuccessCancelAction();
	void StartGuardBlock();
	void StartHitReact(EHitReactType HitType, ECharacterHitReactDirection HitDirection);
	void ApplyCommonKnockback(
		ECharacterKnockbackDirection KnockbackDirection,
		float Strength,
		float BrakingDecelerationOverride,
		float GroundFrictionOverride,
		float BrakingFrictionFactorOverride,
		float OverrideDuration
	);
	void InterruptActionsForHitReaction();
	void UpdatePlayerHitState(float DeltaTime);
	void FinishPlayerHitState();

protected: // Defense
	void UpdateBasicAttackJumpAndMoveCancelState(float DeltaTime);
	void UpdateBasicAttackRecoveryBuffer(float DeltaTime);
	void UpdateDefenseInput(float DeltaTime);
	void CompleteGuardBlockReleaseIntoGuardEnd();
	void ExecuteBufferedDefenseTransitionCancelAction();
	void TryExecuteBufferedDefenseTransitionCancelAction();
	void CancelDefenseForCancelTransition(float BlendOutTime = -1.f);
	void FinishDefenseForCancel();
	void StartDefenseSequence();
	void BeginDefenseFromBufferedInput();
	void ResolveCurrentDeflectAttempt();
	void ApplyDefenseStartBlockTags();
	void ApplyGuardOnBlockTags();
	void ClearDefenseBlockTags();
	void EnterGuardLoop();
	void BeginGuardEnd();
	void FinishDefense();

	UFUNCTION()
	void OnGuardStartMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnGuardEndMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnLockOnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected: // Jump / Movement Helpers
	void ApplyRunningStateToAnimInstance(bool bNewIsRunning);
	void SetupMeshAndCollision();
	void SetupCameraComponents();
	void SetupMovementDefaults();
	void UpdateCameraAnchor(float DeltaTime);
	void CacheDefaultMovementBrakingSettings();
	void RestoreDefaultMovementBrakingSettings();
	void TryInitializeCameraRotationFromController();
	void ValidateLockOnTarget();
	void UpdateMovementSpeed(float DeltaTime);
	void UpdateCharacterRotationForCurrentCameraMode(float DeltaTime);
	void UpdateJumpStartAnimTrigger(float DeltaTime);
	void TryStartLockOnTurn();
	void CancelLockOnTurn(float BlendOutTime = -1.f);
	bool CanStartLockOnTurn() const;
	bool IsLockOnTurnPlaying() const;
	float GetLockOnTargetYawDelta() const;
	UAnimMontage* ChooseLockOnTurnMontage(float YawDelta) const;
	float GetCurrentRunMoveSpeed() const;
	FRotator GetJumpLaunchBasisRotation() const;
	FVector BuildJumpLaunchVelocity(const FVector2D& MoveInput) const;
	FVector GetJumpLaunchDirection(const FVector2D& MoveInput) const;
	void UpdateMovementBraking(float DeltaTime);

private:
	UAnimInstance* GetPlayerAnimInstance();

	void SpawnAndAttachWeapon();
	void SpawnAndAttachScabbard();

protected: // Target / Facing
	class AJunCharacter* FindBestAttackTarget();
	void FaceTargetForAttack(AJunCharacter* NewTarget);
	void UpdateAttackFacing(float DeltaTime);
	void BeginAttackFacingAssist(AJunCharacter* NewTarget);
	void EndAttackFacingAssist();

protected: // Camera
	void UpdateJunCamera(float DeltaTime);
	void UpdateFreeCamera(float DeltaTime);
	FVector GetCameraForwardOnPlane() const;

	void StartLockOn(AJunCharacter* NewTarget);
	void EndLockOn();
	bool IsLockOnTargetValid() const;

	void UpdateLockOnCamera(float DeltaTime);
	void UpdateLockOnCharacterRotation(float DeltaTime);
	FVector GetRawLockOnBonePoint() const;
	FVector GetRawLockOnCapsulePoint() const;
	FVector GetFilteredLockOnTargetPoint();

protected: // Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USceneComponent> CameraAnchor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> Camera;

protected: // External References
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AJunCharacter> TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	TObjectPtr<class AJunCharacter> LockOnTarget;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class UAnimInstance> AnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	class AWeaponActor* EquippedWeapon = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	class AWeaponActor* EquippedScabbard = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<class UAnimMontage> CurrentDodgeMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<class UAnimMontage> CurrentLockOnTurnMontage;

protected: // Runtime Camera State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	EJunCameraMode CameraMode = EJunCameraMode::Free;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	FVector2D PendingCameraLookInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float TargetCameraYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float TargetCameraPitch = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bCameraRotationInitialized = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	bool bLockOnActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	FVector CachedLockOnTargetPoint = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|LockOn")
	bool bLockOnTurnInProgress = false;

protected: // Runtime Combat / Defense State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	bool bIsBasicAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	int32 BasicAttackComboIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	bool bCanBufferBasicAttackComboInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	bool bBufferedBasicAttackComboInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	bool bBufferedBasicAttackInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	bool bBasicAttackComboAdvanceStateActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	EJunBufferedRecoveryAction BufferedBasicAttackRecoveryAction = EJunBufferedRecoveryAction::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	float PostBasicAttackJumpDodgeBufferRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	float PostBasicAttackDefenseBufferRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	float DodgeFinishRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	float DodgeInternalCooldownRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	float DodgeInvincibleRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dodge")
	bool bDodgeInputReleasedSinceLastDodge = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float ForwardDodgeFinishTime = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float BackwardDodgeFinishTime = 1.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float SideDodgeFinishTime = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float ForwardDodgeInternalCooldownDuration = 0.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float BackwardDodgeInternalCooldownDuration = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float SideDodgeInternalCooldownDuration = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float ForwardDodgeInvincibleDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float BackwardDodgeInvincibleDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float SideDodgeInvincibleDuration = 0.48f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	EJunDefenseState DefenseState = EJunDefenseState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	EJunBufferedDefenseCancelAction BufferedDefenseTransitionCancelAction = EJunBufferedDefenseCancelAction::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	bool bParryWindowOpen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float ParryWindowRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	bool bDefenseButtonHeld = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	bool bHoldRequestedForCurrentDeflect = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	bool bDeflectResolveReceived = false;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float DeflectResolveRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float CurrentDeflectHeldDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float DefenseTransitionElapsedTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float GuardEndFinishRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float GuardEndBaseReleaseRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float PostDefenseTransitionCancelBufferRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	bool bKeepGuardBaseWhileEnding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float GuardMoveBlendRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	float GuardExitMoveReleaseRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard")
	bool bDeferGuardMoveInput = false;

	FTimerHandle GuardBlockReleaseIntoGuardEndTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	EJunPlayerHitState CurrentHitState = EJunPlayerHitState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float PlayerHitStateRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float ChainParryWindowRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessElapsedTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	EHitReactType CurrentHitReactType = EHitReactType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	ECharacterHitReactDirection CurrentHitReactDirection = ECharacterHitReactDirection::Front_F;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	FVector LastIncomingSwingDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	ECharacterKnockbackDirection LastIncomingKnockbackDirection = ECharacterKnockbackDirection::Backward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	EJunBufferedParrySuccessCancelAction BufferedParrySuccessCancelAction = EJunBufferedParrySuccessCancelAction::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float KnockbackBrakingOverrideRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float KnockbackBrakingDecelerationOverride = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float KnockbackGroundFrictionOverride = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	float KnockbackBrakingFrictionFactorOverride = 0.f;

protected: // Runtime Movement / Jump State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bWalkRequested = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bSprintRequested = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float TargetMaxWalkSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DesiredMoveForward = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DesiredMoveRight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float PendingMoveForward = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float PendingMoveRight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DefaultWalkingBrakingDeceleration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DefaultGroundFriction = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float DefaultBrakingFrictionFactor = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bFreeRunSlideActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector FreeRunSlideDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float FreeRunSlideSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpStartAnimTriggerRemainTime = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	bool bUseAttackFacingAssist = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	float AttackFacingAssistRemainTime = 0.f;

protected: // Camera Tuning
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraAnchorInterpSpeed = 14.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraAnchorDodgeInterpSpeed = 7.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float SpringArmLocationInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraSocketOffsetInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Free")
	FVector FreeCameraSocketOffset = FVector(0.f, 45.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float FreeCameraYawSpeed = 70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float FreeCameraPitchSpeed = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float FreeCameraRotationInterpSpeed = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float MinCameraPitch = -60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float MaxCameraPitch = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float MovingLookInputScale = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Free")
	float AttackLookInputScale = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|Dodge")
	FVector DodgeCameraSocketOffset = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera|LockOn")
	FVector LockOnCameraSocketOffset = FVector(0.f, 45.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnRotationInterpSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnCharacterRotationInterpSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnBreakDistance = 150000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnPitchOffset = -20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnTargetZIgnoreThreshold = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnTargetXYIgnoreThreshold = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnTurnStartAngle = 55.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnTurn180Threshold = 135.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnTurnMaxGroundSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|LockOn")
	float LockOnTurnCancelBlendOutTime = 0.12f;

protected: // Attack / Defense Tuning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target")
	float AttackFacingAssistDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target")
	float AttackFacingInterpSpeed = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float GuardMoveBlendDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float DefaultParryWindowDuration = 0.5f; // 기본 패리 판정 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float DefaultDeflectResolveTime = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float GuardEnterHoldThreshold = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float GuardEndFinishTimeOffset = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float GuardEndBaseReleaseTimeOffset = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float PostDefenseTransitionCancelBufferDuration = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float DefenseCancelBlendOutTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float DefenseMoveCancelOpenTime = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float DefenseMoveCancelBlendOutTime = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float GuardBlockReleaseBlendOutTime = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessDuration = 0.45f; // 몽타주 길이 못 읽은 경우 사용

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ChainParryWindowDuration = 1.f; // 연속 패리 판정 가능 시간

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessBasicAttackCancelOpenTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessJumpCancelOpenTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessDodgeCancelOpenTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessMoveCancelOpenTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessBasicAttackCancelBlendOutTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessJumpCancelBlendOutTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessDodgeCancelBlendOutTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessMoveCancelBlendOutTime = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float GuardBlockDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float HitReactDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float LightHitReactDuration = 0.22f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float GuardBlockKnockbackStrength = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float GuardBlockKnockbackBrakingDeceleration = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float GuardBlockKnockbackGroundFriction = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float GuardBlockKnockbackBrakingFrictionFactor = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float GuardBlockKnockbackOverrideDuration = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessKnockbackStrength = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessKnockbackBrakingDeceleration = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessKnockbackGroundFriction = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessKnockbackBrakingFrictionFactor = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	float ParrySuccessKnockbackOverrideDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	float PostBasicAttackJumpDodgeBufferDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	float PostBasicAttackDefenseBufferDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	TArray<float> BasicAttackDodgeCancelOpenTimes = { 0.55f, 0.55f, 0.6f, 0.7f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	TArray<float> BasicAttackJumpCancelOpenTimes = { 0.55f, 0.55f, 0.6f, 0.7f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	TArray<float> BasicAttackDefenseCancelOpenTimes = { 0.4f, 0.5f, 0.6f, 0.7f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	TArray<float> BasicAttackMoveCancelOpenTimes = { 1.2f, 1.f, 1.f, 1.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	float BasicAttackDodgeCancelBlendOutTime = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	float BasicAttackJumpCancelBlendOutTime = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	float BasicAttackDefenseCancelBlendOutTime = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	float BasicAttackMoveCancelBlendOutTime = 0.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BasicAttack")
	float BasicAttackSectionElapsedTime = 0.f;

	float GuardExitMoveBlendDuration = 0.06f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	float GuardExitMoveReleaseDelay = 0.04f;

protected: // Movement / Jump Tuning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkMoveSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float FreeRunMoveSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float LockOnRunMoveSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintMoveSpeed = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float GuardMoveSpeed = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MoveSpeedInterpRate = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float FreeRunStopBrakingDeceleration = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float FreeRunSlideMinStartSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	float LockOnJumpMinimumHorizontalSpeed = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	float JumpStartAnimTriggerDuration = 0.1f;

protected: // Animation Assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BasicAttack")
	TObjectPtr<class UAnimMontage> BasicAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<class UAnimMontage> DodgeMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|LockOn")
	TObjectPtr<class UAnimMontage> LockOnDodgeRightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|LockOn")
	TObjectPtr<class UAnimMontage> LockOnDodgeBackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|LockOn")
	TObjectPtr<class UAnimMontage> LockOnDodgeLeftMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<class UAnimMontage> LockOnTurnLeft90Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<class UAnimMontage> LockOnTurnRight90Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<class UAnimMontage> LockOnTurnLeft180Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<class UAnimMontage> LockOnTurnRight180Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	TObjectPtr<class UAnimMontage> GuardStartMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Guard")
	TObjectPtr<class UAnimMontage> GuardEndMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> ParrySuccessL_UpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> ParrySuccessL_DownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> ParrySuccessR_UpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> ParrySuccessR_DownMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> CurrentParrySuccessMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> GuardBlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> LightHitFront_FMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> LightHitFront_LMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit")
	TObjectPtr<class UAnimMontage> LightHitFront_RMontage;

protected: // Weapon Assets
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeaponActor> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AWeaponActor> DefaultScabbardClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("WeaponSocket_R");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName ScabbardSocketName = TEXT("ScabSocket");
};
