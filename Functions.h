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
	
	//Return Value is not always accurate
	Unreal::UObject* SpawnActor(std::wstring Class) {
		GetPC();
		struct {
			Unreal::FString Cmd;
		} params;
		params.Cmd = Class.c_str();//std::wstring(Class.begin(), Class.end()).c_str();
		(*Finder::Find(Globals::GPC, "CheatManager"))->ProcessEvent(FindObject("/Script/Engine.CheatManager:Summon"), &params);

		//Not Accurate
		return FindObject(std::string(Class.begin(), Class.end()) + "_", false);
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

			return params.Ret;
		}
	}

	namespace Frontend {
		struct FFortHomeBaseInfo {
			int Rating;
			int PartyRating;
			unsigned char Banner[0xE8];
			unsigned char Color[0x10];
			unsigned char Name[0x18];
			Unreal::TArray<int> Resources;
			bool ValidData;
			unsigned char UKD_00[0x3];
		};

		struct FortTeamMemberInfo {
			unsigned char MemberUniqueId[0x8];
			unsigned char PartyLeaderUniqueId[0x8];
			unsigned char PlayerName[0x18];
			bool bPartyLeader;
			bool bIsInZone;
			unsigned char UKD_00[0x2];
			int NumPlayersInParty;
			int PlayerIndex;
			uint8_t TeamAffiliation;
			unsigned char UKD_01[0x3];
			unsigned char HeroClass[0x18];
			unsigned char HeroLevel[0x18];
			int HeroXP;
			Unreal::UObject* HeroItem;
			Unreal::TArray<Unreal::UObject*> SelectedGadgetItems;
			unsigned char SlateBrush[0x74];
			unsigned char HomeBaseInfo[0x11C];
			//FFortHomeBaseInfo HomeBaseInfo;
		};


		void GiveAllItems() {
			GetPC();
			Unreal::UObject* Func = FindObject("/Script/FortniteGame.FortPlayerController:ClientGivePlayerLocalAccountItem");

			for (int i = 0; i < GObjs->ObjObject.Num; i++) {
				Unreal::UObject* Object = GObjs->ObjObject.Objects[i].Object;
				if (!Object->IsValid()) continue;

				std::string ClassName = Object->Class->GetName();

				if (ClassName == "/Script/FortniteGame.FortHeroType" || ClassName == "/Script/FortniteGame.FortWorkerType" || ClassName == "/Script/FortniteGame.FortPersistentResourceItemDefinition" || ClassName == "/Script/FortniteGame.FortSchematicItemDefinition") {
					if (!Object->GetName().contains("Default__")) {
						struct
						{
							Unreal::UObject* ItemDefinition;
							int Count;
						} Parms{ Object, 1 };

						Globals::GPC->ProcessEvent(Func, &Parms);
					}
				}
			}
		}

		void SetupHero() {
			Unreal::UObject* FortHero = FindObject("/Engine/Transient.FortHero_0");
			if (FortHero->IsValid()) {
				*Finder::Find<int*>(FortHero, "Level") = 25;
				*Finder::Find<int*>(FortHero, "XP") = 300;
				*Finder::Find<Unreal::FString*>(FortHero, "Hero_Name") = L"Rescue Trooper Ramirez";
				*Finder::Find<int*>(FortHero, "Gender") = 2;

				//FindObject("/Engine/Transient.FortEngine_0:FortLocalPlayer_0.FortFrontEndContext_0")->ProcessEvent(FindObject("/Script/FortniteUI.FortFrontEndContext:SetPersonalHeroChoice"), &FortHero);
			}
		}

		//This would fix Hero Level and Homebase shit
		void FixTeamInfo() {
			FortTeamMemberInfo TeamInfo;
			FindObject("/Engine/Transient.FortEngine_0:FortLocalPlayer_0.FortPartyContext_0")->ProcessEvent(FindObject("/Script/FortniteGame.FortPartyContext:GetLocalPlayerTeamMemberInfo"), &TeamInfo);

			//TeamInfo (Too Lazy)
		}

		void DestroyTT() {
			Unreal::UObject* Func = FindObject("/Script/Engine.ActorComponent:K2_DestroyComponent");
			for (int i = 0; i < GObjs->ObjObject.Num; i++) {
				Unreal::UObject* Object = GObjs->ObjObject.Objects[i].Object;
				if (!Object->IsValid()) continue;

				std::string ObjName = Object->GetName();

				if (ObjName.contains("HB_") && Object->Class->GetName() == "/Script/UMG.WidgetComponent") {
					Object->ProcessEvent(Func, &Object);
				}
			}
		}

		void Setup() {
			GiveAllItems();
			DestroyTT();
			SetupHero();
			*Finder::Find<bool*>(Globals::GPC, "bTutorialCompleted") = true;
		}
	}

	namespace Inventory {
		struct FFortItemEntryStateValue {
			int IntValue;
			unsigned char NameValue[0x8];
			uint8_t StateType;
			unsigned char UKD_00[0x3];
		};

		struct FFortGiftingInfo {
			Unreal::FString PlayerName;
			Unreal::UObject* HeroType;
			unsigned char UKD_00[0x8];
		};

		struct FFortItemEntry : public FFastArraySerializerItem {
			int Count;
			Unreal::UObject* ItemDefinition;
			float Durability;
			int Level;
			int LoadedAmmo;
			Unreal::TArray<Unreal::UObject*> AlterationDefinitions;
			Unreal::FString ItemSource;
			Unreal::FGuid ItemGuid;
			bool bInOverflow;
			bool bInStorageVault;
			bool bIsReplicatedCopy;
			bool bIsDirty;
			FFortGiftingInfo GiftingInfo;
			Unreal::TArray<FFortItemEntryStateValue> StateValues;
			Unreal::TWeakObjectPtr<Unreal::UObject> ParentInventory;
			int GameplayAbilitySpecHandle; //FGameplayAbilitySpecHandle
		};

		struct FFortItemList : public FFastArraySerializer {
			Unreal::TArray<FFortItemEntry> ReplicatedEntries;
			unsigned char UKD_00[0x3C];
			Unreal::TArray<Unreal::UObject*> ItemInstances;
			unsigned char UKD_01[0x3C];

			void Init() {
				this->ReplicatedEntries = Unreal::TArray<FFortItemEntry>(6);
				this->ItemInstances = Unreal::TArray<Unreal::UObject*>(6);
			}
		};
		
		struct FQuickBarSlotData {
			unsigned char Size[0x28];
		};

		struct FQuickBarSlot {
			Unreal::TArray<Unreal::FGuid> Items;
		};
		
		struct FQuickBarData {
			Unreal::TArray<FQuickBarSlotData> QuickbarSlots;
		};

		struct FQuickBar {
			int CurrentSlot;
			int PrevSlot;
			Unreal::TArray<FQuickBarSlot> Slots;
			FQuickBarData DataDefinition;
		};

		void Update() {
			GetPC();
			Unreal::UObject* Inv = *Finder::Find(Globals::GPC, "WorldInventory");
			Unreal::UObject* QB = *Finder::Find(Globals::GPC, "QuickBars");

			Inv->ProcessEvent(FindObject("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate"));

			Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:HandleWorldInventoryLocalUpdate"));

			Globals::GPC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_QuickBar"));

			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:OnRep_PrimaryQuickBar"));

			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:OnRep_SecondaryQuickBar"));

			//MP Shit
			//Finder::Find<FFortItemList*>(Inv, "Inventory")->MarkArrayDirty();
		}

		void AddItem(Unreal::UObject* ItemDef, int Slot = 0, int Count = 1, uint8_t Quickbar = 0) {
			GetPC();
			if (!ItemDef) return;
			Unreal::UObject* Inv = *Finder::Find(Globals::GPC, "WorldInventory");
			Unreal::UObject* Item;
			ItemDef->ProcessEvent(FindObject("/Script/FortniteGame.FortItemDefinition:CreateTemporaryItemInstanceBP"), &Item);

			FFortItemList* ItemList = Finder::Find<FFortItemList*>(Inv, "Inventory");
			ItemList->ItemInstances.Add(Item);
			ItemList->ReplicatedEntries.Add(*Finder::Find<FFortItemEntry*>(Item, "ItemEntry"));
			Unreal::FGuid ItemGUID;
			Item->ProcessEvent(FindObject("/Script/FortniteGame.FortItem:GetItemGuid"), &ItemGUID);
			struct {
				Unreal::FGuid ItemGuid;
				uint8_t Quickbar;
				int Slot;
			}p1{ ItemGUID,Quickbar,Slot};
			Unreal::UObject* QB = *Finder::Find(Globals::GPC, "QuickBars");
			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:ServerAddItemInternal"), &p1);
		}

		void AddBaseItems() {
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/EditTool.EditTool"));

			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall"), 0, 1, 1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor"), 1, 1, 1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W"), 2, 1, 1);
			AddItem(FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS"), 3, 1, 1);

			AddItem(FindObject("/Game/Weapons/WeaponItemData/Melee/Melee_Impact_Pickaxe_T02.Melee_Impact_Pickaxe_T02"), 0);

			Update();
		}

		Unreal::FGuid GetItemGUID(bool isSecondary, int Slot) {
			Unreal::UObject* QB = *Finder::Find(Globals::GPC, "QuickBars");
			FQuickBar* FQB = ((isSecondary) ? (Finder::Find<FQuickBar*>(QB, "SecondaryQuickBar")) : ((Finder::Find<FQuickBar*>(QB, "PrimaryQuickBar"))));

			return FQB->Slots.At(Slot).Items.At(0);
		}

		Unreal::UObject* GetItemFromGUID(Unreal::FGuid GUID) {
			Unreal::UObject* Inv = *Finder::Find(Globals::GPC, "WorldInventory");
			FFortItemList* ItemList = Finder::Find<FFortItemList*>(Inv, "Inventory");

			for (int i = 0; i < ItemList->ReplicatedEntries.Num(); i++) {
				FFortItemEntry Entry = ItemList->ReplicatedEntries.At(i);
				if (Entry.ItemGuid == GUID) return Entry.ItemDefinition;
			}
		}

		Unreal::UObject* (__thiscall* EquipWeaponData)(Unreal::UObject*, Unreal::UObject*, Unreal::FGuid);

		void Equip(Unreal::UObject* ItemDef, Unreal::FGuid ItemGUID = Unreal::FGuid()) {
			//AFortWeapon* AFortPawn::EquipWeaponData(UFortItemDefinition*, FGuid) 0x40A8D0
			uint8_t* Role = Finder::Find<uint8_t*>(Globals::GPawn, "Role");
			uint8_t OldRole = *Role;
			*Role = 3;
			Unreal::UObject* FortWeapon = EquipWeaponData(Globals::GPawn, ItemDef, ItemGUID);//reinterpret_cast<Unreal::UObject*(*)(Unreal::UObject*, Unreal::UObject*, Unreal::FGuid)>(Memory::GetAddressFromOffset(0x40A8D0))(Globals::GPawn, ItemDef, ItemGUID);
			*Role = OldRole;
		}

		void Setup() {
			GetPC();
			*Finder::Find<int*>(Globals::GPC, "OverriddenBackpackSize") = 999;
			Unreal::UObject* Inv = FindObject("FortInventory_1", false);
			FFortItemList ItemList = FFortItemList();
			ItemList.Init();
			*Finder::Find<FFortItemList*>(Inv, "Inventory") = ItemList;
			Inv->ProcessEvent(FindObject("/Script/Engine.Actor:SetOwner"), &Globals::GPC);
			*Finder::Find(Globals::GPC, "WorldInventory") = Inv;
			SpawnActor(L"FortQuickBars");
			Unreal::UObject* QB = FindObject("FortQuickBars_0", false);
			QB->ProcessEvent(FindObject("/Script/Engine.Actor:SetOwner"), &Globals::GPC);
			*Finder::Find(Globals::GPC, "QuickBars") = QB;
			Update();

			*Finder::Find<bool*>(Globals::GPC, "bHasInitializedWorldInventory") = true;
		}
	}
}