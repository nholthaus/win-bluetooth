#include <bluetoothRadio.h>
#include <bluetoothException.h>

#include <utility>
#include <cassert>

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
bool BluetoothRadio::enumerateLocalRadios()
{
	// this thing is required by the API but is pointless for our purposes. Probably used to
	// differentiate x86/x64 systems
	BLUETOOTH_FIND_RADIO_PARAMS btfrp;
	btfrp.dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

	HANDLE radio;

	// Get the first local radio
	BluetoothFindRadioHandle radioFindHandle = BluetoothFindFirstRadio(&btfrp, &radio);
	if (radio)
		m_radios.push_back(radio);
	else
	{
		// this PC doesn't have any radios
		if (ERR == ERROR_NO_MORE_ITEMS)
			return false;
		else
			throw BluetoothException(ERR);
	}

	// get the rest of the local radios
	while (BluetoothFindNextRadio(radioFindHandle, &radio))
	{
		m_radios.push_back(radio);
	}

	if (ERR != ERROR_NO_MORE_ITEMS)
		throw BluetoothException(ERR);

	return m_radios.size();
}

