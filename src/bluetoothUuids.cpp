//------------------------------
//	INCLUDES
//------------------------------
#include "bluetoothUuids.h"

//--------------------------------------------------------------------------------------------------
//	UUIDS
//--------------------------------------------------------------------------------------------------

// To add a UUID, use the `DEFINE_GUID` function and then add it to the `m_uuids` map

DEFINE_GUID(MSDNBluetoothConnectionExample, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);
DEFINE_GUID(RFCOMM, 0x00030000, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

 std::unordered_map<Protocol, const GUID*> BluetoothUuid::m_uuids
{
	{ Protocol::MSDNBluetoothConnectionExample, &MSDNBluetoothConnectionExample },
	{ Protocol::RFCOMM, &RFCOMM }
};