#pragma once
#include "pch.h"
#include "Memory.h"
#include "Functions.h"
#include <cstdarg>
#include <regex>
#include "MinHook.h"
#pragma comment(lib,"minhook.lib")
bool InGame = false;
void SetupThread() {
	Unreal::UObject* GS = nullptr;
	while (true) {
		GS = FindObject("/Game/Maps/Zones/TheFarmstead/Zone_Onboarding_FarmsteadFort.Zone_Onboarding_FarmsteadFort:PersistentLevel.FortGameState_0",true,false);
		Globals::GPawn = FindObject("/Game/Maps/Zones/TheFarmstead/Zone_Onboarding_FarmsteadFort.Zone_Onboarding_FarmsteadFort:PersistentLevel.PlayerPawn_Generic_C_0", true, false);
		if (GS->IsValid() && Globals::GPawn->IsValid()) {
			break;
		}
		Sleep(1000 / 30);
	}
	Globals::GPC->ProcessEvent(FindObject("/Script/Engine.Controller:Possess"), &Globals::GPawn);
	Unreal::UObject* CM = *Finder::Find(Globals::GPC, "CheatManager");
	CM->ProcessEvent(FindObject("/Script/Engine.CheatManager:God"));
	(*Finder::Find(Globals::GPawn, "PlayerState"))->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerState:OnRep_CharacterParts"));
	FindObject("/Game/Maps/Zones/TheFarmstead/Zone_Onboarding_FarmsteadFort.Zone_Onboarding_FarmsteadFort:PersistentLevel.FortGameMode_0")->ProcessEvent(FindObject("/Script/Engine.GameMode:StartMatch"));
	Functions::ToggleHUD(true);
	//Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:ServerReadyToStartMatch"));
	DumpObjects();
}
namespace Hooks {
	typedef enum CURLcode {
		OK = 0,
	};
	
	typedef enum CURLOPT {
		URL = 10002,
	};

	CURLcode(*CurlEasySetOpt)(void*, CURLOPT, ...);

	CURLcode CESO_Hk(void* data, CURLOPT option, ...) {
		va_list arg;
		va_start(arg, option);
		if (option == CURLOPT::URL) {
			std::string url = va_arg(arg, char*);
			std::regex ReplaceH("(.*).epicgames.com");
			std::cout << "URL: " << url;
			if (std::regex_search(url, std::regex(ReplaceH))) {
				url = const_cast<char*>(std::regex_replace(url, ReplaceH, "http://127.0.0.1:6923").c_str());

				return CurlEasySetOpt(data, option, url);
			}
			
		}
		return CurlEasySetOpt(data, option, arg);
	}

	void* __fastcall PEHook(Unreal::UObject* _Obj, Unreal::UObject* Obj, Unreal::UObject* Func, void* Params) {
		if (Obj->IsValid() && Func->IsValid()) {
			std::string FuncName = Func->GetName();
			if (FuncName == "/Script/Engine.GameMode:ReadyToStartMatch" && InGame == false) {
				InGame = true;
				DumpObjects();
				std::cout << "RTSM!\n";
				Functions::GetPC();
				MessageBoxA(0, "After pressing Ok, Type the commands in order to continue!\nsummon PlayerPawn_Generic_C", "Alpharium", MB_OK);
				Unreal::UObject* CM = Functions::SetupCM();
				CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetupThread, 0, 0, 0);
			}
			if (GetAsyncKeyState(VK_F1) & 0x1) {
				DumpObjects();
				Sleep(1000);
			}
			return Unreal::ProcessEventOG(_Obj, Obj, Func, Params);
		}
	}
}

namespace Core {
	void Init() {
		if (MH_Initialize() != MH_STATUS::MH_OK) {
			MessageBoxA(0, "MH Failed!", "ERROR", MB_OK);
		}
		/*AllocConsole();
		FILE* pFile;
		freopen_s(&pFile, ("CONOUT$"), "w", stdout);*/
		GObjs = reinterpret_cast<Unreal::FUObjectArray*>(Memory::GetAddressFromOffset(0x2DC3AAC));
		uintptr_t PE_Addr = Memory::GetAddressFromOffset(0x9A9E70);
		Unreal::ProcessEventOG = decltype(Unreal::ProcessEventOG)(PE_Addr);
		Unreal::GetPathName = decltype(Unreal::GetPathName)(Memory::GetAddressFromOffset(0x132C790));
		Unreal::Free = decltype(Unreal::Free)(Memory::GetAddressFromOffset(0x89CAD0));
		Globals::GEngine = FindObject("/Engine/Transient.FortEngine_0");
		DumpObjects();
		Functions::GetPC();
		uintptr_t CESO_Addr = Memory::GetAddressFromOffset(0x2090420);
		/*MH_CreateHook((void*)CESO_Addr, Hooks::CESO_Hk, (void**)&Hooks::CurlEasySetOpt);
		MH_EnableHook((void*)CESO_Addr);*/
		Functions::SetupConsole();
		MessageBoxA(0, "Press OK to Load In-Game!", "Alpharium", MB_OK);
		Unreal::FString Map = L"Zone_Onboarding_FarmsteadFort?game=FortniteGame.FortGameMode";
		Globals::GPC->ProcessEvent(FindObject("/Script/Engine.PlayerController:SwitchLevel"), &Map);
		MH_CreateHook((void*)PE_Addr, Hooks::PEHook, (LPVOID*)&Unreal::ProcessEventOG);
		MH_EnableHook((void*)PE_Addr);
	}
}