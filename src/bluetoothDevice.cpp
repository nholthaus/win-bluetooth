#include <bluetoothDevice.h>
#include <windows.h>
#include <bluetoothapis.h>
#include <bluetoothUtils.h>

#define DEVINFO static_cast<BLUETOOTH_DEVICE_INFO*>(m_deviceInfo)

BluetoothDevice::BluetoothDevice(void* radioHandle /*= nullptr*/, void* deviceInfo /*= nullptr*/)
	: m_radioHandle(radioHandle)
	, m_deviceInfo(new BLUETOOTH_DEVICE_INFO)
{
	if (deviceInfo)
	{
		auto info = static_cast<BLUETOOTH_DEVICE_INFO*>(deviceInfo);
			
		DEVINFO->dwSize = info->dwSize;
		DEVINFO->Address = info->Address;
		DEVINFO->ulClassofDevice = info->ulClassofDevice;
		DEVINFO->fConnected = info->fConnected;
		DEVINFO->fRemembered = info->fRemembered;
		DEVINFO->fAuthenticated = info->fAuthenticated;
		DEVINFO->stLastSeen = info->stLastSeen;
		DEVINFO->stLastUsed = info->stLastUsed;
		wcscpy_s(DEVINFO->szName, info->szName);
	}
}

BluetoothDevice::~BluetoothDevice()
{
	delete m_deviceInfo;
}

unsigned long long BluetoothDevice::address() const
{
	return DEVINFO->Address.ullLong;
}

std::wstring BluetoothDevice::name() const
{
	return DEVINFO->szName;
}

unsigned long BluetoothDevice::classOfDevice() const
{
	return DEVINFO->ulClassofDevice;
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
	return m_connected;
}

bool BluetoothDevice::remembered()
{
	refresh();
	return m_remembered;
}

bool BluetoothDevice::authenticated()
{
	refresh();
	return m_authenticated;
}

std::string_view BluetoothDevice::lastSeen()
{
	refresh();
	return m_lastSeen;
}

std::string_view BluetoothDevice::lastUsed()
{
	refresh();
	return m_lastUsed;
}

bool BluetoothDevice::operator==(const unsigned long long address) const
{
	return address == DEVINFO->Address.ullLong;
}

bool BluetoothDevice::operator==(const std::wstring_view name) const
{
	return name == DEVINFO->szName;
}