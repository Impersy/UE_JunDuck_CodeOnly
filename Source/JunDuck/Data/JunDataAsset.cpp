


#include "Data/JunDataAsset.h"
#include "UObject/ObjectSaveContext.h"

void UJunDataAsset::PreSave(FObjectPreSaveContext objectSaveContext)
{
	Super::PreSave(objectSaveContext);

	AssetNameToPath.Empty();
	AssetLabelToSet.Empty();

	AssetGroupNameToSet.KeySort([](const FName& A, const FName& B)
		{
			return (A.Compare(B) < 0);
		});

	
	for (const auto& Pair : AssetGroupNameToSet) 
	{
		// Name To Set 으로 되어있는 맵에서 FAssetSet 단위로 순회
		const FAssetSet& AssetSet = Pair.Value;

		// 현재 FAssetSet 기준으로 그 안에 있는 TArray인 AssetEntries를 순회
		for (FAssetEntry AssetEntry : AssetSet.AssetEntries) 
		{
			// (Name, Path) 로 미리 캐싱
			FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
			const FString& AssetName = AssetPath.GetAssetName();
			if(AssetName.StartsWith(TEXT("BP_")) || AssetName.StartsWith(TEXT("B_")) ||
			   AssetName.StartsWith(TEXT("GE_")) || AssetName.StartsWith(TEXT("GA_")))
			{
				FString AssetPathString = AssetPath.GetAssetPathString();
				AssetPathString.Append(TEXT("_C"));
				AssetPath = FSoftObjectPath(AssetPathString);
			}

			AssetNameToPath.Emplace(AssetEntry.AssetName, AssetEntry.AssetPath);

			// (Label, AssetSet(AssetEntries)) 로 미리 캐싱
			for (const FName& Label : AssetEntry.AssetLabels) 
			{
				// 이 코드는 아직 문법이 어려워서 정확한 분석이 안됨
				AssetLabelToSet.FindOrAdd(Label).AssetEntries.Emplace(AssetEntry);
			}

		}


	}
}

FSoftObjectPath UJunDataAsset::GetAssetPathByName(const FName& AssetName)
{
	FSoftObjectPath* AssetPath = AssetNameToPath.Find(AssetName);
	ensureAlwaysMsgf(AssetPath, TEXT("Can't find Asset Path from Asset Name [%s]."), *AssetName.ToString());
	return *AssetPath;
}

const FAssetSet& UJunDataAsset::GetAssetSetByLabel(const FName& Label)
{
	const FAssetSet* AssetSet = AssetLabelToSet.Find(Label);
	ensureAlwaysMsgf(AssetSet, TEXT("Can't find Asset Set from Label [%s]."), *Label.ToString());
	return *AssetSet;
}
