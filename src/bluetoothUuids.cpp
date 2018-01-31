//------------------------------
//	INCLUDES
//------------------------------
#include "bluetoothUuids.h"

//--------------------------------------------------------------------------------------------------
//	UUIDS
//--------------------------------------------------------------------------------------------------

std::unordered_map<Protocol, QUuid> BluetoothUuid::m_protocolUuids
{
	{ Protocol::RFCOMM, QUuid((int)Protocol::RFCOMM << 16, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB) },
};

 std::unordered_map<ServiceClass, QUuid> BluetoothUuid::m_serviceClassUuids
 {

	 // application specific custom UUIDs
	 { ServiceClass::MSDNBluetoothConnectionExample, QUuid(0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74) },
 };

 //--------------------------------------------------------------------------------------------------
 //	BluetoothUuid (public ) []
 //--------------------------------------------------------------------------------------------------
 BluetoothUuid::BluetoothUuid()
	 : m_uuid(0)
 {

 }

 //--------------------------------------------------------------------------------------------------
 //	BluetoothUuid () []
 //--------------------------------------------------------------------------------------------------
 BluetoothUuid::BluetoothUuid(Protocol protocol)
	 : m_uuid(m_protocolUuids[protocol])
 {

 }

 //--------------------------------------------------------------------------------------------------
 //	BluetoothUuid () []
 //--------------------------------------------------------------------------------------------------
 BluetoothUuid::BluetoothUuid(ServiceClass serviceClass)
	 : m_uuid(m_serviceClassUuids[serviceClass])
 {

 }

 //--------------------------------------------------------------------------------------------------
 //	toString (public ) []
 //--------------------------------------------------------------------------------------------------
QString BluetoothUuid::toString() const
{
	return m_uuid.toString().toUpper();
}

 //--------------------------------------------------------------------------------------------------
 //	operator GUID (public ) []
 //--------------------------------------------------------------------------------------------------
 BluetoothUuid::operator QUuid() const
 {
	 return m_uuid;
 }

 //--------------------------------------------------------------------------------------------------
 //	operator GUID (public ) []
 //--------------------------------------------------------------------------------------------------
 BluetoothUuid::operator GUID() const
 {
	 return m_uuid;
 }
