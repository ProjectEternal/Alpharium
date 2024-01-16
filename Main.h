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

enum class EFortTeam
{
	HumanCampaign = 0,
	Monster = 1,
	HumanPvP_Team1 = 2,
	HumanPvP_Team2 = 3,
	HumanPvP_Team3 = 4,
	HumanPvP_Team4 = 5,
	HumanPvP_Team5 = 6,
	HumanPvP_Team6 = 7,
	HumanPvP_Team7 = 8,
	HumanPvP_Team8 = 9,
	HumanPvP_Team9 = 10,
	HumanPvP_Team10 = 11,
	Spectator = 12,
	MAX = 13,
	EFortTeam_MAX = 14
};

enum class EFortGameplayState
{
	NormalGameplay = 0,
	WaitingToStart = 1,
	EndOfZone = 2,
	EnteringZone = 3,
	LeavingZone = 4,
	Invalid = 5,
	EFortGameplayState_MAX = 6
};

enum class EMovementMode : uint8_t
{
	MOVE_None = 0,
	MOVE_Walking = 1,
	MOVE_NavWalking = 2,
	MOVE_Falling = 3,
	MOVE_Swimming = 4,
	MOVE_Flying = 5,
	MOVE_Custom = 6,
	MOVE_MAX = 7
};

enum class ENetRole
{
	ROLE_None = 0,
	ROLE_SimulatedProxy = 1,
	ROLE_AutonomousProxy = 2,
	ROLE_Authority = 3,
	ROLE_MAX = 4
};

enum class EComponentMobility
{
	Static = 0,
	Stationary = 1,
	Movable = 2,
	EComponentMobility_MAX = 3
};

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

void SetupPawn() {
	Functions::GetPC();
	/*auto Class = FindObject("/Script/FortniteGame.FortCombatManager");
	auto CoM = SpawnActor_OG(Functions::GetWorld(), Class, Class, {}, {});
	*Finder::Find(Globals::GPC, "CombatManager") = CoM;
	Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_CombatManager"));*/
	//static auto GAS = Unreal::SLO(FindObject("/Script/FortniteGame.FortAbilitySet"), nullptr, L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer", 0, 0, 0, 0);
	//DumpObjects();
	//Unreal::UObject* PS = *Finder::Find(Globals::GPC, "PlayerState");
	Globals::GPC->ProcessEvent(FindObject("/Script/Engine.Controller:Possess"), &Globals::GPawn);
	MessageBoxA(0, "Possesed", "Debug", MB_OK);
	Unreal::UObject* CM = Functions::SetupCM();
	CM->ProcessEvent(FindObject("/Script/Engine.CheatManager:God"));
	MessageBoxA(0, "God Mode", "Debug", MB_OK);
	Unreal::FString Gender = L"Male";
	Globals::GPawn->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerPawn:RandomizeOutfit"), &Gender);
	(*Finder::Find(Globals::GPawn, "PlayerState"))->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerState:OnRep_CharacterParts"));
	//Globals::GPawn->ProcessEvent(FindObject("/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C:OnCharacterPartsReinitialized"));
	Globals::GPawn->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerPawn:ToggleGender"));
	MessageBoxA(0, "Skin Set", "Debug", MB_OK);
	auto MovementSet = (*Finder::Find(Globals::GPawn, "MovementSet"));
	*Finder::Find<float*>(MovementSet, "BackwardSpeedMultiplier") = 0.65f;
	*Finder::Find<float*>(MovementSet, "WalkSpeed") = 200.0f;
	*Finder::Find<float*>(MovementSet, "RunSpeed") = 410.0f;
	*Finder::Find<float*>(MovementSet, "SprintSpeed") = 550.0f;
	//MovementSet->ProcessEvent(FindObject("/Script/FortniteGame.FortMovementSet:OnRep_SpeedMultiplier"));
	MessageBoxA(0, "MS Setup", "Debug", MB_OK);
	auto AttrSet = FindObject("_0.PlayerAttrSet", false);
	*Finder::Find<float*>(AttrSet, "StaminaRegenDelay") = 0.5f;
	*Finder::Find<float*>(AttrSet, "StaminaRegenRate") = 0.65f;
	*Finder::Find<float*>(AttrSet, "Stamina") = 100.0f;
	*Finder::Find<float*>(AttrSet, "MaxStamina") = 100.0f;
	AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_Stamina"));
	AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_MaxStamina"));
	AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_StaminaRegenDelay"));
	AttrSet->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_StaminaRegenRate"));
	MessageBoxA(0, "AS Setup", "Debug", MB_OK);
	/*int CurrentValue_Offset = Finder::GetPropByClass(FindObject(""), "CurrentValue");
	*Finder::Find<float*>(Finder::Find<void*>(MovementSet, "SpeedMultiplier"),"CurrentValue") = 1.0f;
	Finder::Find<void*>(MovementSet, "WalkSpeed") = 1.1f;
	Finder::Find<void*>(MovementSet, "RunSpeed") = 1.5f;
	MovementSet->ProcessEvent(FindObject("/Script/FortniteGame.FortMovementSet:OnRep_SpeedMultiplier"));*/

	//Grant Abilities and Effects
	/*if (GAS->IsValid()) {
		auto GameplayAbilities = Finder::Find<Unreal::TArray<Unreal::UObject*>*>(GAS, "GameplayAbilities");
		for (int i = 0; i < GameplayAbilities->Num(); i++) {
			GrantAbility(GameplayAbilities->Data[i]);
		}
	}*/

	/*GrantAbility(FindObject("/Script/FortniteGame.FortGameplayAbility_Jump"));
	GrantAbility(FindObject("/Script/FortniteGame.FortGameplayAbility_Sprint"));*/
	//GrantEffect(Unreal::SLO(FindObject("/Script/Engine.BlueprintGeneratedClass"), nullptr, L"/Game/TheManor/GE_Manor_Character_Move_Infinite_Stamina.GE_Manor_Character_Move_Infinite_Stamina_C", 0, 0, 0, 0));
	/*GrantEffect(Unreal::SLO(FindObject("/Script/Engine.BlueprintGeneratedClass"), nullptr, L"/Game/Abilities/Player/Alterations/GE_Alteration_Attribute_MoveSpeed.GE_Alteration_Attribute_MoveSpeed_C", 0, 0, 0, 0));
	GrantEffect(Unreal::SLO(FindObject("/Script/Engine.BlueprintGeneratedClass"), nullptr, L"/Game/Abilities/Player/Alterations/GE_Alteration_Attribute_SprintSpeed.GE_Alteration_Attribute_SprintSpeed_C", 0, 0, 0, 0));
	GrantEffect(Unreal::SLO(FindObject("/Script/Engine.BlueprintGeneratedClass"), nullptr, L"/Game/Abilities/Player/Alterations/GE_Alteration_Attribute_StaminaCostReduction.GE_Alteration_Attribute_StaminaCostReduction_C", 0, 0, 0, 0));
	GrantEffect(Unreal::SLO(FindObject("/Script/Engine.BlueprintGeneratedClass"), nullptr, L"/Game/Abilities/Player/Alterations/GE_Alteration_Attribute_JumpHeight.GE_Alteration_Attribute_JumpHeight_C", 0, 0, 0, 0));
	GrantEffect(Unreal::SLO(FindObject("/Script/Engine.BlueprintGeneratedClass"), nullptr, L"/Game/TheManor/GE_Manor_Character_Move_Infinite_Stamina.GE_Manor_Character_Move_Infinite_Stamina_C", 0, 0, 0, 0));*/

	/*auto PAS = FindObject("_0.PlayerAttrSet", false);

	*Finder::Find<float*>(PAS, "MaxStamina") = 9999.0f;
	PAS->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_MaxStamina"));
	*Finder::Find<float*>(PAS, "Stamina") = 9999.0f;
	PAS->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerAttrSet:OnRep_Stamina"));*/

	DumpObjects();

	//auto MovementSet = *Finder::Find(Globals::GPawn, "MovementSet");
	//*Finder::Find<float*>(MovementSet, "WalkSpeed") = 1.0f;
	//*Finder::Find<float*>(MovementSet, "SpeedMultiplier") = 1.1f;

	//MovementSet->ProcessEvent(FindObject("/Script/FortniteGame.FortMovementSet:OnRep_SpeedMultiplier"));

	//auto IC = Finder::Find(Globals::GPawn, "InputComponent");
	//auto CMC = *Finder::Find(Globals::GPawn, "CharacterMovement");
	//MessageBoxA(0, (*IC)->GetName().c_str(), "InputComponent", MB_OK);
	//MessageBoxA(0, std::string(CMC->Class->GetName() + CMC->GetName()).c_str(), "CharacterMovement", MB_OK);
	//*IC = CMC;
	//(*IC)->ProcessEvent(FindObject("/Script/Engine.ActorComponent:Deactivate"));
}

void Setup() {
	//auto PawnClass = FindObject("/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C");
	//Globals::GPawn = SpawnActor_OG(Functions::GetWorld(), PawnClass, PawnClass, new Unreal::FTransform(Unreal::FVector{ 3577.5227,-537.3214,-1444.0 }), {});
	//MessageBoxA(0, "Pawn Spawned!", "Debug", MB_OK);
	////Basic Player Setup
	//SetupPawn();
	//MessageBoxA(0, "Pawn Setup!", "Debug", MB_OK);
	Functions::GetPC();
	//Globals::GameMode->ProcessEvent(FindObject("/Script/Engine.GameMode:StartMatch"));
	*Finder::Find<bool*>(Globals::GPC, "bHasClientFinishedLoading") = true;
	*Finder::Find<bool*>(Globals::GPC, "bHasServerFinishedLoading") = true;
	*Finder::Find<bool*>(Globals::GPC, "bReadyToStartMatch") = true;
	*Finder::Find<bool*>(Globals::GPC, "bClientPawnIsLoaded") = true;
	*Finder::Find<bool*>(Globals::GPC, "bHasInitiallySpawned") = true;
	Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_bHasServerFinishedLoading"));
	MessageBoxA(0, "PC Setup!", "Debug", MB_OK);
	DumpObjects();
}

namespace Hooks {
	std::ofstream PE_Log("PE.txt");
	void* __fastcall PEHook(Unreal::UObject* _Obj, Unreal::UObject* Obj, Unreal::UObject* Func, void* Params) {
		if (_Obj->IsValid() && Func->IsValid()) {
			std::string FuncName = Func->GetName();
			if (FuncName != "/Script/Engine.Actor:ReceiveBeginPlay" && FuncName != "/Script/Engine.Actor:ReceiveTick" && !FuncName.contains("UserConstructionScript")) {
				PE_Log << "ObjName: " << _Obj->GetName()/* << " _ObjName: " << _Obj->GetName()*/ << " FuncName: " << FuncName << std::endl;
			}

			if (FuncName == "/Script/Engine.GameMode:ReadyToStartMatch" && InGame == false && Globals::GameMode->IsValid()) {
				InGame = true;
				MessageBoxA(0, "RTSM", "A", MB_OK);
				Setup();
			}

			if (GetAsyncKeyState(VK_F1) & 0x1) {
				DumpObjects();
				Sleep(1000);
			}	
		}
		return Unreal::ProcessEventOG(_Obj, Obj, Func, Params);
	}
	Unreal::UObject* Team = nullptr;

	//Scuffed In Game
	std::ofstream SA_Log("SA.txt");
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
		SA_Log << "Class: " << ClassName << std::endl;
		//SA_Log << "Class2: " << Class->GetName() << std::endl;
		//Spawn the Actor
		auto ret = SpawnActor_OG(InWorld, Class, Class1, Transform, SpawnParameters);
		if (!ret->IsValid()) return ret;
		//Setup extra stuff
		if (ClassName == "/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C") {
			Globals::GPawn = ret;
			SetupPawn();
		}
		else if (ClassName == "/Script/FortniteGame.FortGameMode") {
			Globals::GameMode = ret;
			/*auto TIC = FindObject("/Script/FortniteGame.FortTeamInfoPvPBaseDestruction");
			Team = SpawnActor_OG(InWorld, TIC, TIC, {}, {});
			*Finder::Find<EFortTeam*>(Team, "Team") = EFortTeam::HumanPvP_Team1;*/
		}
		else if (ClassName == GameStateClass) {
			Globals::GameState = ret;
			/**Finder::Find<int*>(Globals::GameState, "TeamSize") = 4;
			*Finder::Find<int*>(Globals::GameState, "TeamCount") = 2;*/
			/*auto WM_Class = FindObject("/Script/FortniteGame.FortWorldManager");
			*Finder::Find(ret, "WorldManager") = SpawnActor_OG(Functions::GetWorld(), WM_Class, WM_Class, {}, {});
			ret->ProcessEvent(FindObject("/Script/FortniteGame.FortGameState:OnRep_WorldManager"));*/
		}
		else if (ClassName == PlayerStateClass) {
			Functions::GetPC();
			/*if (Team->IsValid()) {
				*Finder::Find(ret, "PlayerTeam") = Team;
				Finder::Find<Unreal::TArray<Unreal::UObject*>*>(Team, "TeamMembers")->Add(Globals::GPC);
				ret->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerState:OnRep_PlayerTeam"));
			}*/
			/**Finder::Find(ret, "HeroType") = FindObject("/Game/Heroes/Class_Commando.Class_Commando");
			ret->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerState:OnRep_HeroType"));*/
		}
		return ret;
	}
}

namespace Core {
	void Init() {
		/*AllocConsole();
		FILE* pFile;
		freopen_s(&pFile, ("CONOUT$"), "w", stdout);*/
		GObjs = reinterpret_cast<Unreal::FUObjectArray*>(Memory::GetAddressFromOffset(0x2DC3AAC));
		uintptr_t PE_Addr = Memory::GetAddressFromOffset(0x9A9E70);
		//Unreal::ProcessEventOG = decltype(Unreal::ProcessEventOG)(PE_Addr);
		Unreal::GetPathName = decltype(Unreal::GetPathName)(Memory::GetAddressFromOffset(0x132C790));
		Unreal::Free = decltype(Unreal::Free)(Memory::GetAddressFromOffset(0x89CAD0));
		Unreal::SLO = decltype(Unreal::SLO)(Memory::GetAddressFromOffset(0x9AFBD0));
		uintptr_t SA_Addr = Memory::GetAddressFromOffset(0x1352D70);
		//SpawnActor_OG = decltype(SpawnActor_OG)(SA_Addr);
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