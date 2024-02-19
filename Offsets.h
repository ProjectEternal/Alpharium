#pragma once
namespace Offsets {
	namespace AOnlineBeacon {
		int InitHost = 0x10B1E80;
		int NotifyControlMessage = 0x10B40F0;
		int PauseBeaconReqeusts = 0x3AB760;
	}

	namespace UNetDriver {
		int TickFlush = 0x13B6E70;
		int InitListen = 0x10B2120;
		int SetWorld = 0x13B4D70;

		int ReplicationFrame = 0x1DC;
	}

	namespace UWorld {
		int WelcomePlayer = 0x15D4280;
		int NotifyControlMessage = 0x15CA480;
	}

	namespace UChannel {
		int ReplicateActor = 0x127ED10;
		int SetChannelActor = 0x1281D70;

		int CloseChannel = 0x0; //Not needed but would be nice
	}

	namespace UNetConnection {
		int CreateChannel = 0x13A08D0;
	}

	namespace APlayerController {
		int SendClientAdjustment = 0x1486B60;
	}

	namespace AFortPlayerPawn {
		int EquipWeaponData = 0x40A8D0;
	}

	namespace AActor {
		int CallPreReplication = 0x0;
	}

	namespace Misc {
		int ValidationFailure = 0x3ABAA0;
		int ReservationFailure = 0x3ABC40;
	}
}