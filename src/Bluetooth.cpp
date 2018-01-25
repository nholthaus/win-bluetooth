#include <Bluetooth.h>
#include <bluetoothException.h>
#include <bluetoothRadio.h>

#include <utility>
#include <cassert>
#include <algorithm>

#include <windows.h>
#include <BluetoothAPIs.h>

#pragma comment(lib, "Bthprops.lib")

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

std::vector<BluetoothRadio>& Bluetooth::remoteDevices(bool refreshList /*= false*/)
{
	enumerateLocalRadios(refreshList);
	return m_remoteDevices;
}

const std::vector<BluetoothRadio>& Bluetooth::remoteDevices(bool refreshList /*= false*/) const
{
	enumerateLocalRadios(refreshList);
	return m_remoteDevices;
}
