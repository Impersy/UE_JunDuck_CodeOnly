

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighLightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHighLightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class JUNDUCK_API IHighLightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void HighLight() = 0;
	virtual void UnHighLight() = 0;



};
