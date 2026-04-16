


#include "System/JunAssetManager.h"
#include "JunLogChannels.h"

UJunAssetManager::UJunAssetManager() : Super()
{
}

UJunAssetManager& UJunAssetManager::Get()
{
	if (UJunAssetManager* Singleton = Cast<UJunAssetManager>(GEngine->AssetManager))
	{
		return *Singleton; //*ptr은 "값"이 아니라 “메모리 위치에 있는 객체 자체를 표현하는 식” 이다
	}

	UE_LOG(LogJun, Fatal, TEXT("Can't find UJunAssetManager"));

	return *NewObject<UJunAssetManager>();
}

void UJunAssetManager::Initialize()
{
	Get().LoadPreloadAssets();

}


void UJunAssetManager::LoadSyncByPath(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		UObject* LoadedAsset = AssetPath.ResolveObject();
		if (LoadedAsset == nullptr)
		{
			if (UAssetManager::IsInitialized())
			{
				LoadedAsset = UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
			}
			else
			{
				LoadedAsset = AssetPath.TryLoad();
			}
		}

		if (LoadedAsset)
		{
			Get().AddLoadedAsset(AssetPath.GetAssetFName(), LoadedAsset);
		}
		else
		{
			UE_LOG(LogJun, Fatal, TEXT("Failed to load asset [%s]"), *AssetPath.ToString());
		}
	}
}

void UJunAssetManager::LoadSyncByName(const FName& AssetName)
{
	UJunDataAsset* AssetData = Get().LoadedAssetData;
	check(AssetData);

	const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByName(AssetName);
	LoadSyncByPath(AssetPath);
}

void UJunAssetManager::AddLoadedAsset(const FName& AssetName, const UObject* Asset)
{
	if (AssetName.IsValid() && Asset)
	{
		//멀티스레드 활용해서 로딩할 필요가 있을 시 Lock 사용
		//FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);

		if (NameToLoadedAsset.Contains(AssetName) == false)
		{
			NameToLoadedAsset.Add(AssetName, Asset);
		}
	}
}

void UJunAssetManager::LoadSyncByLabel(const FName& Label)
{
	if (UAssetManager::IsInitialized() == false)
	{
		UE_LOG(LogJun, Error, TEXT("AssetManager must be initialized"));
		return;
	}

	UJunDataAsset* AssetData = Get().LoadedAssetData;
	check(AssetData);

	TArray<FSoftObjectPath> AssetPaths;

	const FAssetSet& AssetSet = AssetData->GetAssetSetByLabel(Label);
	for (const FAssetEntry& AssetEntry : AssetSet.AssetEntries)
	{
		const FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
		LoadSyncByPath(AssetPath);
		if (AssetPath.IsValid())
		{
			AssetPaths.Emplace(AssetPath);
		}
	}

	GetStreamableManager().RequestSyncLoad(AssetPaths);

	for (const FAssetEntry& AssetEntry : AssetSet.AssetEntries)
	{
		const FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
		if (AssetPath.IsValid())
		{
			if (UObject* LoadedAsset = AssetPath.ResolveObject())
			{
				Get().AddLoadedAsset(AssetEntry.AssetName, LoadedAsset);
			}
			else
			{
				UE_LOG(LogJun, Fatal, TEXT("Failed to load asset [%s]"), *AssetPath.ToString());
			}
		}
	}
}

void UJunAssetManager::LoadAsyncByPath(const FSoftObjectPath& AssetPath, FAsyncLoadCompletedDelegate CompletedDelegate)
{
	if (UAssetManager::IsInitialized() == false)
	{
		UE_LOG(LogJun, Error, TEXT("AssetManager must be initialized"));
		return;
	}

	if (AssetPath.IsValid())
	{
		if (UObject* LoadedAsset = AssetPath.ResolveObject())
		{
			Get().AddLoadedAsset(AssetPath.GetAssetFName(), LoadedAsset);
		}
		else
		{
			TArray<FSoftObjectPath> AssetPaths;
			AssetPaths.Add(AssetPath);

			TSharedPtr<FStreamableHandle> Handle = GetStreamableManager().RequestAsyncLoad(AssetPaths);

			Handle->BindCompleteDelegate(FStreamableDelegate::CreateLambda([AssetName = AssetPath.GetAssetFName(), AssetPath, CompleteDelegate = MoveTemp(CompletedDelegate)]()
				{
					UObject* LoadedAsset = AssetPath.ResolveObject();
					Get().AddLoadedAsset(AssetName, LoadedAsset);
					if (CompleteDelegate.IsBound())
						CompleteDelegate.Execute(AssetName, LoadedAsset);
				}));
		}
	}
}

void UJunAssetManager::LoadAsyncByName(const FName& AssetName, FAsyncLoadCompletedDelegate CompletedDelegate)
{
	if (UAssetManager::IsInitialized() == false)
	{
		UE_LOG(LogJun, Error, TEXT("AssetManager must be initialized"));
		return;
	}

	UJunDataAsset* AssetData = Get().LoadedAssetData;
	check(AssetData);

	const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByName(AssetName);
	LoadAsyncByPath(AssetPath, CompletedDelegate);
}

void UJunAssetManager::ReleaseByPath(const FSoftObjectPath& AssetPath)
{
	FName AssetName = AssetPath.GetAssetFName();
	ReleaseByName(AssetName);
}

void UJunAssetManager::ReleaseByName(const FName& AssetName)
{
	UJunAssetManager& AssetManager = Get();
	if (AssetManager.NameToLoadedAsset.Contains(AssetName))
	{
		AssetManager.NameToLoadedAsset.Remove(AssetName);
	}
	else
	{
		UE_LOG(LogJun, Log, TEXT("Can't find loaded asset by assetName [%s]."), *AssetName.ToString());
	}
}

void UJunAssetManager::ReleaseByLabel(const FName& Label)
{
	UJunAssetManager& AssetManager = Get();
	UJunDataAsset* LoadedAssetData = AssetManager.LoadedAssetData;
	const FAssetSet& AssetSet = LoadedAssetData->GetAssetSetByLabel(Label);

	for (const FAssetEntry& AssetEntry : AssetSet.AssetEntries)
	{
		const FName& AssetName = AssetEntry.AssetName;
		if (AssetManager.NameToLoadedAsset.Contains(AssetName))
		{
			AssetManager.NameToLoadedAsset.Remove(AssetName);
		}
		else
		{
			UE_LOG(LogJun, Log, TEXT("Can't find loaded asset by assetName [%s]."), *AssetName.ToString());
		}
	}
}

void UJunAssetManager::ReleaseAll()
{
	UJunAssetManager& AssetManager = Get();
	AssetManager.NameToLoadedAsset.Reset();
}

void UJunAssetManager::LoadPreloadAssets()
{
	if (LoadedAssetData)
		return;

	UJunDataAsset* AssetData = nullptr;
	FPrimaryAssetType PrimaryAssetType(UJunDataAsset::StaticClass()->GetFName());
	TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
	if (Handle.IsValid())
	{
		Handle->WaitUntilComplete(0.f, false);
		AssetData = Cast<UJunDataAsset>(Handle->GetLoadedAsset());
	}

	if (AssetData)
	{
		LoadedAssetData = AssetData;
		LoadSyncByLabel("Preload");
	}
	else
	{
		UE_LOG(LogJun, Fatal, TEXT("Failed to load AssetData asset type [%s]."), *PrimaryAssetType.ToString());
	}
}