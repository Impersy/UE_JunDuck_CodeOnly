// Copyright Epic Games, Inc. All Rights Reserved.

#include "JunDuck.h"
#include "Modules/ModuleManager.h"
#include "JunLogChannels.h"

class FJunModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override 
	{
		UE_LOG(LogJun, Log, TEXT("StartupModule - Jun Log Test"));
	}

	virtual void ShutdownModule() override
	{

	}

};



IMPLEMENT_PRIMARY_GAME_MODULE(FJunModule, JunDuck, "JunDuck" );
