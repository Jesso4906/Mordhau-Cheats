#include "directx11.h"
#include "memoryTools.h"

#include "mordhauTypes.h"

void Aimbot(AMordhauCharacter* localPlayer, AMordhauCharacter* targetPlayer);

float GetProjectileVelocity(AMordhauCharacter* localPlayer);

void MoveYaw(float deltaYaw, float speed);

AMordhauCharacter* GetPlayer(AMordhauGameState* gameState, int index);

AMordhauCharacter* GetClosestPlayerToCrosshair(UWorld* uWorld);

bool InitFunctions(uintptr_t mordhauBaseAddress);

bool PatchIsBanned(uintptr_t mordhauBaseAddress);

bool IsValidPtr(void* ptr);

bool IsValidPlayer(AMordhauCharacter* player);

void SendRightClick();

bool IsCursorInWindow();