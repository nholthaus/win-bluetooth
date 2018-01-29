//------------------------------
//	INCLUDES
//------------------------------
#include "bluetoothUuids.h"

//--------------------------------------------------------------------------------------------------
//	UUIDS
//--------------------------------------------------------------------------------------------------

std::unordered_map<Protocol, GUID> BluetoothUuid::m_uuids;

DEFINE_GUID(MSDN, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);
DEFINE_GUID(RFCOMM, 0x00030000, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

//--------------------------------------------------------------------------------------------------
//	METHODS
//--------------------------------------------------------------------------------------------------

BluetoothUuid::BluetoothUuid()
{
	m_uuids.try_emplace(Protocol::MSDN, MSDN);
	m_uuids.try_emplace(Protocol::RFCOMM, RFCOMM);
}

const GUID BluetoothUuid::operator()(Protocol uuid) const
{
	return m_uuids[uuid];
}

