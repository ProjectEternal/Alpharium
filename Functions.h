#pragma once
#include "pch.h"
#include "Unreal.h"

namespace Globals {
	Unreal::UObject* GEngine; //Global FortEngine
	Unreal::UObject* GPC; //Global PlayerController
	Unreal::UObject* GPawn = nullptr; //Global PlayerPawn

	Unreal::UObject* GameMode; //GameMode
	Unreal::UObject* GameState; //GameState
}

struct FFortGameplayAttributeData {
	unsigned char UnknownData00[0x8];
	float BaseValue;
	float CurrentValue;
	float Minimum;
	float Maximum;
	bool bIsCurrentClamped;
	bool bIsBaseClamped;
	bool bShouldClampBase;
	unsigned char UnknownData01[0x1];
	float UnclampedBaseValue;
	float UnclampedCurrentValue;
	unsigned char UnknownData02[0x4];
};

namespace Functions {
	void SetAttribute(FFortGameplayAttributeData& Attribute, int CurrentVal, int MaxVal)
	{
		Attribute.BaseValue = 0;
		Attribute.Maximum = 0;
		Attribute.CurrentValue = CurrentVal;
		Attribute.Maximum = MaxVal;
	}

	//Gets Player Controller
	void GetPC() {
		auto GI = *Finder::Find(Globals::GEngine, "GameInstance");
		auto Player = Finder::Find<Unreal::TArray<Unreal::UObject*>*>(GI,"LocalPlayers")->Data[0];
		Globals::GPC = *Finder::Find(Player, "PlayerController");
	}

	Unreal::FName String2Name(Unreal::FString Str) {
		struct {
			Unreal::FString InStr;
			Unreal::FName Ret;
		} params{Str};
		FindObject("/Script/Engine.Default__KismetStringLibrary")->ProcessEvent(FindObject("/Script/Engine.KismetStringLibrary:Conv_StringToName"), &params);
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
	
	Unreal::UObject* SpawnObject(Unreal::UObject* Class, Unreal::UObject* Outer) {
		struct {
			Unreal::UObject* TargetClass;
			Unreal::UObject* Outer;
			Unreal::UObject* Ret;
		} params;

		params.TargetClass = Class;
		params.Outer = Outer;

		FindObject("/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:SpawnObject"), &params);

		return params.Ret;
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

		params.TargetClass = FindObject("/Script/FortniteGame.FortCheatManager");
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