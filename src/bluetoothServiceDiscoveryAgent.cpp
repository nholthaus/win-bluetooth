#include <bluetoothServiceDiscoveryAgent.h>
#include <bluetoothAddress.h>
#include <bluetoothException.h>

#include <QAtomicInt>
#include <QDebug>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>

#define NOMINMAX
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
//	BluetoothServiceDiscoveryAgentPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothServiceDiscoveryAgentPrivate
{
public:

	Q_DECLARE_PUBLIC(BluetoothServiceDiscoveryAgent);
	BluetoothServiceDiscoveryAgentPrivate(BluetoothServiceDiscoveryAgent* parent) : q_ptr(parent) { }
	virtual ~BluetoothServiceDiscoveryAgentPrivate() = default;
	
	void setError(BluetoothServiceDiscoveryAgent::Error error, const QString& errorString)
	{
		Q_Q(BluetoothServiceDiscoveryAgent);
		this->error = error;
		this->errorString = errorString;
		this->stopDiscovery.store(true);
		q->error(error);
	}

private:

	BluetoothServiceDiscoveryAgent*			q_ptr;

	BluetoothServiceDiscoveryAgent::Error	error = BluetoothServiceDiscoveryAgent::NoError;
	QString									errorString;
	BluetoothAddress						remoteAddress;
	QList<BluetoothUuid>					uuidFilter;
	QList<BluetoothServiceInfo>				discoveredServices;
	QAtomicInt								stopDiscovery = false;
	QFutureSynchronizer<bool>				serviceDiscoveryFutures;

};

//--------------------------------------------------------------------------------------------------
//	CALLBACK FUNCTIONS
//--------------------------------------------------------------------------------------------------

BOOL __stdcall callback(ULONG uAttribId, LPBYTE pValueStream, ULONG cbStreamSize, LPVOID pvParam)
{
	SDP_ELEMENT_DATA element;
	if (BluetoothSdpGetElementData(pValueStream, cbStreamSize, &element) != ERROR_SUCCESS)
		return FALSE;

	auto device = reinterpret_cast<BluetoothServiceInfo*>(pvParam);

	switch ((SDP_TYPE)element.type)
	{
		// TODO: do something with the SDP attributes
	case SDP_TYPE_UINT:
	case SDP_TYPE_INT:
	case SDP_TYPE_UUID:

	case SDP_TYPE_STRING:

	case SDP_TYPE_URL:
	case SDP_TYPE_SEQUENCE:
	case SDP_TYPE_ALTERNATIVE:
	case SDP_TYPE_BOOLEAN:

	case SDP_TYPE_NIL:
		break;
	default:
		break;
	}

	if (element.type == SDP_TYPE_SEQUENCE)
	{
		HBLUETOOTH_CONTAINER_ELEMENT container = nullptr;
		SDP_ELEMENT_DATA sequence_data;
		while (ERROR_SUCCESS == BluetoothSdpGetContainerElementData(element.data.sequence.value, element.data.sequence.length, &container, &sequence_data))
		{
			qDebug() << "don't compile this away";
		}
	}

	// TODO: do something with the SDP attributes


	return TRUE;
}

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceDiscoveryAgent::BluetoothServiceDiscoveryAgent(QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, d_ptr(new BluetoothServiceDiscoveryAgentPrivate(this))
{
	connect(this, &BluetoothServiceDiscoveryAgent::stop, this, [&]()
	{
		Q_D(BluetoothServiceDiscoveryAgent);
		d->stopDiscovery.store(true);
	});
}

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceDiscoveryAgent::BluetoothServiceDiscoveryAgent(const BluetoothAddress &deviceAdapter, QObject *parent /*= Q_NULLPTR*/)
	: BluetoothServiceDiscoveryAgent(parent)
{
	this->setRemoteAddress(deviceAdapter);
}

//--------------------------------------------------------------------------------------------------
//	~BluetoothServiceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceDiscoveryAgent::~BluetoothServiceDiscoveryAgent()
{
	Q_D(BluetoothServiceDiscoveryAgent);

	d->serviceDiscoveryFutures.waitForFinished();
}

//--------------------------------------------------------------------------------------------------
//	discoveredServices (public ) []
//--------------------------------------------------------------------------------------------------
QList<BluetoothServiceInfo> BluetoothServiceDiscoveryAgent::discoveredServices() const
{
	const Q_D(BluetoothServiceDiscoveryAgent);
	return d->discoveredServices;
}

//--------------------------------------------------------------------------------------------------
//	clear (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceDiscoveryAgent::clear()
{
	if (!isActive())
	{
		Q_D(BluetoothServiceDiscoveryAgent);
		d->discoveredServices.clear();
		d->uuidFilter.clear();
	}
}

//--------------------------------------------------------------------------------------------------
//	start (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceDiscoveryAgent::start(DiscoveryMode mode /*= MinimalDiscovery*/)
{
	Q_D(BluetoothServiceDiscoveryAgent);
	d->stopDiscovery.store(false);

	d->serviceDiscoveryFutures.addFuture(QtConcurrent::run([this, mode]()
	{
		Q_D(BluetoothServiceDiscoveryAgent);

		// Get the windows style address
		SOCKADDR_BTH btAddr;
		btAddr.addressFamily = AF_BTH;
		btAddr.btAddr = d->remoteAddress;

		// Create the query set
		DWORD flags = 0;
		WSAQUERYSET query;
		HANDLE lookupHandle;
		ZeroMemory(&query, sizeof(query));

		// get the WSA address string required for the query
		const DWORD STRLEN = 256;
		char contextStr[256];
		ZeroMemory(&contextStr, STRLEN);
		LPSTR pContextStr = &contextStr[0];

		if (SOCKET_ERROR == WSAAddressToString((SOCKADDR*)&btAddr, sizeof(SOCKADDR_BTH), nullptr, pContextStr, (LPDWORD)&STRLEN))
			d->setError(InvalidBluetoothAdapterError, BluetoothException(ERR).what());

		// service class UUID
		GUID uuid = BluetoothUuid(Protocol::RFCOMM);

		// check this link out for further info: https://msdn.microsoft.com/en-us/library/windows/desktop/aa362914(v=vs.85).aspx
		query.dwSize = sizeof(WSAQUERYSET);
		query.dwNameSpace = NS_BTH;
		query.dwNumberOfCsAddrs = 0;
		query.lpszContext = (LPSTR)&contextStr[0];
		query.lpServiceClassId = &uuid;

		// set the service flags
		if (mode == FullDiscovery)
			flags |= LUP_FLUSHCACHE;	// ignore system cache and do a new query
		flags |= LUP_RETURN_ALL;

		if (!d->stopDiscovery)
		{
			// start the service lookup
			if (SOCKET_ERROR == WSALookupServiceBegin(&query, flags, &lookupHandle))
			{
				switch (WSAGetLastError())
				{
				case WSASERVICE_NOT_FOUND:
					// usually means the device couldn't be connected to or doesn't advertise any services
					return true;
				default:
					d->setError(InvalidBluetoothAdapterError, BluetoothException(ERR).what());
				}
			}
		}

		if (!d->stopDiscovery)
		{
			// find all the services
			const DWORD BUFFSIZE = 2048;
			char resultsBuff[BUFFSIZE];
			ZeroMemory(&resultsBuff, BUFFSIZE);

			LPWSAQUERYSET results = reinterpret_cast<WSAQUERYSET*>(&resultsBuff[0]);
			results->dwSize = sizeof(WSAQUERYSET);
			int ret = SOCKET_ERROR;

			do
			{
				if (d->stopDiscovery)
					break;

				ret = WSALookupServiceNext(lookupHandle, flags, (LPDWORD)&BUFFSIZE, results);
				auto CSAddr = reinterpret_cast<CSADDR_INFO*>(results->lpcsaBuffer);
				auto blob = reinterpret_cast<BLOB*>(results->lpBlob);
				if (blob)
				{
					BluetoothServiceInfo info;
					if (!BluetoothSdpEnumAttributes(blob->pBlobData, blob->cbSize, callback, (LPVOID)&info))
						d->setError(UnknownError, BluetoothException(ERR).what());
					else
					{
						d->discoveredServices.append(info);
						this->serviceDiscovered(info);
					}
				}
			} while (ret != SOCKET_ERROR);

			switch (WSAGetLastError())
			{
			case WSA_E_NO_MORE:
				// expected way for the loop to end
				break;
			default:
				d->setError(UnknownError, BluetoothException(ERR).what());
				break;
			}
		}

		WSALookupServiceEnd(lookupHandle);

		// check for stop
		bool retVal = true;

		if (d->stopDiscovery)
		{
			this->canceled();
			retVal = false;
		}

		this->finished();
		return retVal;
	}));
}

//--------------------------------------------------------------------------------------------------
//	error (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceDiscoveryAgent::Error BluetoothServiceDiscoveryAgent::error() const
{
	const Q_D(BluetoothServiceDiscoveryAgent);
	return d->error;
}

//--------------------------------------------------------------------------------------------------
//	errorString (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothServiceDiscoveryAgent::errorString() const
{
	const Q_D(BluetoothServiceDiscoveryAgent);
	return d->errorString;
}

//--------------------------------------------------------------------------------------------------
//	isActive (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceDiscoveryAgent::isActive() const
{
	const Q_D(BluetoothServiceDiscoveryAgent);
	bool isActive = false;
	for (const auto& future : d->serviceDiscoveryFutures.futures())
		isActive |= future.isRunning();
	return isActive;
}

//--------------------------------------------------------------------------------------------------
//	remoteAddress (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothServiceDiscoveryAgent::remoteAddress() const
{
	const Q_D(BluetoothServiceDiscoveryAgent);
	return d->remoteAddress;
}

//--------------------------------------------------------------------------------------------------
//	setRemoteAddress (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceDiscoveryAgent::setRemoteAddress(const BluetoothAddress &address)
{
	Q_D(BluetoothServiceDiscoveryAgent);
	if (!isActive()) 
	{
		d->remoteAddress = address;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------
//	setUuidFilter (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceDiscoveryAgent::setUuidFilter(const QList<BluetoothUuid> &uuids)
{
	Q_D(BluetoothServiceDiscoveryAgent);
	d->uuidFilter = uuids;
}

//--------------------------------------------------------------------------------------------------
//	setUuidFilter (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceDiscoveryAgent::setUuidFilter(const BluetoothUuid &uuid)
{
	Q_D(BluetoothServiceDiscoveryAgent);
	d->uuidFilter = QList<BluetoothUuid>{ uuid };
}

//--------------------------------------------------------------------------------------------------
//	uuidFilter (public ) []
//--------------------------------------------------------------------------------------------------
QList<BluetoothUuid> BluetoothServiceDiscoveryAgent::uuidFilter() const
{
	const Q_D(BluetoothServiceDiscoveryAgent);
	return d->uuidFilter;
}

