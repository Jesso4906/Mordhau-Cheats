#pragma once
#include "mathStructs.h"

// these are offset from Mordhau-Win64-Shipping.exe
const unsigned int uEngineOffset = 0x58FC430;
const unsigned int uWorldOffset = 0x58FFE00;

const unsigned int isInitOffset = 0xA38;
struct UEngine 
{
	char pad1[isInitOffset];
	bool isInitialized;
};

const unsigned int playerCountOffset = 0x240;
const unsigned int charDistArrOffset = 0x3B0;
struct AMordhauGameState 
{
	char pad1[playerCountOffset];
	int playerCount;
	
	char pad2[charDistArrOffset - playerCountOffset - sizeof(playerCount)];
	uintptr_t characterDisatanceArray;
};

const unsigned int gameStateOffset = 0x120;
struct UWorld
{
	char pad1[gameStateOffset];
	AMordhauGameState* gameState;
};

const unsigned int textOffset = 0x28;
struct FText 
{
	char pad1[textOffset];
	wchar_t* text;
};

const unsigned int equipmentNameOffset = 0x550;
const unsigned int canAttackOffset = 0xCB9;
struct AMordhauEquipment 
{
	char pad1[equipmentNameOffset];
	FText* equipmentName;

	char pad2[canAttackOffset - equipmentNameOffset - sizeof(equipmentName)];
	bool canAttack;
};

const unsigned int isDeadOffset = 0x504;
const unsigned int pitchOffset = 0x520;
const unsigned int yawOffset = 0x524;
const unsigned int teamOffset = 0x660;
const unsigned int lookUpLimitOffset = 0x8A0;
const unsigned int lookDownLimitOffset = 0x8A4;
const unsigned int lookSmoothingSlowAlphaOffset = 0x8C0;
const unsigned int lookUpRateCapTargetOffset = 0x8D0;
const unsigned int camDodgeOffset = 0xE66;
const unsigned int isBlockingOffset = 0x10CA;
const unsigned int rightHandEquipmentOffset = 0x11F8;
const unsigned int leftHandEquipmentOffset = 0x1200;
struct AMordhauCharacter // inherits from: AAdvancedCharacter, ACharacter, APawn, AActor
{
	char pad1[isDeadOffset];
	bool isDead;
	
	char pad2[pitchOffset - isDeadOffset - sizeof(isDead)];
	float pitch;
	float yaw;

	char pad3[teamOffset - yawOffset - sizeof(yaw)];
	char team;

	char pad4[lookUpLimitOffset - teamOffset - sizeof(team)];
	float lookUpLimit;
	float lookDownLimit;

	char pad5[lookSmoothingSlowAlphaOffset - lookDownLimitOffset - sizeof(lookDownLimit)];
	float lookSmoothingSlowAlpha;
	float turnRateCap;
	float turnRateCapTarget;
	float lookUpRateCap;
	float lookUpRateCapTarget;

	char pad6[camDodgeOffset - lookUpRateCapTargetOffset - sizeof(lookUpRateCapTarget)];
	bool canDodge;

	char pad7[isBlockingOffset - camDodgeOffset - sizeof(canDodge)];
	char isBlocking;

	char pad8[rightHandEquipmentOffset - isBlockingOffset - sizeof(isBlocking)];
	AMordhauEquipment* rightHandEquipment;
	AMordhauEquipment* leftHandEquipment;
};

struct FRotator 
{
	float pitch, yaw, roll;
};

typedef AMordhauGameState* (__fastcall* GetGameStateType)(UWorld* uWorld); // UWorld::GetGameState
GetGameStateType GetGameState;

typedef Vector3* (__fastcall* GetVelocityType)(AMordhauCharacter* uPawn, Vector3* result); // APawn::GetVelocity
GetVelocityType GetVelocity;

typedef Vector3* (__fastcall* GetPawnViewLocationType)(AMordhauCharacter* uPawn, Vector3* result); // APawn::GetPawnViewLocation
GetPawnViewLocationType GetPawnViewLocation;

typedef bool (__fastcall* IsPlayerControlledType)(AMordhauCharacter* aPawn); // APawn::IsPlayerControlled
IsPlayerControlledType IsPlayerControlled;