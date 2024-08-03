#include "directx11.h"
#include "memoryTools.h"

#include "mordhauTypes.h"

bool PatchIsBanned(uintptr_t mordhauBaseAddress);

AMordhauCharacter* GetPlayer(AMordhauGameState* gameState, int index);

AMordhauCharacter* GetClosestPlayerToCrosshair(UWorld* uWorld);

void MoveYaw(float deltaYaw, float speed);

float GetProjectileVelocity(AMordhauCharacter* localPlayer);

void Aimbot(AMordhauCharacter* localPlayer, AMordhauCharacter* targetPlayer);

bool InitFunctions(uintptr_t mordhauBaseAddress);

bool IsValidPtr(void* ptr);

bool IsValidPlayer(AMordhauCharacter* player);

void SendRightClick();

bool IsCursorInWindow();