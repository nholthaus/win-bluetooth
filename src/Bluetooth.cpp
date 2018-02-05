#include <Bluetooth.h>
#include <bluetoothException.h>
#include <bluetoothRadio.h>

#include <utility>
#include <cassert>
#include <algorithm>
#include <QHostInfo>

#include <winsock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>

#pragma comment(lib, "Bthprops.lib")
#pragma comment(lib, "Ws2_32.lib")

//------------------------------
//	MACROS
//------------------------------

#define ERR HRESULT_FROM_WIN32(GetLastError())

//------------------------------
//	STATIC MEMBERS
//------------------------------

std::unordered_map<QString, BluetoothRadio> Bluetooth::m_localRadios;
std::unordered_map<QString, BluetoothDevice> Bluetooth::m_remoteDevices;
BluetoothDevice Bluetooth::m_invalidDevice;
BluetoothRadio Bluetooth::m_invalidRadio;
QString Bluetooth::m_hostname(QHostInfo::localHostName().toUpper());

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
bool Bluetooth::enumerateLocalRadios(bool refreshList /*= false*/)
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
		{
			BluetoothRadio r(radio);
			m_localRadios[r.name()] = std::move(r);
		}
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
			BluetoothRadio r(radio);
			m_localRadios[r.name()] = std::move(r);
		}

		if (GetLastError() != ERROR_NO_MORE_ITEMS)
			throw BluetoothException(ERR);
	}

	return m_localRadios.size();
}

bool Bluetooth::enumerateRemoteDevices(bool refreshList /*= false*/)
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

		for (const auto& [radioName, radio] : localRadios())
		{
			btdsp.hRadio = (HANDLE)radio.handle();
	
			// Get the first local radio
			BluetoothFindDeviceHandle device = BluetoothFindFirstDevice(&btdsp, &btdi);
			if (device) 
			{
				m_remoteDevices.emplace(std::piecewise_construct,
					std::forward_as_tuple(QString::fromWCharArray(btdi.szName)), 
					std::forward_as_tuple(btdsp.hRadio, &btdi));
			}
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
				m_remoteDevices.emplace(std::piecewise_construct,
					std::forward_as_tuple(QString::fromWCharArray(btdi.szName)),
					std::forward_as_tuple(btdsp.hRadio, &btdi));
			}
	
			if (GetLastError() != ERROR_NO_MORE_ITEMS)
				throw BluetoothException(ERR);
		}
	}

	return m_remoteDevices.size();
}

//--------------------------------------------------------------------------------------------------
//	lookupServices (private ) [static ]
//--------------------------------------------------------------------------------------------------

BOOL __stdcall callback(ULONG uAttribId, LPBYTE pValueStream, ULONG cbStreamSize, LPVOID pvParam)

{

	SDP_ELEMENT_DATA element;

	// Just a verification, uncomment to see the output!!!

	//printf("Callback() uAttribId: %ul\n", uAttribId);

	//printf("Callback() pValueStream: %d\n ", pValueStream);

	//printf("Callback() cbStreamSize: %ul\n ", cbStreamSize);



	if (BluetoothSdpGetElementData(pValueStream, cbStreamSize, &element) != ERROR_SUCCESS)

	{

		// Just a verification

		// printf("BluetoothSdpGetElementData() failed with error code %ld\n", WSAGetLastError());

		return FALSE;

	}

	else

	{

		// Just a verification

		// printf("BluetoothSdpGetElementData() is OK!\n");

		return TRUE;

	}

}


bool Bluetooth::lookupServices(const BluetoothDevice& device)
{
	WSAPROTOCOL_INFO protocolInfo;
	int protocolInfoSize;
	WSAQUERYSET querySet, *pResults, querySet2;
	HANDLE hLookup, hLookup2;
	int result;
	static int i;
	BYTE buffer[2000];
	BYTE buffer1[2000];
	DWORD bufferLength, flags, addressSize, bufferLength1;
	CSADDR_INFO *pCSAddr;
	BTH_DEVICE_INFO *pDeviceInfo;
	char addressAsString[2000];
	BLOB *pBlob;
	GUID protocol;

	protocolInfoSize = sizeof(protocolInfo);

	// Query set criteria
	memset(&querySet, 0, sizeof(querySet));
	querySet.dwSize = sizeof(querySet);
	querySet.dwNameSpace = NS_BTH;

	// Set the flags for query
	flags = LUP_RETURN_NAME | LUP_CONTAINERS | LUP_RETURN_ADDR | LUP_FLUSHCACHE |
		LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE;

	// Start a device in range query...
	result = WSALookupServiceBegin(&querySet, flags, &hLookup);

	// If OK
	if (result == 0)
	{
		printf("	WSALookupServiceBegin() is OK!\n");
		i = 0;

		while (result == 0)
		{
			bufferLength = sizeof(buffer);
			pResults = (WSAQUERYSET *)&buffer[0];

			// Next query...
			result = WSALookupServiceNext(hLookup, flags, &bufferLength, pResults);
			if (result != 0)
			{
				printf("	WSALookupServiceNext() failed with error code %ld\n", WSAGetLastError());
				printf("	%s\n", BluetoothException(ERR).what());
			}
			else
			{
				// Get the device info, name, address etc
				printf("	WSALookupServiceNext() is OK!\n");
				printf("	The service instance name is %S\n", pResults->lpszServiceInstanceName);
				pCSAddr = (CSADDR_INFO *)pResults->lpcsaBuffer;
				pDeviceInfo = (BTH_DEVICE_INFO *)pResults->lpBlob;
				memset(&querySet2, 0, sizeof(querySet2));
				querySet2.dwSize = sizeof(querySet2);
				protocol = RFCOMM_PROTOCOL_UUID;
				querySet2.lpServiceClassId = &protocol;
				querySet2.dwNameSpace = NS_BTH;

				addressSize = sizeof(addressAsString);

// 				// Print the local bluetooth device address...
// 				if (WSAAddressToString(pCSAddr->LocalAddr.lpSockaddr, pCSAddr->LocalAddr.iSockaddrLength,
// 					&protocolInfo, addressAsString, &addressSize) == 0)
// 				{
// 					printf("	WSAAddressToString() for local address is fine!\n");
// 					printf("	The local address: %S\n", addressAsString);
// 				}
// 				else
// 					printf("	WSAAddressToString() for local address failed with error code %ld\n", WSAGetLastError());
// 
// 				addressSize = sizeof(addressAsString);
// 
// 				// Print the remote bluetooth device address...
// 				if (WSAAddressToString(pCSAddr->RemoteAddr.lpSockaddr, pCSAddr->RemoteAddr.iSockaddrLength,
// 					&protocolInfo, addressAsString, &addressSize) == 0)
// 				{
// 					printf("	WSAAddressToString() for remote address is fine!\n");
// 					printf("	The remote device address: %S\n", addressAsString);
// 				}
// 				else
// 					printf("	WSAAddressToString() for remote address failed with error code %ld\n", WSAGetLastError());

				// Prepare for service query set
				querySet2.lpszContext = (LPSTR)device.addressString().toStdString().c_str();

				flags = LUP_FLUSHCACHE | LUP_RETURN_NAME | LUP_RETURN_TYPE |
					LUP_RETURN_ADDR | LUP_RETURN_BLOB | LUP_RETURN_COMMENT;

				// Start service query
				result = WSALookupServiceBegin(&querySet2, flags, &hLookup2);
				if (result == 0)
				{
					printf("	WSALookupServiceBegin() is OK!\n");
					while (result == 0)
					{
						bufferLength1 = sizeof(buffer1);
						pResults = (WSAQUERYSET *)&buffer1;

						// Next service query
						result = WSALookupServiceNext(hLookup2, flags, &bufferLength1, pResults);

						if (result == 0)
						{
							// Populate the service info
							printf("	WSALookupServiceNext() is OK!\n");
							printf("	WSALookupServiceNext() - service instance name: %S\n",
								pResults->lpszServiceInstanceName);
							printf("	WSALookupServiceNext() - comment (if any): %s\n", pResults->lpszComment);
							pCSAddr = (CSADDR_INFO *)pResults->lpcsaBuffer;

							// Extract the sdp info
							if (pResults->lpBlob)
							{
								pBlob = (BLOB*)pResults->lpBlob;
								if (!BluetoothSdpEnumAttributes(pBlob->pBlobData, pBlob->cbSize, callback, 0))
								{
									printf("BluetoothSdpEnumAttributes() failed with error code %ld\n", WSAGetLastError());
								}
								else
								{
									printf("BluetoothSdpEnumAttributes() #%d is OK!\n", i++);
								}
							}
						}
						else
						{
							printf("	WSALookupServiceNext() failed with error code %ld\n", WSAGetLastError());
							printf("	Error code = 11011 ~ WSA_E_NO_MORE ~ No more device!\n");
						}
					}

					// Close the handle to service query
					if (WSALookupServiceEnd(hLookup2) == 0)
						printf("WSALookupServiceEnd(hLookup2) is fine!\n", WSAGetLastError());
					else
						printf("WSALookupServiceEnd(hLookup2) failed with error code %ld\n");
				}
				else
					printf("WSALookupServiceBegin() failed with error code %ld\n", WSAGetLastError());
			}
		}

		// Close handle to the device query
		if (WSALookupServiceEnd(hLookup) == 0)
			printf("WSALookupServiceEnd(hLookup) is fine!\n", WSAGetLastError());
		else
			printf("WSALookupServiceEnd(hLookup) failed with error code %ld\n");
	}
	else
	{
		printf("WSALookupServiceBegin() failed with error code %ld\n", WSAGetLastError());
	}// end WSALookupServiceBegin()

	return true;
}

BluetoothRadio& Bluetooth::localRadio(const QString& name, bool refreshList /*= false*/)
{
	enumerateLocalRadios(refreshList);
	return m_localRadios.count(name) ? m_localRadios[name] : m_invalidRadio;
}

BluetoothRadio& Bluetooth::localRadio(bool refreshList /*= false*/)
{
	return localRadio(m_hostname, refreshList);
}

std::unordered_map<QString, BluetoothRadio>& Bluetooth::localRadios(bool refreshList /*= false*/)
{
	enumerateLocalRadios(refreshList);
	return m_localRadios;
}

BluetoothDevice& Bluetooth::remoteDevice(const QString& name, bool refreshList /*= false*/)
{
	enumerateRemoteDevices(refreshList);
	return m_remoteDevices.count(name) ? m_remoteDevices[name] : m_invalidDevice;
}

std::unordered_map<QString, BluetoothDevice>& Bluetooth::remoteDevices(bool refreshList /*= false*/)
{
	enumerateRemoteDevices(refreshList);
	return m_remoteDevices;
}

//--------------------------------------------------------------------------------------------------
//	name (public ) [static ]
//--------------------------------------------------------------------------------------------------
QString Bluetooth::name(const BluetoothAddress& address)
{
	for (const auto& [name, radio] : m_localRadios)
	{
		if (radio.address() == address)
			return name;
	}
	for (const auto&[name, device] : m_remoteDevices)
	{
		if (device.address() == address)
			return name;
	}

	return "INVALID";
}
