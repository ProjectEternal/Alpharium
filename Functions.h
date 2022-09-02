#pragma once
#include "pch.h"
#include "Unreal.h"

namespace Globals {
	Unreal::UObject* GEngine; //Global FortEngine
	Unreal::UObject* GPC; //Global PlayerController
	Unreal::UObject* GPawn = nullptr; //Global PlayerPawn
}

namespace Functions {
	//Gets Player Controller
	void GetPC() {
		auto GI = *Finder::Find(Globals::GEngine, "GameInstance");
		auto Player = Finder::Find<Unreal::TArray<Unreal::UObject*>*>(GI,"LocalPlayers")->Data[0];
		Globals::GPC = *Finder::Find(Player, "PlayerController");
	}

	Unreal::UObject* GetWorld() {
		auto GV = *Finder::Find(Globals::GEngine, "GameViewport");
		auto Ret = *Finder::Find(GV, "World");
		return Ret;
	}

	//Set an actors visibility
	void ToggleActorVisibility(Unreal::UObject* Object, bool Show) {
		Object->ProcessEvent(FindObject("/Script/Engine.Actor:SetActorHiddenInGame"), &Show);
	}

	void SpawnActor(std::string Class) {
		struct {
			Unreal::UObject* World;
			Unreal::FString Cmd;
			Unreal::UObject* Plr;
		} params;
		params.World = GetWorld();
		params.Plr = Globals::GPC;
		params.Cmd = std::wstring(L"summon " + std::wstring(Class.begin(), Class.end())).c_str();
		FindObject("/Script/Engine.Default__KismetSystemLibrary")->ProcessEvent(FindObject("/Script/Engine.KismetSystemLibrary:ExecuteConsoleCommand"), &params);
	}

	//Enables UE Console
	void SetupConsole() {
		struct {
			Unreal::UObject* TargetClass;
			Unreal::UObject* Outer;
			Unreal::UObject* Ret;
		} params;

		params.TargetClass = *Finder::Find(Globals::GEngine, "ConsoleClass");
		params.Outer = *Finder::Find(Globals::GEngine, "GameViewport");

		FindObject("/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:SpawnObject"), &params);
		*Finder::Find(params.Outer, "ViewportConsole") = params.Ret;
	}

	//Enables CheatManager
	Unreal::UObject* SetupCM() {
		struct {
			Unreal::UObject* TargetClass;
			Unreal::UObject* Outer;
			Unreal::UObject* Ret;
		} params;

		params.TargetClass = FindObject("/Script/Engine.CheatManager");
		params.Outer = Globals::GPC;

		FindObject("/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:SpawnObject"), &params);

		*Finder::Find(Globals::GPC,"CheatManager") = params.Ret;
		return params.Ret;
	}

	//Self Explanatory
	void ToggleHUD(bool Show) {
		Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:SetShowHUD"), &Show);
	}
}