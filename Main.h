#pragma once
#include "pch.h"
#include "Memory.h"
#include "Functions.h"
#include <cstdarg>
#include <regex>
#include "MinHook.h"
#pragma comment(lib,"minhook.lib")
bool Ready = false;
bool InGame = false;

#ifdef DEBUG
std::ofstream SA_Log("SA.txt");
#endif
std::ofstream PE_Log("PE.txt");

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

void GrantEffect(Unreal::UObject* Pawn, Unreal::UObject* Effect) {
	if (!Effect) {
		return;
	}

	Unreal::UObject* ASC = *Finder::Find(Pawn, "AbilitySystemComponent");
	struct
	{
		Unreal::UObject* GameplayEffectClass;
		float Level;
		FGameplayEffectContextHandle EffectContext;
		FActiveGameplayEffectHandle ret;
	} params{ Effect, 1.0f, {}, {} };

	ASC->ProcessEvent(FindObject("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectToSelf"), &params);
}

void GrantAbility(Unreal::UObject* Pawn, Unreal::UObject* Ability) {
	if (!Ability) {
		return;
	}

	Unreal::UObject* ASC = *Finder::Find(Pawn, "AbilitySystemComponent");

	static Unreal::UObject* GE_Class = nullptr;
	if (GE_Class == nullptr) {
		GE_Class = FindObject("/Game/Abilities/Player/Constructor/Traits/CreativeEngineering/GE_GrantReflectMeleeDmg.GE_GrantReflectMeleeDmg_C");
	}

	static Unreal::UObject* DefaultGE_Class = nullptr;
	if (DefaultGE_Class == nullptr) {
		DefaultGE_Class = FindObject("/Game/Abilities/Player/Constructor/Traits/CreativeEngineering/GE_GrantReflectMeleeDmg.Default__GE_GrantReflectMeleeDmg_C");
	}

	Finder::Find<Unreal::TArray<FGameplayAbilitySpecDef>*>(DefaultGE_Class, "GrantedAbilities")->Data[0].Ability = Ability;

	GrantEffect(Pawn, GE_Class);
}

bool bRealPawn = false;

void God(Unreal::UObject* Pawn) {
	*Finder::Find<bool*>(Pawn, "bCanBeDamaged") = false;
}

void SetupPawn(Unreal::UObject* PC, Unreal::UObject* Pawn) {
	PC->ProcessEvent(FindObject("/Script/Engine.Controller:Possess"), &Pawn);;
	God(Pawn);
	if (true) {
		//Character Parts
		Unreal::FString Gender = L"Male";
		Pawn->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerPawn:RandomizeOutfit"), &Gender);
		(*Finder::Find(Pawn, "PlayerState"))->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerState:OnRep_CharacterParts"));
		Pawn->ProcessEvent(FindObject("/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C:OnCharacterPartsReinitialized"));
		Pawn->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerPawn:ToggleGender"));

		//Movement Fix
		auto MovementSet = (*Finder::Find(Pawn, "MovementSet"));
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

		//bilities
		GrantAbility(Pawn, FindObject("/Script/FortniteGame.FortGameplayAbility_Jump"));
		GrantAbility(Pawn, FindObject("/Script/FortniteGame.FortGameplayAbility_Sprint"));
		GrantAbility(Pawn, Unreal::StaticLoadObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GA_DefaultPlayer_InteractSearch.GA_DefaultPlayer_InteractSearch_C", FindObject("/Script/Engine.BlueprintGeneratedClass")));
		GrantAbility(Pawn, Unreal::StaticLoadObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GA_DefaultPlayer_InteractUse.GA_DefaultPlayer_InteractUse_C", FindObject("/Script/Engine.BlueprintGeneratedClass")));
		GrantAbility(Pawn, Unreal::StaticLoadObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GA_DefaultPlayer_Consumable.GA_DefaultPlayer_Consumable_C", FindObject("/Script/Engine.BlueprintGeneratedClass")));

		//Show Map (TODO)
		//CM->ProcessEvent(FindObject("/Script/FortniteGame.FortCheatManager:UncoverMap"));
	}
}

void SetupMatch() {
	*Finder::Find<bool*>(Globals::GameMode, "bTravelInitiated") = true;
	*Finder::Find<bool*>(Globals::GameMode, "bWorldIsReady") = true;
	*Finder::Find<bool*>(Globals::GameMode, "bTeamGame") = true;

	*Finder::Find<int*>(Globals::GameState, "WorldLevel") = 1;
	*Finder::Find<float*>(Globals::GameState, "GameDifficulty") = 1.0f;

	Globals::GameState->ProcessEvent(FindObject("/Script/FortniteGame.FortGameState:OnRep_GameplayState"));
	Globals::GameState->ProcessEvent(FindObject("/Script/FortniteGame.FortGameState:OnRep_WorldManager"));

	Globals::GameMode->ProcessEvent(FindObject("/Script/Engine.GameMode:StartMatch"));
	Globals::GameMode->ProcessEvent(FindObject("/Script/Engine.GameMode:StartPlay"));
}

void SetupPC(Unreal::UObject* PC) {
	*Finder::Find<bool*>(PC, "bHasClientFinishedLoading") = true;
	*Finder::Find<bool*>(PC, "bHasServerFinishedLoading") = true;
	BitField* FPC_BF = Finder::Find<BitField*>(PC, "bFailedToRespawn");
	FPC_BF->D = FPC_BF->F = FPC_BF->G = true;
	*Finder::Find<bool*>(PC, "bClientPawnIsLoaded") = true;
	*Finder::Find<bool*>(PC, "bHasInitiallySpawned") = true;

	PC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_bHasServerFinishedLoading"));

	PC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:ServerReadyToStartMatch"));
}

__int64 patch() {
	return 1;
}

//void(__thiscall* MiscCrashOG)(void*, char*);
//void MiscCrash(void* a1, char* Str) {
//	const char* Str2 = "Ok";
//	if (Str) {
//		MessageBoxA(0, "MiscCrash", "TEST", 0);
//		void* v4 = reinterpret_cast<void* (*)(void*, void*, void*)>(Memory::GetAddressFromOffset(0x899D20))(a1, Str, &Str2);
//		MessageBoxA(0, Str2, "TEST", MB_OK);
//
//		if (v4) return MiscCrashOG(a1, Str);
//	}
//	return;
//	//PseudoCode
//	char* v2 = Str;
//	if (Str) {
//		void* v4 = 0;//sub_899D20(a1,(void*)Str, (void*)&Str);
//		short v5 = *reinterpret_cast<short*>(__int64(v4) + 2); //CRASH
//	}
//}

namespace Server {
	struct FURL {
		Unreal::FString Protocol;
		Unreal::FString Host;
		int Port;
		Unreal::FString Map;
		Unreal::FString RedirectURL;
		Unreal::TArray<Unreal::FString> Op;
		Unreal::FString Portal;
		int Valid;
	};

	Unreal::UObject* BeaconHost;
	Unreal::UObject* NetDriver;

	void* (*DispatchReqOG)(void* a1, void* RequestContent);
	void* DispatchReqHook(void* a1, void* RequestContent) {
		(*reinterpret_cast<int*>((__int64(RequestContent)) + 5)) = 3;
		return DispatchReqOG(a1, RequestContent);
	}

	//TODO: Handle bOnlyRelevantToOwner Actors
	bool IsReplicableActor(Unreal::AActor* Actor) {
		static int Offset = Finder::GetPropByClass(FindObject("/Script/Engine.Actor"), "RemoteRole");
		if (!Actor->IsValid()) return false; //Invalid Actor
		else if (Actor->bAlwaysRelevant()) return true; //Always Relevant
		else if (*reinterpret_cast<uint8_t*>(__int64(Actor) + Offset) != 0) return true;
		else return false;
	}

	Unreal::UObject* GetOrInitCh(Unreal::UObject* Client, Unreal::UObject* Actor) {
		if (!Client->IsValid() || !Actor->IsValid()) return nullptr;

		//Search for an existing channel
		Unreal::TArray<Unreal::UObject*>* OpenChannels = Finder::Find<Unreal::TArray<Unreal::UObject*>*>(Client, "OpenChannels");

		for (int i = 0; i < OpenChannels->Num(); i++) {
			Unreal::UObject* Ch = OpenChannels->At(i);

			if (Ch->IsValid() && Ch->Class == FindObject("/Script/Engine.ActorChannel") && *Finder::Find(Ch, "Actor") == Actor) return Ch;
		}

		//Create one if possible
		Unreal::UObject* Ret = reinterpret_cast<Unreal::UObject * (__thiscall*)(Unreal::UObject*, int, bool, int)>(Memory::GetAddressFromOffset(Offsets::UNetConnection::CreateChannel))(Client, 2, true, -1);
		if (Ret) {
			reinterpret_cast<void(__thiscall*)(Unreal::UObject*, Unreal::UObject*)>(Memory::GetAddressFromOffset(Offsets::UChannel::SetChannelActor))(Ret, Actor);
			*Finder::Find(Ret, "Connection") = Client;
		}

		return Ret;
	}
	#define REP
	void ServerReplicateActors() {
		if (!NetDriver->IsValid()) return;
		++*(DWORD*)(__int64(NetDriver) + Offsets::UNetDriver::ReplicationFrame);
		Unreal::TArray<Unreal::UObject*>* Connections = Finder::Find< Unreal::TArray<Unreal::UObject*>*>(NetDriver, "ClientConnections");

		if (Connections->Num() > 0 && Connections->IsValid(0) && Connections->At(0)->IsValid() && *Finder::Find<bool*>(Connections->At(0), "InternalAck") == false) {
			std::vector<Unreal::UObject*> RepActors;
#ifdef REP
			struct {
				Unreal::UObject* World;
				Unreal::UObject* Class;
				Unreal::TArray<Unreal::AActor*> Out;
			}params{ Functions::GetWorld(), FindObject("/Script/Engine.Actor") };
			
			FindObject("/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:GetAllActorsOfClass"), &params);
			Unreal::TArray<Unreal::AActor*> WorldActors = params.Out;
			for (int i = 0; i < WorldActors.Num(); i++) {
				Unreal::AActor* Actor = WorldActors.At(i);

				if (Actor->IsValid() && IsReplicableActor(Actor)) RepActors.push_back(Actor);
			}
#endif

			for (int i = 0; i < Connections->Num(); i++) {
				Unreal::UObject* Client = Connections->At(i);
				if (!Client->IsValid()) continue;

				Unreal::UObject* OwningActor = *Finder::Find(Client, "OwningActor");
				Unreal::UObject* PlayerController = *Finder::Find(Client, "PlayerController");
				Unreal::UObject** ViewTarget = Finder::Find(Client, "ViewTarget");

				if (PlayerController->IsValid() && OwningActor->IsValid()) PlayerController->ProcessEvent(FindObject("/Script/Engine.Controller:GetViewTarget"), ViewTarget);
				else if (OwningActor->IsValid()) *ViewTarget = OwningActor;
				else *ViewTarget = nullptr;

				if (!(*ViewTarget)->IsValid()) continue;

				if (PlayerController->IsValid()) reinterpret_cast<void(__thiscall*)(Unreal::UObject * PC)>(Memory::GetAddressFromOffset(Offsets::APlayerController::SendClientAdjustment))(PlayerController);
#ifdef REP
				for (Unreal::UObject* Actor : RepActors) {
					if (!Actor->IsValid()) continue;

					reinterpret_cast<void(__thiscall*)(Unreal::UObject*, Unreal::UObject*)>(Memory::GetAddressFromOffset(Offsets::AActor::CallPreReplication))(Actor, NetDriver);

					Unreal::UObject* Ch = GetOrInitCh(Client, Actor);
					if (Ch) reinterpret_cast<bool(__thiscall*)(Unreal::UObject*)>(Memory::GetAddressFromOffset(Offsets::UChannel::ReplicateActor))(Ch);
				}
#endif
			}
		}
	}

	void* (__thiscall* TickFlushOG)(Unreal::UObject*, int);

	void* TickFlush(Unreal::UObject* ND, int DeltaSeconds) {
		if (NetDriver->IsValid() && ND == NetDriver) {
			ServerReplicateActors();
		}

		return TickFlushOG(ND, DeltaSeconds);
	}

	void InitRep() {
		MessageBoxA(0, "Listening", "KMS", MB_OK);
		uintptr_t Reservation = Memory::GetAddressFromOffset(Offsets::Misc::ReservationFailure);
		MH_CreateHook((void*)Reservation, patch, nullptr);
		MH_EnableHook((void*)Reservation);
		
		uintptr_t Validation = Memory::GetAddressFromOffset(Offsets::Misc::ValidationFailure);
		MH_CreateHook((void*)Validation, patch, nullptr);
		MH_EnableHook((void*)Validation);
		
		uintptr_t Kick = Memory::GetAddressFromOffset(Offsets::Misc::KickPlayer);
		MH_CreateHook((void*)Kick, patch, nullptr);
		MH_EnableHook((void*)Kick);

		while (true) {
			Sleep(5000);
			ServerReplicateActors();
		}
	}

	void Listen() {
		BeaconHost = Functions::SpawnActor(FindObject("/Script/OnlineSubsystemUtils.OnlineBeaconHost"));
		if (BeaconHost->IsValid()) {
			if (!reinterpret_cast<bool(__thiscall*)(Unreal::UObject*)>(Memory::GetAddressFromOffset(Offsets::AOnlineBeacon::InitHost))(BeaconHost)) return (void)MessageBoxA(0, "BeaconFailed", "Bruh", MB_OK);
			MessageBoxA(0, "Beacon Listening", "KMS", MB_OK);
			NetDriver = *Finder::Find(BeaconHost, "NetDriver");
			if (NetDriver->IsValid()) {
				MessageBoxA(0, "Beacon NetDriver Valid", "KMS", MB_OK);

				(*Finder::Find<Unreal::FName*>(NetDriver, "NetDriverName")) = Unreal::FName(282);
				*Finder::Find(NetDriver, "World") = Functions::GetWorld();
				*Finder::Find(Functions::GetWorld(), "NetDriver") = NetDriver;
				MessageBoxA(0, "GameDriverName Set", "KMS", MB_OK);

				FURL URL;

				URL.Port = 7777;

				Unreal::FString Err;

				if (!reinterpret_cast<bool(__thiscall*)(Unreal::UObject * ND, void* NF, FURL&, bool, Unreal::FString&)>(Memory::GetAddressFromOffset(Offsets::UNetDriver::InitListen))(NetDriver, Functions::GetWorld(), URL, false, Err))
				{
					MessageBoxA(0, "Server Failed to Start", "KMS", MB_OK);
				}
				
				reinterpret_cast<void(__thiscall*)(Unreal::UObject*, Unreal::UObject*)>(Memory::GetAddressFromOffset(Offsets::UNetDriver::SetWorld))(NetDriver, Functions::GetWorld());

				MessageBoxA(0, "SetWorld", "KMS", MB_OK);

				CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitRep, 0, 0, 0);
			}
		}
	}
}

namespace Hooks {
	//Scuffed Af
	Unreal::UObject* __fastcall SpawnActor_Hk(Unreal::UObject* InWorld, Unreal::UObject* Class, Unreal::UObject* Class1, Unreal::FTransform* Loc, const FActorSpawnParameters& SpawnParameters) {
		static auto PlayerStateClass = ("/Script/FortniteGame.FortPlayerState");
		static auto PlayerControllerClass = ("/Script/FortniteGame.FortPlayerController");
		static auto GameStateClass = ("/Script/FortniteGame.FortGameState");
		std::string ClassName = Class1->GetName();
		//Override Classes
		if (ClassName == "/Script/Engine.SpectatorPawn") {
			Class1 = FindObject("/Game/Abilities/Player/Pawns/PlayerPawn_Generic.PlayerPawn_Generic_C");
		}
		else if (ClassName == PlayerControllerClass) {
			Class1 = FindObject("/Script/FortniteGame.FortPlayerControllerZone");
		}
		else if (ClassName == PlayerStateClass) {
			Class1 = FindObject("/Script/FortniteGame.FortPlayerStateZone");
		}
		else if (ClassName == GameStateClass) {
			Class1 = FindObject("/Script/FortniteGame.FortGameStateZone");
		}
#ifdef DEBUG
		SA_Log << "Class: " << ClassName << std::endl;
#endif
		//Spawn the Actor
		auto ret = SpawnActor_OG(InWorld, Class, Class1, Loc, SpawnParameters);
		if (!ret->IsValid()) return ret;
		//Setup extra stuff
		else if (ClassName == "/Script/FortniteGame.FortGameMode") Globals::GameMode = ret;
		else if (ClassName == GameStateClass) Globals::GameState = ret;
		return ret;
	}

	void* __fastcall PEHook(Unreal::UObject* _Obj, Unreal::UObject* Obj, Unreal::UObject* Func, void* Params) {
		if (_Obj->IsValid() && Func->IsValid() && Ready) {
			std::string FuncName = Func->GetName();
#ifdef DEBUG
			if (!FuncName.contains(":Receive") && !FuncName.contains("ServerTriggerCombatEvent") && !FuncName.contains("ActorBlueprints") && !FuncName.contains("ServerFireAIDirectorEvent") && !FuncName.contains("BuildingActor:") && !FuncName.contains(":ReadyTo") && !FuncName.contains("BuildingSMActor:") && !FuncName.contains("K2Node") && !FuncName.contains("ExecuteUbergraph") && !FuncName.contains("FortDayNightLightingAndFog:") && !FuncName.contains("AnimInstance") && !FuncName.contains(":Blueprint") && !FuncName.contains("EvaluateGraph") && !FuncName.contains(".CharacterMesh") && !FuncName.contains("BlueprintModify") && !FuncName.contains("/Script/Engine.HUD") && !FuncName.contains("OnMatchStarted") && !FuncName.contains("Construct") && !FuncName.contains("/Script/UMG.") && !FuncName.contains("/Game/UI/")) {
				PE_Log << "ObjName: " << _Obj->GetName() << " FuncName: " << FuncName << std::endl;
			}
#endif
			//MP
			if (FuncName == "/Script/Engine.GameMode:K2_PostLogin" && InGame) {
				Unreal::UObject* PC = *reinterpret_cast<Unreal::UObject**>(Params);

				if (PC->IsValid() && Obj->GetName() == "/Game/Maps/PVP/PVP_Tower.PVP_Tower:PersistentLevel.FortGameMode_0") {
					SetupPC(PC);
					if (*Finder::Find(PC, "Pawn")) {
						SetupPawn(PC, *Finder::Find(PC, "Pawn"));
					}
					else {
						MessageBoxA(0, "Pawn Not Setup", "OTMP", 0);
					}
				}
			}

			if (FuncName == "/Script/Engine.PlayerController:ClientReturnToMainMenu") {
				return 0;
			}

			//Gameplay

			if (FuncName == "/Script/Engine.GameMode:ReadyToStartMatch" && InGame == false) {
				InGame = true;
				Sleep(1000);
				if (Globals::GameMode->IsValid()) {
					SetupMatch();
				}
				else {
					_Obj->ProcessEvent(FindObject("/Script/Engine.GameMode:StartMatch"));
					_Obj->ProcessEvent(FindObject("/Script/Engine.GameMode:StartPlay"));
				}
			}

			if (FuncName == "/Script/FortniteGame.FortCheatManager:CheatScript") {
				std::string Str = reinterpret_cast<Unreal::FString*>(Params)->ToString();

				if (Str == "findercache") {
					Finder::DumpCache();
				}

				if (Str == "testspawn") {
					Functions::SpawnActor(FindObject("/Script/FortniteGame.FortGameState"));
				}

				return 0;
			}

			//Inventory
			//if (FuncName == "/Game/Effects/Fort_Effects/Gameplay/Pickups/B_Pickups.B_Pickups_C:OnAboutToEnterBackpack") {
			//	Unreal::UObject* Def = *Finder::Find(_Obj, "ItemDefinition");
			//	int Count = 1;
			//	if (Def->IsValid()) { 
			//		std::string ClassName = Def->Class->GetName();
			//		//TODO: Find another function
			//		if (ClassName == "/Script/FortniteGame.FortResourceItemDefinition") Count = 10;
			//		Functions::Inventory::AddItem(Def, 0, Count, 1);
			//		Functions::Inventory::Update();
			//	}
			//}

			if (FuncName == "/Script/FortniteGame.FortQuickBars:ServerActivateSlotInternal") {
				struct SAS_P {
					uint8_t QB;
					int Slot;
					float AcivateDelay;
				};

				Unreal::UObject* Pawn;
				_Obj->ProcessEvent(FindObject("/Script/Engine.Actor:GetOwner"), &Pawn);

				if (!Pawn) return 0;

				SAS_P* InParams = reinterpret_cast<SAS_P*>(Params);

				Unreal::FGuid ItemGUID = Functions::Inventory::GetItemGUID(Pawn, (bool)InParams->QB, InParams->Slot);
				Functions::Inventory::Equip(Pawn, Functions::Inventory::GetItemFromGUID(Pawn, ItemGUID), ItemGUID);
			}
			static bool bInGame = false;
			if (GetAsyncKeyState(VK_F1) & 0x1) {
				if (bInGame) {
					Server::ServerReplicateActors();
				}
				else {
					bInGame = true;
					Sleep(1000);
					Server::Listen();
				}
			}
		}
		else if (!Ready) {
			if (GetAsyncKeyState(VK_F1) & 0x1) {
				Ready = true;
				uintptr_t SA_Addr = Memory::GetAddressFromOffset(0x1352D70);
				Sleep(1000);
				MessageBoxA(0, "Press OK to Load In-Game!", "Alpharium", MB_OK);
				//Unreal::FString Map = L"AITestbed_2?Game=FortniteGame.FortGameMode";
				Unreal::FString Map = L"PvP_Tower?Game=Engine.GameMode";
				Functions::GetLocalPC()->ProcessEvent(FindObject("/Script/Engine.PlayerController:SwitchLevel"), &Map);
				auto GI = *Finder::Find(Globals::GEngine, "GameInstance");
				Finder::Find<Unreal::TArray<Unreal::UObject*>*>(GI, "LocalPlayers")->Remove(0);
				MH_CreateHook((void**)SA_Addr, Hooks::SpawnActor_Hk, (void**)&SpawnActor_OG);
				MH_EnableHook((void**)SA_Addr);
			}
		}

		return Unreal::ProcessEventOG(_Obj, Obj, Func, Params);
	}
}

namespace Core {
	void Init() {
		//Init Unreal Stuff
		GObjs = reinterpret_cast<Unreal::FUObjectArray*>(Memory::GetAddressFromOffset(0x2DC3AAC));
		uintptr_t PE_Addr = Memory::GetAddressFromOffset(0x9A9E70);
		Unreal::GetPathName = decltype(Unreal::GetPathName)(Memory::GetAddressFromOffset(0x132C790));
		Unreal::Free = decltype(Unreal::Free)(Memory::GetAddressFromOffset(0x89CAD0));
		Unreal::SLO = decltype(Unreal::SLO)(Memory::GetAddressFromOffset(0x9AFBD0));
		Functions::SpawnActor_Vectored = decltype(Functions::SpawnActor_Vectored)(Memory::GetAddressFromOffset(0x01353930));

		//Game Stuff
		Functions::Inventory::EquipWeaponData = decltype(Functions::Inventory::EquipWeaponData)(Memory::GetAddressFromOffset(0x40A8D0));
		Globals::GEngine = FindObject("/Engine/Transient.FortEngine_0");

		//Hooking
		if (MH_Initialize() != MH_STATUS::MH_OK) {
			MessageBoxA(0, "MH Failed!", "ERROR", MB_OK);
		}
		MH_CreateHook((void**)PE_Addr, Hooks::PEHook, (void**)&Unreal::ProcessEventOG);
		MH_EnableHook((void**)PE_Addr);
		MessageBoxA(0, "Welcome to Eternal!", "@GDBOI101", MB_OK); //Credit
		//Setup Frontend and Enable UE Console
		Functions::Frontend::Setup();
		Functions::SetupConsole();
	}
}