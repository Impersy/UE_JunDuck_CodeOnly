

#pragma once


UENUM(BlueprintType)
enum class ECreatureState : uint8
{
	None,
	Moving,
	InAir,
	Skill,
	Dead
};

// State 여러가지가 동시에 가능할수있어서 비트마스크? 도입 해보자
