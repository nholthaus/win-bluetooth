#include <bluetoothRadio.h>
#include <bluetoothException.h>
#include <bluetoothUuids.h>

#include <winsock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>
#include <initguid.h>

std::unique_ptr<BluetoothUuid> BluetoothRadio::m_uuid = std::make_unique<BluetoothUuid>();

BluetoothRadio::BluetoothRadio(void* radioHandle)
	: m_handle(radioHandle)
	, m_radioInfo(new BLUETOOTH_RADIO_INFO)
{
	if (!m_handle)
	{
		m_isValid = false;
		m_address = 0;
		m_name = L"INVALID";
	}
	else
	{
		auto* info = static_cast<BLUETOOTH_RADIO_INFO*>(m_radioInfo);
		info->dwSize = sizeof(BLUETOOTH_RADIO_INFO);

		BluetoothGetRadioInfo(m_handle, info);

		m_address = info->address.ullLong;
		m_class = info->ulClassofDevice;
		m_lmpSubversion = info->lmpSubversion;
		m_manufacturer = info->manufacturer;
		m_name = info->szName;
	}
}

BluetoothRadio::~BluetoothRadio()
{
	CloseHandle(m_handle);
}

bool BluetoothRadio::connectTo(BluetoothAddress address)
{
	if (!address)
		return false;

	SOCKADDR_BTH btAddress;
	btAddress.addressFamily = AF_BTH;
	btAddress.serviceClassId = (*m_uuid)(Protocol::MSDN);
	btAddress.port = 0;
	btAddress.btAddr = address;

	// create a socket
	SOCKET sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (sock == INVALID_SOCKET) 
	{
		throw BluetoothException(HRESULT_FROM_WIN32(WSAGetLastError()));
		return false;
	}

	// connect it to the remote device
	if(SOCKET_ERROR == connect(sock, (struct sockaddr *) &btAddress, sizeof(SOCKADDR_BTH)))
		throw BluetoothException(HRESULT_FROM_WIN32(WSAGetLastError()));

	return true;
}

unsigned short BluetoothRadio::manufacturer() const
{
	return m_manufacturer;
}

bool BluetoothRadio::discoverable() const
{
	return BluetoothIsDiscoverable(m_handle);
}

void BluetoothRadio::setDiscoverable(bool enable)
{
	BluetoothEnableDiscovery(m_handle, enable);
}

bool BluetoothRadio::connectable() const
{
	return BluetoothIsConnectable(m_handle);
}

void BluetoothRadio::setConnectable(bool connectable)
{
	BluetoothEnableIncomingConnections(m_handle, connectable);
}

void* BluetoothRadio::handle()
{
	return m_handle;
}

const void* BluetoothRadio::handle() const
{
	return m_handle;
}

bool BluetoothRadio::isValid() const
{
	return m_isValid;
}

unsigned long long BluetoothRadio::address() const
{
	return m_address;
}

std::wstring BluetoothRadio::name() const
{
	return m_name;
}

unsigned long BluetoothRadio::classOfDevice() const
{
	return m_class;
}

bool BluetoothRadio::operator==(const unsigned long long address) const
{
	return address == m_address;
}

bool BluetoothRadio::operator==(const std::wstring_view name) const
{
	return name == m_name;
}


