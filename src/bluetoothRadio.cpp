#include <bluetoothRadio.h>
#include <windows.h>
#include <bluetoothapis.h>

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

