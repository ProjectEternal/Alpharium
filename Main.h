#pragma once
#include "pch.h"
#include "Memory.h"
#include "Functions.h"
#include <cstdarg>
#include <regex>
#include "MinHook.h"
#pragma comment(lib,"minhook.lib")
bool InGame = false;
Unreal::UObject* (__fastcall*SpawnActor_OG)(Unreal::UObject* InWorld, Unreal::UObject* Class, Unreal::UObject* Class1, void* Transform, void* SpawnParameters);

#ifdef DEBUG
std::ofstream SA_Log("SA.txt");
std::ofstream PE_Log("PE.txt");
#endif

struct FGameplayAbilitySpecDef
{
	Unreal::UObject* Ability;
	unsigned char Unk00[0x90];
};

struct FGameplayEffectContextHandle
{
	unsigned char Unk00[0x30];
};

struct FActiveGameplayEffectHandle
{
	int Handle;
	bool bPassedFiltersAndWasExecuted;
	unsigned char Unk00[0x3];
};

void GrantEffect(Unreal::UObject* Effect) {
	if (!Effect) {
		return;
	}

	Unreal::UObject* ASC = *Finder::Find(Globals::GPawn, "AbilitySystemComponent");
	struct
	{
		Unreal::UObject* GameplayEffectClass;
		float Level;
		FGameplayEffectContextHandle EffectContext;
		FActiveGameplayEffectHandle ret;
	} params{ Effect, 1.0f, {}, {} };

	ASC->ProcessEvent(FindObject("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectToSelf"), &params);
}

void GrantAbility(Unreal::UObject* Ability) {
	if (!Ability) {
		return;
	}

	Unreal::UObject* ASC = *Finder::Find(Globals::GPawn, "AbilitySystemComponent");

	static Unreal::UObject* GE_Class = nullptr;
	if (GE_Class == nullptr) {
		GE_Class = FindObject("/Game/Abilities/Player/Constructor/Traits/CreativeEngineering/GE_GrantReflectMeleeDmg.GE_GrantReflectMeleeDmg_C");
	}

	static Unreal::UObject* DefaultGE_Class = nullptr;
	if (DefaultGE_Class == nullptr) {
		DefaultGE_Class = FindObject("/Game/Abilities/Player/Constructor/Traits/CreativeEngineering/GE_GrantReflectMeleeDmg.Default__GE_GrantReflectMeleeDmg_C");
	}

	Finder::Find<Unreal::TArray<FGameplayAbilitySpecDef>*>(DefaultGE_Class, "GrantedAbilities")->Data[0].Ability = Ability;

	GrantEffect(GE_Class);
}

bool bRealPawn = false;

void SetupPawn() {
	Functions::GetPC();
	Globals::GPC->ProcessEvent(FindObject("/Script/Engine.Controller:Possess"), &Globals::GPawn);
	Unreal::UObject* CM = Functions::SetupCM();
	CM->ProcessEvent(FindObject("/Script/Engine.CheatManager:God"));
	if (bRealPawn) {
		Unreal::FString Gender = L"Male";
		Globals::GPawn->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerPawn:RandomizeOutfit"), &Gender);
		(*Finder::Find(Globals::GPawn, "PlayerState"))->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerState:OnRep_CharacterParts"));
		Globals::GPawn->ProcessEvent(FindObject("/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C:OnCharacterPartsReinitialized"));
		Globals::GPawn->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerPawn:ToggleGender"));
		auto MovementSet = (*Finder::Find(Globals::GPawn, "MovementSet"));
		*Finder::Find<float*>(MovementSet, "BackwardSpeedMultiplier") = 0.65f;
		*Finder::Find<float*>(MovementSet, "WalkSpeed") = 200.0f;
		*Finder::Find<float*>(MovementSet, "RunSpeed") = 410.0f;
		*Finder::Find<float*>(MovementSet, "SprintSpeed") = 550.0f;
		MovementSet->ProcessEvent(FindObject("/Script/FortniteGame.FortMovementSet:OnRep_SpeedMultiplier"));
		auto AttrSet = FindObject("_0.PlayerAttrSet", false);
		*Finder::Find<float*>(AttrSet, "StaminaRegenDelay") = 0.5f;
		*Finder::Find<float*>(AttrSet, "StaminaRegenRate") = 0.65f;
		*Finder::Find<float*>(AttrSet, "Stamina") = 100.0f;
		*Finder::Find<float*>(AttrSet, "MaxStamina") = 100.0f;
		AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_Stamina"));
		AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_MaxStamina"));
		AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_StaminaRegenDelay"));
		AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_StaminaRegenRate"));

		GrantAbility(FindObject("/Script/FortniteGame.FortGameplayAbility_Jump"));
		GrantAbility(FindObject("/Script/FortniteGame.FortGameplayAbility_Sprint"));
	}

	DumpObjects();
}

void Setup() {
	Functions::GetPC();
	*Finder::Find<bool*>(Globals::GPC, "bHasClientFinishedLoading") = true;
	*Finder::Find<bool*>(Globals::GPC, "bHasServerFinishedLoading") = true;
	*Finder::Find<bool*>(Globals::GPC, "bReadyToStartMatch") = true;
	*Finder::Find<bool*>(Globals::GPC, "bClientPawnIsLoaded") = true;
	*Finder::Find<bool*>(Globals::GPC, "bHasInitiallySpawned") = true;
	Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_bHasServerFinishedLoading"));
	Globals::GameMode->ProcessEvent(FindObject("/Script/Engine.GameMode:StartMatch"));
	DumpObjects();
}

namespace Hooks {
	void* __fastcall PEHook(Unreal::UObject* _Obj, Unreal::UObject* Obj, Unreal::UObject* Func, void* Params) {
		if (_Obj->IsValid() && Func->IsValid()) {
			std::string FuncName = Func->GetName();
#ifdef DEBUG
			if (FuncName != "/Script/Engine.Actor:ReceiveBeginPlay" && FuncName != "/Script/Engine.Actor:ReceiveTick" && !FuncName.contains("UserConstructionScript")) {
				PE_Log << "ObjName: " << _Obj->GetName() << " FuncName: " << FuncName << std::endl;
			}
#endif

			if (FuncName == "/Script/Engine.GameMode:ReadyToStartMatch" && InGame == false && Globals::GameMode->IsValid()) {
				InGame = true;
				MessageBoxA(0, "RTSM", "A", MB_OK);
				Setup();
			}

			if (FuncName == "/game/UI/Global_Elements/UIManager.UIManager_C:Construct") {
				bRealPawn = true;
				Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:Suicide"));
			}

			if (GetAsyncKeyState(VK_F1) & 0x1) {
				DumpObjects();
				Sleep(1000);
			}	
		}
		return Unreal::ProcessEventOG(_Obj, Obj, Func, Params);
	}
	Unreal::UObject* Team = nullptr;

	//Scuffed Af
	Unreal::UObject* __fastcall SpawnActor_Hk(Unreal::UObject* InWorld, Unreal::UObject* Class, Unreal::UObject* Class1, void* Transform, void* SpawnParameters) {
		static auto PlayerStateClass = ("/Script/FortniteGame.FortPlayerState");
		static auto PlayerControllerClass = ("/Script/FortniteGame.FortPlayerController");
		static auto GameStateClass = ("/Script/FortniteGame.FortGameState");
		std::string ClassName = Class1->GetName();
		//Override Classes
		if (ClassName == "/Script/Engine.SpectatorPawn") {
			Class1 = FindObject("/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C");
		}
		else if (ClassName == PlayerControllerClass) {
			Class1 = FindObject("/Script/FortniteGame.FortPlayerControllerPvP");
		}
		else if (ClassName == PlayerStateClass) {
			Class1 = FindObject("/Script/FortniteGame.FortPlayerStatePvP");
		}
		else if (ClassName == GameStateClass) {
			Class1 = FindObject("/Script/FortniteGame.FortGameStatePvP");
		}
		else if (ClassName == "/Script/FortniteGame.FortUIZone") {
			Class1 = FindObject("/Script/FortniteGame.FortUIPvP");
		}
#ifdef DEBUG
		SA_Log << "Class: " << ClassName << std::endl;
#endif
		//Spawn the Actor
		auto ret = SpawnActor_OG(InWorld, Class, Class1, Transform, SpawnParameters);
		if (!ret->IsValid()) return ret;
		//Setup extra stuff
		if (ClassName == "/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C") {
			Globals::GPawn = ret;
			SetupPawn();
		}
		else if (ClassName == "/Script/FortniteGame.FortGameMode") Globals::GameMode = ret;
		else if (ClassName == GameStateClass) Globals::GameState = ret;
		return ret;
	}
}

namespace Core {
	void Init() {
		GObjs = reinterpret_cast<Unreal::FUObjectArray*>(Memory::GetAddressFromOffset(0x2DC3AAC));
		uintptr_t PE_Addr = Memory::GetAddressFromOffset(0x9A9E70);
		Unreal::GetPathName = decltype(Unreal::GetPathName)(Memory::GetAddressFromOffset(0x132C790));
		Unreal::Free = decltype(Unreal::Free)(Memory::GetAddressFromOffset(0x89CAD0));
		Unreal::SLO = decltype(Unreal::SLO)(Memory::GetAddressFromOffset(0x9AFBD0));
		uintptr_t SA_Addr = Memory::GetAddressFromOffset(0x1352D70);
		Globals::GEngine = FindObject("/Engine/Transient.FortEngine_0");
		DumpObjects();
		Functions::GetPC();
		if (MH_Initialize() != MH_STATUS::MH_OK) {
			MessageBoxA(0, "MH Failed!", "ERROR", MB_OK);
		}
		MH_CreateHook((void**)SA_Addr, Hooks::SpawnActor_Hk, (void**)&SpawnActor_OG);
		MH_EnableHook((void**)SA_Addr);
		MH_CreateHook((void**)PE_Addr, Hooks::PEHook, (void**)&Unreal::ProcessEventOG);
		MH_EnableHook((void**)PE_Addr);
		MessageBoxA(0, "Press OK to Load In-Game!", "Alpharium", MB_OK);
		Unreal::FString Map = L"PvP_Tower?Game=FortniteGame.FortGameMode";
		Globals::GPC->ProcessEvent(FindObject("/Script/Engine.PlayerController:SwitchLevel"), &Map);
		Functions::SetupConsole();
	}
}