


#include "Data/JunInputData.h"
#include "JunLogChannels.h"

const UInputAction* UJunInputData::FindInputActionByTag(const FGameplayTag& InputTag) const
{

    for (const FJunInputAction& Action : InputActions) 
    {
        if (Action.InputAction && Action.InputTag == InputTag) 
        {
            return Action.InputAction;
        }
    }

    UE_LOG(LogJun, Error, TEXT("Can't find InputAction for InputTag [%s]"), *InputTag.ToString());
    return nullptr;
}
