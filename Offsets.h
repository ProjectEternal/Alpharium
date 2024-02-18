#pragma once
namespace Offsets {
	namespace AOnlineBeacon {
		int InitHost = 0x10B1E80;
		int NotifyAcceptingConnection = 0x0; //Fixes BeaconRequests being Paused (PauseBeaconReqeusts)
	}

	namespace UNetDriver {
		int TickFlush = 0x0;

		int ReplicationFrame = 0x0;
	}

	namespace UChannel {
		int ReplicateActor = 0x0;
		int SetChannelActor = 0x0;

		int CloseChannel = 0x0; //Not needed but would be nice

	}

	namespace APlayerController {
		int ClientSendAdjustment = 0x0;
	}

	namespace AFortPlayerPawn {
		int EquipWeaponData = 0x40A8D0;
	}

	namespace AActor {
		int CallPreReplication = 0x0;
	}
}