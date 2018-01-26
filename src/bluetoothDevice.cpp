#include <bluetoothDevice.h>
#include <windows.h>
#include <bluetoothapis.h>
#include <bluetoothUtils.h>

#define DEVINFO static_cast<BLUETOOTH_DEVICE_INFO*>(m_deviceInfo)

BluetoothDevice::BluetoothDevice(void* radioHandle /*= nullptr*/, void* deviceInfo /*= nullptr*/)
	: m_radioHandle(radioHandle)
{
	if (deviceInfo)
	{
		m_deviceInfo = new BLUETOOTH_DEVICE_INFO;	

		auto devinfo = DEVINFO;
		auto info = static_cast<BLUETOOTH_DEVICE_INFO*>(deviceInfo);
			
		wcscpy_s(devinfo->szName, info->szName);
		devinfo->dwSize = info->dwSize;
		devinfo->Address = info->Address;
		devinfo->ulClassofDevice = info->ulClassofDevice;
		devinfo->fConnected = info->fConnected;
		devinfo->fRemembered = info->fRemembered;
		devinfo->fAuthenticated = info->fAuthenticated;
		devinfo->stLastSeen = info->stLastSeen;
		devinfo->stLastUsed = info->stLastUsed;
	}
}

BluetoothDevice::BluetoothDevice(const BluetoothDevice& other)
{
	m_radioHandle = other.m_radioHandle;
	if (other.m_deviceInfo)
	{
		m_deviceInfo = new BLUETOOTH_DEVICE_INFO;

		auto devinfo = DEVINFO;
		auto info = static_cast<BLUETOOTH_DEVICE_INFO*>(other.m_deviceInfo);

		wcscpy_s(devinfo->szName, info->szName);
		devinfo->dwSize = info->dwSize;
		devinfo->Address = info->Address;
		devinfo->ulClassofDevice = info->ulClassofDevice;
		devinfo->fConnected = info->fConnected;
		devinfo->fRemembered = info->fRemembered;
		devinfo->fAuthenticated = info->fAuthenticated;
		devinfo->stLastSeen = info->stLastSeen;
		devinfo->stLastUsed = info->stLastUsed;
	}
}

BluetoothDevice::BluetoothDevice(BluetoothDevice&& other)
{
	m_radioHandle = other.m_radioHandle; other.m_radioHandle = nullptr;
	m_deviceInfo = other.m_deviceInfo; other.m_deviceInfo = nullptr;
}

BluetoothDevice::~BluetoothDevice()
{
	delete m_deviceInfo;
	m_deviceInfo = nullptr;
}

BluetoothDevice& BluetoothDevice::operator=(const BluetoothDevice& other)
{
	m_radioHandle = other.m_radioHandle;
	if (other.m_deviceInfo)
	{
		if(!m_deviceInfo)
			m_deviceInfo = new BLUETOOTH_DEVICE_INFO;

		auto devinfo = DEVINFO;
		auto info = static_cast<BLUETOOTH_DEVICE_INFO*>(other.m_deviceInfo);

		wcscpy_s(devinfo->szName, info->szName);
		devinfo->dwSize = info->dwSize;
		devinfo->Address = info->Address;
		devinfo->ulClassofDevice = info->ulClassofDevice;
		devinfo->fConnected = info->fConnected;
		devinfo->fRemembered = info->fRemembered;
		devinfo->fAuthenticated = info->fAuthenticated;
		devinfo->stLastSeen = info->stLastSeen;
		devinfo->stLastUsed = info->stLastUsed;
	}
	else
	{
		delete m_deviceInfo;
		m_deviceInfo = nullptr;
	}

	return *this;
}

BluetoothDevice& BluetoothDevice::operator=(BluetoothDevice&& other)
{
	delete m_deviceInfo;
	m_radioHandle = other.m_radioHandle; other.m_radioHandle = nullptr;
	m_deviceInfo = other.m_deviceInfo; other.m_deviceInfo = nullptr;
	return *this;
}

unsigned long long BluetoothDevice::address() const
{
	return m_deviceInfo ? DEVINFO->Address.ullLong : 0;
}

std::wstring BluetoothDevice::addressString() const
{
	return bluetoothAddressToString(address());
}

std::wstring BluetoothDevice::name() const
{
	return m_deviceInfo ? DEVINFO->szName : L"INVALID";
}

unsigned long BluetoothDevice::classOfDevice() const
{
	return m_deviceInfo ? DEVINFO->ulClassofDevice : 0;
}

void BluetoothDevice::refresh()
{
	BluetoothGetDeviceInfo(m_radioHandle, static_cast<BLUETOOTH_DEVICE_INFO*>(m_deviceInfo));
}

bool BluetoothDevice::isValid() const
{
	return m_deviceInfo != nullptr;
}

bool BluetoothDevice::connected()
{
	refresh();
	return m_deviceInfo ? DEVINFO->fConnected : false;
}

bool BluetoothDevice::remembered()
{
	refresh();
	return m_deviceInfo ? DEVINFO->fRemembered : false;
}

bool BluetoothDevice::authenticated()
{
	refresh();
	return m_deviceInfo ? DEVINFO->fAuthenticated : false;
}

std::string_view BluetoothDevice::lastSeen()
{
	refresh();
	return m_deviceInfo ? systemTimeToString(DEVINFO->stLastSeen) : "00-00-0000 00:00:00.000";
}

std::string_view BluetoothDevice::lastUsed()
{
	refresh();
	return m_deviceInfo ? systemTimeToString(DEVINFO->stLastUsed) : "00-00-0000 00:00:00.000";
}

bool BluetoothDevice::operator==(const unsigned long long address) const
{
	return address == DEVINFO->Address.ullLong;
}

bool BluetoothDevice::operator==(const std::wstring_view name) const
{
	return (name.compare(DEVINFO->szName) == 0);
}

BluetoothDevice::operator BluetoothAddress() const
{
	return address();
}