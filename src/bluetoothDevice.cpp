#include <bluetoothDevice.h>
#include <windows.h>
#include <bluetoothapis.h>
#include <bluetoothUtils.h>
#include <bluetoothAddress.h>
#include <bluetoothException.h>
#include <functional>

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

		// set authentication callback
		HBLUETOOTH_AUTHENTICATION_REGISTRATION authHandle;
		bool param;
		static std::function<bool(LPVOID, PBLUETOOTH_DEVICE_INFO)> callback{ [](LPVOID param, PBLUETOOTH_DEVICE_INFO info) -> bool { throw BluetoothException("OMG AUTHENTICATION!"); } };
		if (ERROR_SUCCESS != BluetoothRegisterForAuthentication(DEVINFO, &authHandle, (PFN_AUTHENTICATION_CALLBACK)&callback, &param))
			throw BluetoothException("That sucks!");
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

BluetoothAddress BluetoothDevice::address() const
{
	return m_deviceInfo ? DEVINFO->Address.ullLong : 0;
}

QString BluetoothDevice::addressString() const
{
	return address();
}

QString BluetoothDevice::name() const
{
	return m_deviceInfo ? QString::fromWCharArray(DEVINFO->szName) : "INVALID";
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

QDateTime BluetoothDevice::lastSeen()
{
	refresh();
	return m_deviceInfo ? systemTimeToDateTime(DEVINFO->stLastSeen) : QDateTime();
}

QDateTime BluetoothDevice::lastUsed()
{
	refresh();
	return m_deviceInfo ? systemTimeToDateTime(DEVINFO->stLastUsed) : QDateTime();
}

bool BluetoothDevice::operator<(const BluetoothDevice& other)
{
	return address() < other.address();
}

bool BluetoothDevice::operator==(BluetoothAddress address) const
{
	return address == DEVINFO->Address.ullLong;
}

bool BluetoothDevice::operator==(const QString& name) const
{
	return (name.compare(QString::fromWCharArray(DEVINFO->szName)) == 0);
}

BluetoothDevice::operator BluetoothAddress() const
{
	return address();
}