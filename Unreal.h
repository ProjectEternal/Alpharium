#pragma once
#include "pch.h"
namespace Unreal {
	void(__cdecl* Free)(void* buf);
	void* (__fastcall* ProcessEventOG)(struct UObject*, struct UObject*, struct UObject*, void*); //UObject::ProcessEvent
	struct FString(*GetPathName)(struct UObject*); //UKismetSystemLibrary::GetPathName

	template <class T> struct TArray {
		TArray() { Data = nullptr; Max = Count = 0; }; //Default TArray Constructor (creates an empty TArray)

		//Returns number of elements in the TArray
		int Num() {
			return Count;
		}

		//Check if the TArray is Valid (90% sure this is not needed but whatever)
		bool IsValid() {
			return (Count <= Max && Count != 0);
		}

		//Check if the passed Index is not over the Max and make sure that the TArray is Valid
		bool IsValid(int Index) {
			return (IsValid() && Index <= Max);
		}

		//Add a new Value into the TArray
		void Add(T NewValue) {
			Data = (T*)realloc(Data, (sizeof(T) * (Count++)));
			Data[Count] = NewValue;
			Max = Count;
		}

		T* Data; //The Data Entrys stored in the array
		int Count; //The Current number of Elements
		int Max; //The Max number of Elements
	};

	//Pretty much just a string in the form of a TArray. idk.
	struct FString : public TArray<wchar_t> {
		FString() { Data = nullptr; Max = Count = 0; }; //Default FString Constructor (creates an empty FString)

		//Creates a new FString with the value being the passed String
		FString(const wchar_t* String) { Data = const_cast<wchar_t*>(String); Count = Max = (std::wcslen(String) + 1); };

		bool IsValid() {
			return Data != nullptr && Data != L"";
		}

		std::wstring ToWString() {
			return Data;
		}

		std::string ToString() {
			auto length = std::wcslen(Data);

			std::string str(length, '\0');

			std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

			Free(Data);

			return str;
		}
	};

	struct FName {
		int NameIndex; //FNames use an Index so this can be used to change one FName to another. Example: BeaconNetDriver(284) -> GameNetDriver(282)
		int Num; //Idk what this is used for.

		FName() { NameIndex = Num = 0; };

		FName(int Index) { NameIndex = Index; Num = 0; };
	};

	struct UObject {
		void** VTable;
		int ObjectFlags; //idk much about these
		int Index; //Im assuming the Objects Index in the Objects Array. (Not 100% sure)
		UObject* Class; //The Objects Class. Example: BlueprintGeneratedClass, Texture2D, etc
		FName Name; //The Objects Name. (Find a better explanation lol)
		UObject* Outer; //Idk how to describe this.

		//Basic way of making sure the object is Valid.
		bool IsValid() {
			return this != nullptr;
		}

		std::string GetName() {
			return IsValid() ? GetPathName(this).ToString() : "INVALID";
		}

		void* ProcessEvent(UObject* Func, void* Params = nullptr) {
			if (IsValid() && Func->IsValid()) {
				return ProcessEventOG(this, this, Func, Params);
			}
			else {
				std::string NameS = "Function/Object Name Unknown";
				if (IsValid()) {
					NameS = std::string("with Object Name: " + GetName());
				}
				else if (Func->IsValid()) {
					NameS = std::string("with Function Name: " + Func->GetName());
				}
				LOG("ProcessEvent Failed: Object or Function was Invalid\n" + NameS);
			}
		}
	};

	struct FUObjectItem {
		UObject* Object; //The UObject thats associated with this.
		int Index; //No Clue what this does
		int SerialNumber; //Also No Clue of its purpose
	};

	struct TUObjectArray {
		FUObjectItem* Objects; //Array of UObjectItems
		int Max; //Max Number of Items.
		int Num; //Current Number of Objects.
	};

	struct FUObjectArray {
		TUObjectArray ObjObject;
	};

	struct FQuat
	{
		float X;
		float Y;
		float Z;
		float W;

		FQuat() {
			X = Y = Z = W = 0.0f;
		}
	};

	struct FVector
	{
		FVector() {
			X = Y = Z = 0.0f;
		}

		FVector(float NX, float NY, float NZ) {
			X = NX;
			Y = NY;
			Z = NZ;
		}

		float X;
		float Y;
		float Z;
	};

	struct FTransform {
		FTransform() {
			Rotation = {};
			Scale3D = { 1,1,1 };
			Translation = {};
		}

		FTransform(FVector Loc) {
			Rotation = {};
			Scale3D = { 1,1,1 };
			Translation = Loc;
		}
		FQuat Rotation;
		FVector Translation;
		unsigned char UnknownData00[0x4];
		FVector Scale3D;
		unsigned char UnknownData01[0x4];
	};

	UObject* (*SLO)(UObject*, UObject*, const TCHAR*, const TCHAR*, uint32_t, UObject*, bool);
}
Unreal::FUObjectArray* GObjs; //Global UObject Array

Unreal::UObject* FindObject(std::string TargetName, bool Exact = true, bool bLog = true) {
	for (int i = 0; i < GObjs->ObjObject.Num; i++) {
		Unreal::UObject* Object = GObjs->ObjObject.Objects[i].Object;

		if (Object->IsValid() && (Exact ? (Object->GetName() == TargetName) : (Object->GetName().contains(TargetName)))) {
			return Object;
		}
	}
	if (bLog) {
		LOG("Object with Name '" + TargetName + "' Was Not Found!");
	}
	return nullptr;
}

namespace Unreal {
	struct UField : UObject
	{
		UField* Next;
	};

	struct UStruct : UField
	{
		UStruct* Super;
		UField* Children;
		int Size;
		int MinAlignment;
		TArray<uint8_t> Script;
		void* PropLink;
		void* RefLink;
		void* DestLink;
		void* PostConstLink;
		TArray<UObject*> ScriptObjRefs;
	};
}

namespace Finder {
	using namespace Unreal;
	UObject* FindChild(Unreal::UObject* InObject, std::string PropName) {
		UObject* Prop = FindObject(InObject->GetName() + ":" + PropName, true, false);
		if (Prop->IsValid()) {
			return Prop;
		}
		return nullptr; 
	}

	int GetOffset(void* TargetProp) {
		return *reinterpret_cast<int*>(__int64(TargetProp) + 0x3C);
	}

	template<class T>
	T GetValuePointer(UObject* Object, void* Prop) {
		return reinterpret_cast<T>(DWORD(Object) + GetOffset(Prop));
	}

	int GetPropByClass(Unreal::UObject* TargetClass, std::string TargetChildName) {
		UObject* Prop = nullptr;
		UStruct* Class = (UStruct*)TargetClass;
		if (Class->Children) {
			Prop = FindChild(Class, TargetChildName);
		}
		if (Prop == nullptr) {
			UStruct* Struct = reinterpret_cast<UStruct*>(Class)->Super;
			while (Struct)
			{
				if (Struct->Children) {
					Prop = FindChild(Struct, TargetChildName);
					if (Prop != nullptr) {
						break;
					}
				}
				Struct = Struct->Super;
			}
		}
		return GetOffset(Prop);
	}

	template<class T = Unreal::UObject**>
	T Find(UObject* TargetObject, std::string TargetChildName) {
		UObject* Prop = nullptr;
		UStruct* Class = (UStruct*)TargetObject->Class;
		if (Class->Children) {
			Prop = FindChild(Class, TargetChildName);
		}
		if (Prop == nullptr) {
			UStruct* Struct = reinterpret_cast<UStruct*>(TargetObject->Class)->Super;
			while (Struct)
			{
				if (Struct->Children) {
					Prop = FindChild(Struct, TargetChildName);
					if (Prop != nullptr) {
						break;
					}
				}
				Struct = Struct->Super;
			}
		}
		if (Prop->IsValid()) {
			return GetValuePointer<T>(TargetObject, Prop);
		}
		else {
			LOG("Failed to Find Offset: " + TargetChildName);
			Sleep(2000);
			return nullptr;
		}
	}
}

#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
void DumpObjects() {
	std::ofstream log("Objects.txt");
	for (int i = 0; i < GObjs->ObjObject.Num; i++) {
		Unreal::UObject* Object = GObjs->ObjObject.Objects[i].Object;
		std::string ObjName = Object->GetName();
		std::string item = "\nName: " + ObjName;
		log << item;
	}
}

struct BitField
{
	unsigned char A : 1;
	unsigned char B : 1;
	unsigned char C : 1;
	unsigned char D : 1;
	unsigned char E : 1;
	unsigned char F : 1;
	unsigned char G : 1;
	unsigned char H : 1;
};


enum class ESpawnActorCollisionHandlingMethod : uint8_t
{
	Undefined = 0,
	AlwaysSpawn = 1,
	AdjustIfPossibleButAlwaysSpawn = 2,
	AdjustIfPossibleButDontSpawnIfColliding = 3,
	DontSpawnIfColliding = 4,
	ESpawnActorCollisionHandlingMethod_MAX = 5
};

struct FActorSpawnParameters
{
	FActorSpawnParameters() : Name(), Template(nullptr), Owner(nullptr), Instigator(nullptr), OverrideLevel(nullptr), SpawnCollisionHandlingOverride(ESpawnActorCollisionHandlingMethod::AlwaysSpawn), bRemoteOwned(0), bNoFail(1),
		bDeferConstruction(0),
		bAllowDuringConstructionScript(1),
		ObjectFlags(),
		bNoCollisionFail(1)
	{
	}
	;

	Unreal::FName Name;

	Unreal::UObject* Template;

	Unreal::UObject* Owner;

	Unreal::UObject* Instigator;

	Unreal::UObject* OverrideLevel;

	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

	uint16_t bNoCollisionFail : 1;

	uint16_t bRemoteOwned : 1;

	uint16_t bNoFail : 1;

	uint16_t bDeferConstruction : 1;

	uint16_t bAllowDuringConstructionScript : 1;

	int32_t ObjectFlags;
};