#pragma once
#include "pch.h"
#include "Unreal.h"

namespace Globals {
	Unreal::UObject* GEngine; //Global FortEngine
	Unreal::UObject* GPC; //Global PlayerController
	Unreal::UObject* GPawn; //Global PlayerPawn

	Unreal::UObject* GameMode; //GameMode
	Unreal::UObject* GameState; //GameState
}

namespace Functions {
	//Gets Player Controller
	void GetPC() {
		auto GI = *Finder::Find(Globals::GEngine, "GameInstance");
		auto Player = Finder::Find<Unreal::TArray<Unreal::UObject*>*>(GI, "LocalPlayers")->Data[0];
		Globals::GPC = *Finder::Find(Player, "PlayerController");
	}

	Unreal::UObject* GetWorld() {
		auto GV = *Finder::Find(Globals::GEngine, "GameViewport");
		auto Ret = *Finder::Find(GV, "World");
		return Ret;
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
	
	Unreal::UObject* SpawnActor(Unreal::UObject* Class, Unreal::UObject* Owner = nullptr, Unreal::FVector Loc = Unreal::FVector(1,1,1)) {
		auto Trans = GTrans;
		Trans->Translation = Loc;
		GParms->Owner = Owner;
		return SpawnActor_OG(GetWorld(), Class, Class, Trans, GParms);
		/*Unreal::UObject* GPS = FindObject("/Script/Engine.Default__GameplayStatics");
		struct {
			Unreal::UObject* World;
			Unreal::UObject* Class;
			Unreal::FTransform Trans;
			bool bFail;
			Unreal::UObject* Owner;
			Unreal::UObject* Ret;
		}p1{ GetWorld(), Class, Trans,true, Owner, nullptr };

		GPS->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:BeginSpawningActorFromClass"), &p1);



		if (p1.Ret) {
			struct {
				Unreal::UObject* In;
				Unreal::FTransform Trans;
				Unreal::UObject* Ret;
			}p2{ p1.Ret,Trans,nullptr };

			GPS->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:FinishSpawningActor"), &p2);

			if (!p2.Ret) p2.Ret = p1.Ret;

			return p2.Ret;
		}
		else {
			MessageBoxA(0, "SA Failed!", MB_OK,0);
			return nullptr;
		}*/
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

		*Finder::Find(Globals::GPC, "CheatManager") = params.Ret;
		return params.Ret;
	}

	namespace Kismet {

		Unreal::FName String2Name(Unreal::FString Str) {
			struct {
				Unreal::FString InStr;
				Unreal::FName Ret;
			} params{ Str };
			FindObject("/Script/Engine.Default__KismetStringLibrary")->ProcessEvent(FindObject("/Script/Engine.KismetStringLibrary:Conv_StringToName"), &params);
		}
	}

	namespace Inventory {
		struct FFortItemEntry : public FFastArraySerializerItem {
			int Count;
			Unreal::UObject* ItemDefinition;
			float Durability;
			int Level;
			int LoadedAmmo;
			uint8_t UKD_00[0x4];
			Unreal::TArray<Unreal::UObject*> AlterationDefinitions;
			Unreal::FString ItemSource;
			Unreal::FGuid ItemGuid;
			bool bInOverflow;
			bool bInStorageVault;
			bool bIsReplicatedCopy;
			bool bIsDirty;
		};

		struct FFortItemList : public FFastArraySerializer {
			Unreal::TArray<FFortItemEntry> ReplicatedEntries;
			uint8_t UKD_00;
			Unreal::TArray<Unreal::UObject*> ItemInstances;
			uint8_t UKD_01;

			void Init() {
				ReplicatedEntries = Unreal::TArray<FFortItemEntry>();
				ItemInstances = Unreal::TArray<Unreal::UObject*>();
			}
		};

		void Update() {
			Unreal::UObject* Inv = *Finder::Find(Globals::GPC, "WorldInventory");
			Unreal::UObject* QB = *Finder::Find(Globals::GPC, "QuickBars");

			Inv->ProcessEvent(FindObject("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate"));
			Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:HandleWorldInventoryLocalUpdate"));

			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:OnRep_PrimaryQuickBar"));
			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:OnRep_SecondaryQuickBar"));

			Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_QuickBar"));

			Finder::Find<FFortItemList*>(Inv, "Inventory")->MarkArrayDirty();
		}

		void AddItem(Unreal::UObject* ItemDef, int Slot = 0, int Count = 1, uint8_t Quickbar = 0) {
			Unreal::UObject* Inv = *Finder::Find(Globals::GPC, "WorldInventory");
			Unreal::UObject* Item;
			ItemDef->ProcessEvent(FindObject("/Script/FortniteGame.FortItemDefinition:CreateTemporaryItemInstanceBP"), &Item);

			Finder::Find<FFortItemEntry*>(Item, "ItemEntry")->Count = 1;
			FFortItemList* ItemList = Finder::Find<FFortItemList*>(Inv, "Inventory");
			ItemList->ReplicatedEntries.Add(*Finder::Find<FFortItemEntry*>(Item, "ItemEntry"));
			ItemList->ItemInstances.Add(Item);
			struct {
				Unreal::FGuid ItemGuid;
				uint8_t Quickbar;
				int Slot;
			}p1{ Finder::Find<FFortItemEntry*>(Item, "ItemEntry")->ItemGuid,Quickbar,Slot};
			(*Finder::Find(Globals::GPC, "QuickBars"))->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:ServerAddItemInternal"), &p1);

			Update();
		}

		void Setup() {
			GetPC();
			Unreal::UObject* Inv = FindObject("FortInventory_1", false);//SpawnActor(FindObject("/Script/FortniteGame.FortInventory"), Globals::GPC);
			//Unreal::UObject* Inv = *Finder::Find(Globals::GPC, "WorldInventory");
			MessageBoxA(0, Inv->GetName().c_str(), "KMS", MB_OK);
			/**Finder::Find<uint8_t*>(Inv, "InventoryType") = 0;
			FFortItemList FortInv = FFortItemList();
			FortInv.Init();
			*Finder::Find<FFortItemList*>(Inv, "Inventory") = FortInv;
			MessageBoxA(0, "SETUP WORLDINVENTORY", "KMS", MB_OK);*/
			*Finder::Find(Globals::GPC, "WorldInventory") = Inv;
			MessageBoxA(0, "Setup WI", "KMS", MB_OK);
			Unreal::UObject* QB = SpawnActor(FindObject("/Script/FortniteGame.FortQuickBars"), Globals::GPC);
			MessageBoxA(0, QB->GetName().c_str(), "KMS", MB_OK);
			if (!QB) QB = FindObject("FortQuickBars_0", false);
			*Finder::Find(Globals::GPC, "QuickBars") = QB;
			MessageBoxA(0, "ADDING ITEMS", "KMS", MB_OK);
			AddItem(FindObject("/Game/Weapons/WeaponItemData/Assault/Assault_Auto_T09.Assault_Auto_T09"), 1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/EditTool.EditTool"));

			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall"),0,1,1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor"),1,1,1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W"),2,1,1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS"),3,1,1);
		}
	}
}