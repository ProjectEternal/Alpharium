#pragma once
#include "pch.h"
#include "Unreal.h"

namespace Globals {
	Unreal::UObject* GEngine; //Global FortEngine

	Unreal::UObject* GameMode; //GameMode
	Unreal::UObject* GameState; //GameState
}

namespace Functions {
	//Gets Player Controller
	Unreal::UObject* GetLocalPC() {
		auto GI = *Finder::Find(Globals::GEngine, "GameInstance");
		auto Player = Finder::Find<Unreal::TArray<Unreal::UObject*>*>(GI, "LocalPlayers")->Data[0];
		return *Finder::Find(Player, "PlayerController");
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
	
	Unreal::UObject* (__fastcall* SpawnActor_Vectored)(Unreal::UObject* World, void* Idk, Unreal::UObject* Class, Unreal::FVector* Loc, Unreal::FVector* Rot, const FActorSpawnParameters& SpawnParams);

	//TODO
	Unreal::UObject* SpawnActor(Unreal::UObject* Class, Unreal::FVector Loc = Unreal::FVector(1, 1, 10000), Unreal::FVector Rot = Unreal::FVector(1,1,1)) {
		return SpawnActor_Vectored(GetWorld(), GetWorld(), Class, &Loc, &Rot, FActorSpawnParameters());
	}

	//Grants CheatManager
	void SetupCM(Unreal::UObject* PC = GetLocalPC()) {
		struct {
			Unreal::UObject* TargetClass;
			Unreal::UObject* Outer;
			Unreal::UObject* Ret;
		} params;

		params.TargetClass = FindObject("/Script/FortniteGame.FortCheatManager"); //Engine CheatManager doesn't have CheatScript
		params.Outer = PC;

		FindObject("/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject("/Script/Engine.GameplayStatics:SpawnObject"), &params);

		*Finder::Find(PC, "CheatManager") = params.Ret;
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


		void GiveAllItems(Unreal::UObject* PC) {
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

						PC->ProcessEvent(Func, &Parms);
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
			//GiveAllItems();
			//DestroyTT();
			//SetupHero();
			//*Finder::Find<bool*>(Globals::GPC, "bTutorialCompleted") = true;
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

		void Update(Unreal::UObject* PC) {
			Unreal::UObject* Inv = *Finder::Find(PC, "WorldInventory");
			Unreal::UObject* QB = *Finder::Find(PC, "QuickBars");

			Inv->ProcessEvent(FindObject("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate"));

			PC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:HandleWorldInventoryLocalUpdate"));

			PC->ProcessEvent(FindObject("/Script/FortniteGame.FortPlayerController:OnRep_QuickBar"));

			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:OnRep_PrimaryQuickBar"));

			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:OnRep_SecondaryQuickBar"));

			//MP Shit
			//Finder::Find<FFortItemList*>(Inv, "Inventory")->MarkArrayDirty();
		}

		void RemoveItem(Unreal::UObject* PC, Unreal::FGuid GUID) {
			Unreal::UObject* Inv = *Finder::Find(PC, "WorldInventory");
			FFortItemList* ItemList = Finder::Find<FFortItemList*>(Inv, "Inventory");
			for (int i = 0; i < ItemList->ReplicatedEntries.Num(); i++) {
				FFortItemEntry Entry = ItemList->ReplicatedEntries.At(i);
				if (Entry.ItemGuid == GUID) {
					ItemList->ReplicatedEntries.Remove(i);
				}
			}

			for (int i = 0; i < ItemList->ItemInstances.Num(); i++) {
				FFortItemEntry Entry = *Finder::Find<FFortItemEntry*>(ItemList->ItemInstances.At(i), "ItemEntry");
				if (Entry.ItemGuid == GUID) {
					ItemList->ItemInstances.Remove(i);
				}
			}
		}

		void AddItem(Unreal::UObject* PC, Unreal::UObject* ItemDef, int Slot = 0, int Count = 1, uint8_t Quickbar = 0) {
			if (!ItemDef) return;
			Unreal::UObject* Inv = *Finder::Find(PC, "WorldInventory");
			FFortItemList* ItemList = Finder::Find<FFortItemList*>(Inv, "Inventory");
			//Stacking
			std::string ItemClassName = ItemDef->Class->GetName();
			if (ItemClassName == "/Script/FortniteGame.FortResourceItemDefinition" || ItemClassName == "/Script/FortniteGame.FortAmmoItemDefinition" || ItemClassName == "/Script/FortniteGame.FortConsumableItemDefinition" || ItemClassName == "/Script/FortniteGame.FortIngredientItemDefinition") {
				for (int i = 0; i < ItemList->ReplicatedEntries.Num(); i++) {
					FFortItemEntry Entry = ItemList->ReplicatedEntries.At(i);
					if (Entry.ItemDefinition == ItemDef) {
						int NewCount = Entry.Count + Count;
						RemoveItem(PC, Entry.ItemGuid);
						AddItem(PC, ItemDef, Slot, NewCount, Quickbar);
						return;
					}
				}
			}
			Unreal::UObject* Item;
			ItemDef->ProcessEvent(FindObject("/Script/FortniteGame.FortItemDefinition:CreateTemporaryItemInstanceBP"), &Item);
			ItemList->ItemInstances.Add(Item);
			ItemList->ReplicatedEntries.Add(*Finder::Find<FFortItemEntry*>(Item, "ItemEntry"));
			Unreal::FGuid ItemGUID;
			Item->ProcessEvent(FindObject("/Script/FortniteGame.FortItem:GetItemGuid"), &ItemGUID);
			struct {
				Unreal::FGuid ItemGuid;
				uint8_t Quickbar;
				int Slot;
			}p1{ ItemGUID,Quickbar,Slot};
			Unreal::UObject* QB = *Finder::Find(PC, "QuickBars");
			QB->ProcessEvent(FindObject("/Script/FortniteGame.FortQuickBars:ServerAddItemInternal"), &p1);
		}

		void AddBaseItems(Unreal::UObject* PC) {
			AddItem(PC, FindObject("/Game/Items/Weapons/BuildingTools/EditTool.EditTool"));

			AddItem(PC, FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall"), 0, 1, 1);
			AddItem(PC, FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor"), 1, 1, 1);
			AddItem(PC, FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W"), 2, 1, 1);
			AddItem(PC, FindObject("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS"), 3, 1, 1);

			AddItem(PC, FindObject("/Game/Weapons/WeaponItemData/Melee/Melee_Impact_Pickaxe_T02.Melee_Impact_Pickaxe_T02"), 0);

			Update(PC);
		}

		Unreal::FGuid GetItemGUID(Unreal::UObject* PC, bool isSecondary, int Slot) {
			Unreal::UObject* QB = *Finder::Find(PC, "QuickBars");
			FQuickBar* FQB = ((isSecondary) ? (Finder::Find<FQuickBar*>(QB, "SecondaryQuickBar")) : ((Finder::Find<FQuickBar*>(QB, "PrimaryQuickBar"))));

			return FQB->Slots.At(Slot).Items.At(0);
		}

		Unreal::UObject* GetItemFromGUID(Unreal::UObject* PC, Unreal::FGuid GUID) {
			Unreal::UObject* Inv = *Finder::Find(PC, "WorldInventory");
			FFortItemList* ItemList = Finder::Find<FFortItemList*>(Inv, "Inventory");

			for (int i = 0; i < ItemList->ReplicatedEntries.Num(); i++) {
				FFortItemEntry Entry = ItemList->ReplicatedEntries.At(i);
				if (Entry.ItemGuid == GUID) return Entry.ItemDefinition;
			}
		}

		Unreal::UObject* (__thiscall* EquipWeaponData)(Unreal::UObject*, Unreal::UObject*, Unreal::FGuid);

		void Equip(Unreal::UObject* Pawn, Unreal::UObject* ItemDef, Unreal::FGuid ItemGUID = Unreal::FGuid()) {
			Unreal::UObject* FortWeapon = EquipWeaponData(Pawn, ItemDef, ItemGUID);

			if (FortWeapon && FortWeapon->Class && FortWeapon->Class->GetName() == "/Game/Weapons/FORT_BuildingTools/Blueprints/DefaultBuildingTool.DefaultBuildingTool_C") {
				*Finder::Find(FortWeapon, "DefaultMetadata") = *Finder::Find(ItemDef, "BuildingMetaData");
				FortWeapon->ProcessEvent(FindObject("/Script/FortniteGame.FortWeap_BuildingTool:OnRep_DefaultMetadata"));
			}
		}

		void Setup(Unreal::UObject* PC) {
			*Finder::Find<int*>(PC, "OverriddenBackpackSize") = 999;
			Unreal::UObject* Inv = SpawnActor(FindObject("/Script/FortniteGame.FortInventory"));
			FFortItemList ItemList = FFortItemList();
			ItemList.Init();
			*Finder::Find<FFortItemList*>(Inv, "Inventory") = ItemList;
			Inv->ProcessEvent(FindObject("/Script/Engine.Actor:SetOwner"), &PC);
			*Finder::Find(PC, "WorldInventory") = Inv;
			Unreal::UObject* QB = SpawnActor(FindObject("/Script/FortniteGame.FortQuickBars"));
			QB->ProcessEvent(FindObject("/Script/Engine.Actor:SetOwner"), &PC);
			*Finder::Find(PC, "QuickBars") = QB;
			Update(PC);

			*Finder::Find<bool*>(PC, "bHasInitializedWorldInventory") = true;
		}
	}
}