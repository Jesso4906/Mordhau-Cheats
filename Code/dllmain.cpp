#include "dllmain.h"

uintptr_t isBannedAddress = 0;

bool useAimbot = true;
bool useParrybot = true;
bool targetSameTeam = true;
bool increaseLookLimit = false;
bool enableDodge = true;
bool disableTurnCap = false;

const float g = -9.81;

const int aimbotCooldown = 1000;

const int parryDelay = 140000;
const int parryCooldown = 1000000;

const int cheatMenuWidth = 400;
const int cheatMenuHeight = 190;

DWORD WINAPI Thread(LPVOID param)
{
	uintptr_t mordhauBaseAddress = (uintptr_t)GetModuleHandle(L"Mordhau-Win64-Shipping.exe");

	if (mordhauBaseAddress == 0 || !InitFunctions(mordhauBaseAddress) || !HookPresent())
	{
		FreeLibraryAndExitThread((HMODULE)param, 0);
		return 0;
	}

	PatchIsBanned(mordhauBaseAddress);

	bool enableAimbot = false;
	AMordhauCharacter* localPlayer = nullptr;
	AMordhauCharacter* aimbotTargetPlayer = nullptr;
	int aimbotTimer = 0;

	AMordhauCharacter* parrybotTargetPlayer = nullptr;
	bool shouldParry = false;
	int parryTimer = 0;

	while (!GetAsyncKeyState(VK_INSERT)) // exit when ins key is pressed
	{
		UEngine* uEngine = *(UEngine**)(mordhauBaseAddress + uEngineOffset);
		if (!IsValidPtr(uEngine) || !uEngine->isInitialized) { continue; }

		UWorld* uWorld = *(UWorld**)(mordhauBaseAddress + uWorldOffset);
		if (!IsValidPtr(uWorld)) { continue; }

		AMordhauGameState* gameState = GetGameState(uWorld);
		if (!IsValidPtr(gameState)) { continue; }

		localPlayer = GetPlayer(gameState, 0);
		if (!IsValidPlayer(localPlayer)) { continue; }

		if (enableDodge) { localPlayer->canDodge = true; }

		if (increaseLookLimit) 
		{
			localPlayer->lookUpLimit = 180;
			localPlayer->lookDownLimit = 180;
		}
		else 
		{
			localPlayer->lookUpLimit = 55;
			localPlayer->lookDownLimit = 70;
		}

		if (disableTurnCap) 
		{
			localPlayer->turnRateCapTarget = -1;
			localPlayer->lookUpRateCapTarget = -1;
		}
		
		if(useAimbot && GetAsyncKeyState(VK_MBUTTON) & 1)
		{
			enableAimbot = !enableAimbot;

			if (enableAimbot) 
			{
				aimbotTargetPlayer = GetClosestPlayerToCrosshair(uWorld);
			}
		}

		if (enableAimbot) 
		{
			if (!IsValidPlayer(localPlayer) || !IsValidPlayer(aimbotTargetPlayer) || !IsCursorInWindow())
			{
				enableAimbot = false; 
				aimbotTimer = 0;
			}
			else if(aimbotTimer > aimbotCooldown)
			{
				Aimbot(localPlayer, aimbotTargetPlayer);
				aimbotTimer = 0;
			}

			aimbotTimer++;
		}

		if (useParrybot) 
		{
			Vector3 localPlayerPos;
			GetPawnViewLocation(localPlayer, &localPlayerPos);

			parrybotTargetPlayer = GetPlayer(gameState, 1);
			if (!IsValidPlayer(parrybotTargetPlayer)) { continue; }
			if (!targetSameTeam && parrybotTargetPlayer->team == localPlayer->team) { continue; }

			Vector3 closestPlayerPos;
			GetPawnViewLocation(parrybotTargetPlayer, &closestPlayerPos);

			Vector3 diff = localPlayerPos - closestPlayerPos;
			float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
			if (distance == 0 || distance > 300) { continue; }

			if (!shouldParry && parrybotTargetPlayer->lookSmoothingSlowAlpha == 0 && parrybotTargetPlayer->isBlocking != 2)
			{ 
				shouldParry = true;
				parryTimer = 0;

				if (IsValidPtr(localPlayer->rightHandEquipment)) { localPlayer->rightHandEquipment->canAttack = false; }
			}
		}
		else { shouldParry = false; }

		if (shouldParry)
		{
			parryTimer++;

			if (!IsValidPlayer(parrybotTargetPlayer))
			{
				parryTimer = 0;
				shouldParry = false;
				parrybotTargetPlayer = nullptr;
				if (IsValidPtr(localPlayer->rightHandEquipment)) { localPlayer->rightHandEquipment->canAttack = true; }
			}
			else if (parryTimer == parryDelay)
			{
				if (parrybotTargetPlayer->lookSmoothingSlowAlpha == 0 && parrybotTargetPlayer->isBlocking != 2) { SendRightClick(); }

				if (IsValidPtr(localPlayer->rightHandEquipment)) { localPlayer->rightHandEquipment->canAttack = true; }

				parryTimer = 0;
			}
			else if (parryTimer > parryCooldown) 
			{
				shouldParry = false;
				
				parrybotTargetPlayer = nullptr;
			}
		}
	}

	UnhookPresent();

	Sleep(100);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
	if (p_context) { p_context->Release(); p_context = NULL; }
	if (p_device) { p_device->Release(); p_device = NULL; }
	SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

	FreeLibraryAndExitThread((HMODULE)param, 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{ 
		CreateThread(0, 0, Thread, hModule, 0, 0); 
	}
	
	return TRUE;
}

void Draw() // called in DetourPresent()
{
	ImU32 primaryTeamColor = IM_COL32(150, 0, 0, 255);
	ImU32 secondaryTeamColor = IM_COL32(75, 0, 0, 255);

	ImGui::PushStyleColor(ImGuiCol_CheckMark, primaryTeamColor);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, primaryTeamColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, primaryTeamColor);

	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, secondaryTeamColor);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, secondaryTeamColor);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, secondaryTeamColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, secondaryTeamColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, secondaryTeamColor);
	
	ImGui::Begin("Jesso Mordhau Cheats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::SetWindowSize(ImVec2(cheatMenuWidth, cheatMenuHeight), ImGuiCond_Always);

	ImGui::Text("Ins - uninject");

	ImGui::Checkbox("Enable aimbot (middle mouse button to use)", &useAimbot);
	ImGui::Checkbox("Enable parrybot", &useParrybot);
	ImGui::Checkbox("Target players on same team", &targetSameTeam);
	ImGui::Checkbox("Set look limit to 180 degrees", &increaseLookLimit);
	ImGui::Checkbox("Disable turn cap", &disableTurnCap);
	ImGui::Checkbox("Enable dodge", &enableDodge);

	ImGui::End();
}

void Aimbot(AMordhauCharacter* localPlayer, AMordhauCharacter* targetPlayer)
{
	Vector3 localPlayerPos;
	GetPawnViewLocation(localPlayer, &localPlayerPos);

	Vector3 targetPlayerPos;
	GetPawnViewLocation(targetPlayer, &targetPlayerPos);
	targetPlayerPos.z -= 50;

	Vector3 diff = localPlayerPos - targetPlayerPos;
	float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
	if (distance == 0) { return; }

	float v = GetProjectileVelocity(localPlayer);

	if (distance > 500) 
	{ 
		Vector3 velocity;
		GetVelocity(targetPlayer, &velocity);
		velocity.z *= 0.1;

		targetPlayerPos = targetPlayerPos + ((velocity * distance * 0.125) / v);
	}

	diff = localPlayerPos - targetPlayerPos;
	
	float targetYaw = (atan2(diff.y, diff.x) * rToD);
	float currentYaw = localPlayer->yaw + 180;

	float deltaYaw = currentYaw - targetYaw;
	if (deltaYaw > 180) { deltaYaw = -(360 - deltaYaw); }
	if (deltaYaw < -180) { deltaYaw = (360 + deltaYaw); }

	// https://en.wikipedia.org/wiki/Projectile_motion#Angle_%CE%B8_required_to_hit_coordinate_(x,_y)
	float targetPitch = -(atan2((v * v - sqrt(v * v * v * v - g * (g * distance * distance + 2 * diff.z * v * v))), g * distance) * rToD) - 6;

	if (targetPitch < 0) { targetPitch += 180; }
	else { targetPitch -= 180; }

	float directPitch = -(asin(diff.z / distance) * rToD) - 6;
	if (directPitch < -30 || directPitch > 60) { targetPitch = directPitch; }

	if (targetPitch > 90 || targetPitch < -90) { return; }

	localPlayer->pitch = targetPitch;
	MoveYaw(deltaYaw, -5);
}

float GetProjectileVelocity(AMordhauCharacter* localPlayer)
{
	if (IsValidPtr(localPlayer->leftHandEquipment))
	{
		wchar_t* equipmentName = localPlayer->leftHandEquipment->equipmentName->text;

		if (wcscmp(equipmentName, L"Longbow") == 0)
		{
			return 600;
		}
		else if (wcscmp(equipmentName, L"Recurve Bow") == 0)
		{
			return 550;
		}
	}
	else if (IsValidPtr(localPlayer->rightHandEquipment))
	{
		wchar_t* equipmentName = localPlayer->rightHandEquipment->equipmentName->text;

		if (wcscmp(equipmentName, L"Crossbow") == 0)
		{
			return 625;
		}
		else if (wcscmp(equipmentName, L"Short Spear") == 0 || wcscmp(equipmentName, L"Javelin") == 0 || wcscmp(equipmentName, L"Partisan") == 0)
		{
			return 280;
		}
		else if (wcscmp(equipmentName, L"Throwing Axe") == 0 || wcscmp(equipmentName, L"Throwing Knife") == 0 || wcscmp(equipmentName, L"Rock") == 0)
		{
			return 230;
		}
		else if (wcscmp(equipmentName, L"War Axe") == 0 || wcscmp(equipmentName, L"Maul") == 0)
		{
			return 125;
		}
	}

	return 250; // any other small throwable weapon
}

void MoveYaw(float deltaYaw, float speed)
{
	INPUT input;
	ZeroMemory(&input, sizeof(input));

	input.type = INPUT_MOUSE;

	MOUSEINPUT mouseInput;
	ZeroMemory(&mouseInput, sizeof(mouseInput));

	mouseInput.dwFlags = MOUSEEVENTF_MOVE;

	float deltaX = deltaYaw * speed;

	if (deltaX > 0 && deltaX < 1) { deltaX = 1; }
	if (deltaX < 0 && deltaX > -1) { deltaX = -1; }

	mouseInput.dx = deltaX;

	input.mi = mouseInput;
	SendInput(1, &input, sizeof(INPUT));
}

AMordhauCharacter* GetPlayer(AMordhauGameState* gameState, int index)
{
	uintptr_t playerList = gameState->characterDisatanceArray; // already sorted by distance to player

	AMordhauCharacter** playerPtr = (AMordhauCharacter**)((playerList + (index * 8)));
	if (!IsValidPtr(playerPtr)) { return nullptr; }
	AMordhauCharacter* player = *playerPtr;
	if (!IsValidPlayer(player)) { return nullptr; }

	return player;
}

AMordhauCharacter* GetClosestPlayerToCrosshair(UWorld* uWorld)
{
	AMordhauGameState* gameState = GetGameState(uWorld);
	if (!IsValidPtr(gameState)) { return nullptr; }

	AMordhauCharacter* localPlayer = GetPlayer(gameState, 0);
	if (!IsValidPlayer(localPlayer)) { return nullptr; }

	float minYawDiff = 999999;
	AMordhauCharacter* result = nullptr;

	for (int i = 1; i < gameState->playerCount; i++)
	{
		AMordhauCharacter* currentPlayer = GetPlayer(gameState, i);
		if (!IsValidPlayer(currentPlayer)) { break; }

		if (!targetSameTeam && currentPlayer->team == localPlayer->team) { continue; }

		Vector3 localPlayerPos;
		GetPawnViewLocation(localPlayer, &localPlayerPos);

		Vector3 targetPlayerPos;
		GetPawnViewLocation(currentPlayer, &targetPlayerPos);

		Vector3 diff = localPlayerPos - targetPlayerPos;

		float targetYaw = (atan2(diff.y, diff.x) * rToD);
		float currentYaw = localPlayer->yaw + 180;

		float yawDiff = currentYaw - targetYaw;
		if (yawDiff > 180) { yawDiff = -(360 - yawDiff); }
		if (yawDiff < -180) { yawDiff = (360 + yawDiff); }

		yawDiff = abs(yawDiff);

		if (yawDiff < minYawDiff)
		{
			minYawDiff = yawDiff;
			result = currentPlayer;
		}
	}

	return result;
}

bool InitFunctions(uintptr_t mordhauBaseAddress)
{
	uintptr_t getGameStateAddress = FindArrayOfBytes(mordhauBaseAddress, (BYTE*)"\x40\x53\x48\x83\xEC\x20\x48\x8B\x99\x20\x01", 11, 0xCC);
	if (getGameStateAddress == 0) { return false; }
	GetGameState = (GetGameStateType)getGameStateAddress;

	uintptr_t getVelocityAddress = FindArrayOfBytes(mordhauBaseAddress, (BYTE*)"\x48\x89\x5C\x24\x10\x57\x48\x83\xEC\x20\x48\x8B\xF9\x48\x8B\xDA\x48\x8B\x89", 19, 0xCC);
	if (getVelocityAddress == 0) { return false; }
	GetVelocity = (GetVelocityType)getVelocityAddress;

	uintptr_t getPawnViewLocationAddress = FindArrayOfBytes(mordhauBaseAddress, (BYTE*)"\x48\x83\xEC\x28\x48\x8B\x81\x30\x01\x00\x00\xF3", 12, 0xCC);
	if (getPawnViewLocationAddress == 0) { return false; }
	GetPawnViewLocation = (GetPawnViewLocationType)getPawnViewLocationAddress;

	return true;
}

bool PatchIsBanned(uintptr_t mordhauBaseAddress)
{
	isBannedAddress = FindArrayOfBytes(mordhauBaseAddress, (BYTE*)"\x48\x89\x5C\x24\x08\x57\x48\x83\xEC\x20\x83\x7A\x08\x01\x48\x8B\xDA\x48\x8B\xF9\x7E\x3F\x4C\x8B\xC2\x48\x81\xC1\x10\x06\x00\x00", 32, 0xCC);
	if (isBannedAddress == 0) { return false; }

	// mov eax, 0
	// ret
	SetBytes((void*)isBannedAddress, (BYTE*)"\xB8\x0\x0\x0\x0\xC3", 6);

	return true;
}

bool IsValidPtr(void* ptr) 
{ 
	return (uintptr_t)ptr > 0x10000 && (uintptr_t)ptr < 0x7FFFFFFFFFFF;
}

bool IsValidPlayer(AMordhauCharacter* player)
{
	return IsValidPtr(player) && !player->isDead;
}

void SendRightClick() 
{
	INPUT input;
	ZeroMemory(&input, sizeof(input));

	input.type = INPUT_MOUSE;

	MOUSEINPUT mouseInput;
	ZeroMemory(&mouseInput, sizeof(mouseInput));

	mouseInput.dwFlags = MOUSEEVENTF_RIGHTDOWN;

	input.mi = mouseInput;
	SendInput(1, &input, sizeof(INPUT));

	mouseInput.dwFlags = MOUSEEVENTF_RIGHTUP;

	input.mi = mouseInput;
	SendInput(1, &input, sizeof(INPUT));
}

bool IsCursorInWindow()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (viewport == nullptr) { return false; }

	ImVec2 windowPos = viewport->Pos;
	ImVec2 windowSize = viewport->Size;
	ImVec2 cursorPos = ImGui::GetIO().MousePos;

	if (cursorPos.x < windowPos.x) { return false; }
	if (cursorPos.x > windowPos.x + windowSize.x) { return false; }

	if (cursorPos.y < windowPos.y) { return false; }
	if (cursorPos.y > windowPos.y + windowSize.y) { return false; }

	return true;
}