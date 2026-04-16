

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "JunInputData.generated.h"


class UInputMappingContext;
class UInputAction;

USTRUCT()
struct FJunInputAction
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;
};

/**
 * 
 */
UCLASS()
class JUNDUCK_API UJunInputData : public UDataAsset
{
	GENERATED_BODY()

public:
	const UInputAction* FindInputActionByTag(const FGameplayTag& InputTag) const;


	
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> IMC_Default;

	UPROPERTY(EditDefaultsOnly)
	TArray<FJunInputAction> InputActions;


};
