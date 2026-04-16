

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JunDataAsset.generated.h"


USTRUCT()
struct FAssetEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	FName AssetName;

	UPROPERTY(EditDefaultsOnly)
	FSoftObjectPath AssetPath;

	UPROPERTY(EditDefaultsOnly)
	TArray<FName> AssetLabels;
};

USTRUCT()
struct FAssetSet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FAssetEntry> AssetEntries;

};

/**
 * 
 */
UCLASS()
class JUNDUCK_API UJunDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual void PreSave(FObjectPreSaveContext objectSaveContext) override;

	
public:
	FSoftObjectPath GetAssetPathByName(const FName& AssetName);
	const FAssetSet& GetAssetSetByLabel(const FName& Label);

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FAssetSet> AssetGroupNameToSet;

	UPROPERTY()
	TMap<FName, FSoftObjectPath> AssetNameToPath;

	UPROPERTY()
	TMap<FName, FAssetSet> AssetLabelToSet;
};
