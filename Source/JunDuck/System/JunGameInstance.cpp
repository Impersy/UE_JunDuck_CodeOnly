


#include "JunGameInstance.h"
#include "JunAssetManager.h"

UJunGameInstance::UJunGameInstance(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{


}

void UJunGameInstance::Init()
{
	Super::Init();

	UJunAssetManager::Initialize();
}

void UJunGameInstance::Shutdown()
{
	Super::Shutdown();
}
