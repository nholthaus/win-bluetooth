#include <Bluetooth.h>
#include <bluetoothException.h>
#include <bluetoothRadio.h>

#include <utility>
#include <cassert>
#include <algorithm>

#include <winsock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>

#pragma comment(lib, "Bthprops.lib")
#pragma comment(lib, "Ws2_32.lib")

//------------------------------
//	MACROS
//------------------------------

#define ERR HRESULT_FROM_WIN32(GetLastError())

//--------------------------------------------------------------------------------------------------
//	RAII WRAPPERS
//--------------------------------------------------------------------------------------------------

class BluetoothFindRadioHandle
{
public:

	BluetoothFindRadioHandle(HBLUETOOTH_RADIO_FIND handle) noexcept : m_handle(handle) {}
	~BluetoothFindRadioHandle() 
	{ 
		// in theory this can fail, but not much we can do about it
		if (!BluetoothFindRadioClose(m_handle))
			assert(true);	// if you hit this assert, you probably need to do some serious debugging
	}
	operator HBLUETOOTH_RADIO_FIND() noexcept { return m_handle; }

private:

	HBLUETOOTH_RADIO_FIND m_handle;
};

class BluetoothFindDeviceHandle
{
public:

	BluetoothFindDeviceHandle(HBLUETOOTH_DEVICE_FIND handle) noexcept : m_handle(handle) {}
	~BluetoothFindDeviceHandle()
	{
		// in theory this can fail, but not much we can do about it
		if (!BluetoothFindDeviceClose(m_handle))
			assert(true);	// if you hit this assert, you probably need to do some serious debugging
	}
	operator HBLUETOOTH_DEVICE_FIND() noexcept { return m_handle; }

private:

	HBLUETOOTH_DEVICE_FIND m_handle;
};

//--------------------------------------------------------------------------------------------------
//	METHODS
//--------------------------------------------------------------------------------------------------
bool Bluetooth::enumerateLocalRadios(bool refreshList /*= false*/) const
{
	if(refreshList || m_localRadios.empty())
	{
		// this thing is required by the API but is pointless for our purposes. Probably used to
		// differentiate x86/x64 systems
		BLUETOOTH_FIND_RADIO_PARAMS btfrp;
		btfrp.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

		HANDLE radio;

		// Get the first local radio
		BluetoothFindRadioHandle radioFindHandle = BluetoothFindFirstRadio(&btfrp, &radio);
		if (radio)
			m_localRadios.emplace_back(radio);
		else
		{
			// this PC doesn't have any radios
			if (GetLastError() == ERROR_NO_MORE_ITEMS)
				return false;
			else
				throw BluetoothException(ERR);
		}

		// get the rest of the local radios
		while (BluetoothFindNextRadio(radioFindHandle, &radio))
		{
			m_localRadios.emplace_back(radio);
		}

		if (GetLastError() != ERROR_NO_MORE_ITEMS)
			throw BluetoothException(ERR);
	}

	return m_localRadios.size();
}


bool Bluetooth::init()
{
	// Ask for Winsock version 2.2.
	WSADATA WSAData = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
		throw BluetoothException("Unable to initialize Winsock version 2.2");

	return true;
}

bool Bluetooth::enumerateRemoteDevices(bool refreshList /*= false*/) const
{
	if (refreshList || m_remoteDevices.empty())
	{
		// this thing is required by the API but is pointless for our purposes. Probably used to
		// differentiate x86/x64 systems
		BLUETOOTH_DEVICE_SEARCH_PARAMS btdsp;
		btdsp.fIssueInquiry = true;
		btdsp.fReturnAuthenticated = true;
		btdsp.fReturnConnected = true;
		btdsp.fReturnRemembered = true;
		btdsp.fReturnUnknown = true;
		btdsp.cTimeoutMultiplier = 1;
		btdsp.dwSize = sizeof(btdsp);

		BLUETOOTH_DEVICE_INFO btdi;
		btdi.dwSize = sizeof(btdi);

		for (auto& radio : localRadios())
		{
			btdsp.hRadio = (HANDLE)radio.handle();
	
			// Get the first local radio
			BluetoothFindDeviceHandle device = BluetoothFindFirstDevice(&btdsp, &btdi);
			if (device)
				m_remoteDevices.emplace_back(btdsp.hRadio, &btdi);
			else
			{
				// this PC doesn't have any radios
				if (GetLastError() == ERROR_NO_MORE_ITEMS)
					return false;
				else
					throw BluetoothException(ERR);
			}
	
			// get the rest of the local radios
			while (BluetoothFindNextDevice(device, &btdi))
			{
				m_remoteDevices.emplace_back(btdsp.hRadio, &btdi);
			}
	
			if (GetLastError() != ERROR_NO_MORE_ITEMS)
				throw BluetoothException(ERR);
		}
	}

	return m_remoteDevices.size();
}

Bluetooth::Bluetooth()
{
	init();
}

BluetoothRadio& Bluetooth::localRadio(unsigned int index /*= 0*/)
{
	enumerateLocalRadios();
	return m_localRadios.at(index);
}

const BluetoothRadio& Bluetooth::localRadio(unsigned int index /*= 0*/) const
{
	enumerateLocalRadios();
	return m_localRadios.at(index);
}

BluetoothRadio& Bluetooth::localRadio(const std::wstring_view& name, bool refreshList /*= false*/)
{
	enumerateLocalRadios(refreshList);

	auto itr = std::find(m_localRadios.begin(), m_localRadios.end(), name);
	return (itr == m_localRadios.end()) ? m_invalidRadio : *itr;
}

const BluetoothRadio& Bluetooth::localRadio(const std::wstring_view& name, bool refreshList /*= false*/) const
{
	enumerateLocalRadios(refreshList);

	auto itr = std::find(m_localRadios.cbegin(), m_localRadios.cend(), name);
	return (itr == m_localRadios.end()) ? m_invalidRadio : *itr;
}

std::vector<BluetoothRadio>& Bluetooth::localRadios(bool refreshList /*= false*/)
{
	enumerateLocalRadios(refreshList);
	return m_localRadios;
}

const std::vector<BluetoothRadio>& Bluetooth::localRadios(bool refreshList /*= false*/) const
{
	enumerateLocalRadios(refreshList);
	return m_localRadios;
}

BluetoothDevice& Bluetooth::remoteDevice(unsigned int index /*= 0*/)
{
	return m_remoteDevices.at(index);
}

BluetoothDevice& Bluetooth::remoteDevice(const std::wstring_view& name, bool refreshList /*= false*/)
{
	enumerateRemoteDevices(refreshList);

	auto itr = std::find(m_remoteDevices.begin(), m_remoteDevices.end(), name);
	return (itr == m_remoteDevices.end()) ? m_invalidDevice : *itr;
}

const BluetoothDevice& Bluetooth::remoteDevice(unsigned int index /*= 0*/) const
{
	return m_remoteDevices.at(index);
}

const BluetoothDevice& Bluetooth::remoteDevice(const std::wstring_view& name, bool refreshList /*= false*/) const
{
	enumerateRemoteDevices(refreshList);

	auto itr = std::find(m_remoteDevices.cbegin(), m_remoteDevices.cend(), name);
	return (itr == m_remoteDevices.end()) ? m_invalidDevice : *itr;
}

std::vector<BluetoothDevice>& Bluetooth::remoteDevices(bool refreshList /*= false*/)
{
	enumerateRemoteDevices(refreshList);
	return m_remoteDevices;
}

const std::vector<BluetoothDevice>& Bluetooth::remoteDevices(bool refreshList /*= false*/) const
{
	enumerateRemoteDevices(refreshList);
	return m_remoteDevices;
}
