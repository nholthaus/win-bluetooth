#include <bluetoothRadio.h>
#include <bluetoothException.h>
#include <bluetoothUuids.h>
#include <bluetoothAddress.h>

#include <winsock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>
#include <initguid.h>

#define ERR HRESULT_FROM_WIN32(GetLastError())

std::unique_ptr<BluetoothUuid> BluetoothRadio::m_uuid = std::make_unique<BluetoothUuid>();

BluetoothRadio::BluetoothRadio(void* radioHandle)
	: m_handle(radioHandle)
	, m_radioInfo(new BLUETOOTH_RADIO_INFO)
{
	if (!m_handle)
	{
		m_isValid = false;
		m_address = 0;
		m_name = "INVALID";
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
		m_name = QString::fromWCharArray(info->szName);
	}
}

BluetoothRadio::BluetoothRadio(BluetoothRadio&& other)
	: m_handle(other.m_handle)
	, m_radioInfo(other.m_radioInfo)
	, m_isValid(std::move(other.m_isValid))
	, m_address(std::move(other.m_address))
	, m_name(std::move(other.m_name))
	, m_class(std::move(other.m_class))
	, m_lmpSubversion(other.m_lmpSubversion)
	, m_manufacturer(other.m_manufacturer)
{
	other.m_handle = nullptr;
	other.m_radioInfo = nullptr;
}

BluetoothRadio& BluetoothRadio::operator=(BluetoothRadio&& other)
{
	m_handle = other.m_handle; other.m_handle = nullptr;
	m_radioInfo = other.m_radioInfo; other.m_radioInfo = nullptr;
	m_isValid = std::move(other.m_isValid);
	m_address = std::move(other.m_address);
	m_name = std::move(other.m_name);
	m_class = std::move(other.m_class);
	m_lmpSubversion = other.m_lmpSubversion;
	m_manufacturer = other.m_manufacturer;
	return *this;
}

BluetoothRadio::~BluetoothRadio()
{
	CloseHandle(m_handle);
}

bool BluetoothRadio::connectTo(BluetoothAddress address)
{
	throw BluetoothException("unimplemented");
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
	setConnectable(enable);
	BluetoothEnableDiscovery(m_handle, enable);

	if(discoverable() != enable)
		throw BluetoothException("Could not change discoverable state");
}

bool BluetoothRadio::connectable() const
{
	return BluetoothIsConnectable(m_handle);
}

void BluetoothRadio::setConnectable(bool connectable)
{
	BluetoothEnableIncomingConnections(m_handle, connectable);

	if(connectable != this->connectable())
		throw BluetoothException("Could not change connection state");
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

QString BluetoothRadio::name() const
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

bool BluetoothRadio::operator==(const QString& name) const
{
	return name == m_name;
}


