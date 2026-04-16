

#pragma once
#include "JunLogChannels.h"
#include "Data/JunDataAsset.h"
#include "Engine/AssetManager.h"
#include "JunAssetManager.generated.h"
class UJunDataAsset;

DECLARE_DELEGATE_TwoParams(FAsyncLoadCompletedDelegate, const FName&/*AssetName or Label*/, UObject*/*LoadedAsset*/);
/**
 * 
 */
UCLASS()
class JUNDUCK_API UJunAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UJunAssetManager();

	static UJunAssetManager& Get();

public:
	static void Initialize();

	// 다른 클래스에서 사용할 GetAsset 함수
	template<typename AssetType>
	static AssetType* GetAssetByName(const FName& AssetName);

	// 동기 로딩 함수
	static void LoadSyncByPath(const FSoftObjectPath& AssetPath);
	static void LoadSyncByName(const FName& AssetName);
	static void LoadSyncByLabel(const FName& Label);

	// 비동기 로딩 함수
	static void LoadAsyncByPath(const FSoftObjectPath& AssetPath, FAsyncLoadCompletedDelegate CompletedDelegate = FAsyncLoadCompletedDelegate());
	static void LoadAsyncByName(const FName& AssetName, FAsyncLoadCompletedDelegate CompletedDelegate = FAsyncLoadCompletedDelegate());

	// 릴리즈 함수
	static void ReleaseByPath(const FSoftObjectPath& AssetPath);
	static void ReleaseByName(const FName& AssetName);
	static void ReleaseByLabel(const FName& Label);
	static void ReleaseAll();

private:
	// PrimaryAsset을 LoadedAssetData로 가져오고 LoadSyncByLabel로 Preload 라벨이 있는 에셋만 로드
	void LoadPreloadAssets();
	// NameToLoadedAsset에 캐싱하는 부분(모든 로딩 함수가 사용하는 부품)
	void AddLoadedAsset(const FName& AssetName, const UObject* Asset);

private:
	UPROPERTY()
	TObjectPtr<UJunDataAsset> LoadedAssetData;

	UPROPERTY()
	TMap<FName, TObjectPtr<const UObject>> NameToLoadedAsset;
};

template<typename AssetType>
AssetType* UJunAssetManager::GetAssetByName(const FName& AssetName)
{
	UJunDataAsset* AssetData = Get().LoadedAssetData;
	check(AssetData);

	AssetType* LoadedAsset = nullptr;
	const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByName(AssetName);
	if (AssetPath.IsValid())
	{
		LoadedAsset = Cast<AssetType>(AssetPath.ResolveObject());
		if (LoadedAsset == nullptr)
		{
			UE_LOG(LogJun, Warning, TEXT("Attempted sync loading because asset hadn't loaded yet [%s]."), *AssetPath.ToString());
			LoadedAsset = Cast<AssetType>(AssetPath.TryLoad());
		}
	}
	return LoadedAsset;
}
