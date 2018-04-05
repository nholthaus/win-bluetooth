#include <bluetoothServiceDiscoveryAgent.h>
#include <bluetoothAddress.h>
#include <bluetoothException.h>

#include <QAtomicInt>
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

/// extract the actual data type from the union
/// https://msdn.microsoft.com/en-us/library/windows/desktop/aa363054(v=vs.85).aspx
QVariant elementData(const SDP_ELEMENT_DATA& element)
{
	switch ((SDP_TYPE)element.type)
	{
		// TODO: do something with the SDP attributes
	case SDP_TYPE_UINT:
		switch ((SDP_SPECIFICTYPE)element.specificType)
		{
		case SDP_ST_UINT128:
			Q_ASSERT(false);
			break;	// support this someday I guess
		case SDP_ST_UINT64:
			return (quint64)element.data.uint64;
			break;
		case SDP_ST_UINT32:
			return (quint32)element.data.uint32;
			break;
		case SDP_ST_UINT16:
			return (quint16)element.data.uint16;
			break;
		case SDP_ST_UINT8:
			return (quint8)element.data.uint8;
			break;
		default:
			Q_ASSERT(false); // this shouldn't ever happen
			break;
		}
		break;
	case SDP_TYPE_INT:
		switch ((SDP_SPECIFICTYPE)element.specificType)
		{
		case SDP_ST_INT128:
			Q_ASSERT(false);
			break;	// support this someday I guess
		case SDP_ST_INT64:
			return (qint64)element.data.int64;
			break;
		case SDP_ST_INT32:
			return (qint32)element.data.int32;
			break;
		case SDP_ST_INT16:
			return (qint16)element.data.int16;
			break;
		case SDP_ST_INT8:
			return (qint8)element.data.int8;
			break;
		default:
			Q_ASSERT(false); // this shouldn't ever happen
			break;
		}
		break;
	case SDP_TYPE_UUID:
		switch ((SDP_SPECIFICTYPE)element.specificType)
		{
		case SDP_ST_UUID128:
			return QVariant(BluetoothUuid(element.data.uuid128));
			break;
		case SDP_ST_UUID32:
			return (quint32)element.data.uuid32;
			break;
		case SDP_ST_UUID16:
			return (quint16)element.data.uuid16;
			break;
		default:
			break;
		}
		break;
	case SDP_TYPE_BOOLEAN:
		return (bool)element.data.booleanVal;
		break;
	case SDP_TYPE_STRING:
		return QString::fromLatin1((char*)element.data.string.value, element.data.string.length);
		break;
	case SDP_TYPE_URL:
		return QString::fromLatin1((char*)element.data.url.value, element.data.url.length);
		break;
	// these need special handling
	case SDP_TYPE_SEQUENCE:		[[fallthrough]];
	case SDP_TYPE_ALTERNATIVE:	[[fallthrough]];
	case SDP_TYPE_NIL:			[[fallthrough]];
	default:
		break;
	}

	return QVariant();
}

/// callback for enumerating SDP attributes
BOOL __stdcall callback(ULONG uAttribId, LPBYTE pValueStream, ULONG cbStreamSize, LPVOID pvParam)
{
	SDP_ELEMENT_DATA element;
	if (BluetoothSdpGetElementData(pValueStream, cbStreamSize, &element) != ERROR_SUCCESS)
		return FALSE;

	auto serviceInfo = reinterpret_cast<BluetoothServiceInfo*>(pvParam);
	
	// handle simple SDP types
	auto data = elementData(element);
	if (data.isValid())
	{
		serviceInfo->setAttribute(uAttribId, data);
		return TRUE;
	}

	// Handle SDP sequences. They have to be unpacked recursively
	if (element.type == SDP_TYPE_SEQUENCE)
	{
		BluetoothServiceInfo::Sequence sequence;

		// recursive lambda :S
		std::function<void(BluetoothServiceInfo::Sequence&, const SDP_ELEMENT_DATA&)> unpackSequence = 
			[&unpackSequence](BluetoothServiceInfo::Sequence& seq, const SDP_ELEMENT_DATA& element)
		{
			SDP_ELEMENT_DATA sequence_data;
			HBLUETOOTH_CONTAINER_ELEMENT container = nullptr;
			while (ERROR_SUCCESS == BluetoothSdpGetContainerElementData(element.data.sequence.value, element.data.sequence.length, &container, &sequence_data))
			{
				if (sequence_data.type == SDP_TYPE_SEQUENCE)
				{
					BluetoothServiceInfo::Sequence sequence;
					seq.append(sequence);
					unpackSequence(sequence, sequence_data);
				}
				else
					seq.append(elementData(sequence_data));
			}
		};
		
		unpackSequence(sequence,element);
		serviceInfo->setAttribute(uAttribId, sequence);
		return TRUE;
	}

	// Handle SDP sequences. They have to be unpacked recursively
	// NOTE: none of my test devices enumerated any attributes, so I'm just treating them like
	// sequences because they kind of look like sequences. This may not be totally correct - NMH 4/5/18.
	if (element.type == SDP_TYPE_ALTERNATIVE)
	{
		BluetoothServiceInfo::Alternative alternative;

		// recursive lambda :S
		std::function<void(BluetoothServiceInfo::Alternative&, const SDP_ELEMENT_DATA&)> unpackAlternative =
			[&unpackAlternative](BluetoothServiceInfo::Alternative& alt, const SDP_ELEMENT_DATA& element)
		{
			SDP_ELEMENT_DATA alternative_data;
			HBLUETOOTH_CONTAINER_ELEMENT container = nullptr;
			while (ERROR_SUCCESS == BluetoothSdpGetContainerElementData(element.data.alternative.value, element.data.alternative.length, &container, &alternative_data))
			{
				if (alternative_data.type == SDP_TYPE_ALTERNATIVE)
				{
					BluetoothServiceInfo::Alternative alternative;
					alt.append(alternative);
					unpackAlternative(alternative, alternative_data);
				}
				else
					alt.append(elementData(alternative_data));
			}
		};

		unpackAlternative(alternative, element);
		serviceInfo->setAttribute(uAttribId, alternative);
		return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceDiscoveryAgent::BluetoothServiceDiscoveryAgent(QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, d_ptr(new BluetoothServiceDiscoveryAgentPrivate(this))
{

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

		bool serviceNotFound = false;

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
					serviceNotFound = true;
				default:
					d->setError(InvalidBluetoothAdapterError, BluetoothException(ERR).what());
				}
			}
		}

		if (!d->stopDiscovery || serviceNotFound)
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
			case ERROR_SUCCESS:
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
			emit this->canceled();
			retVal = false;
		}

		if (serviceNotFound)
			retVal = false;

		emit this->finished();
		return retVal;
	}));
}

//--------------------------------------------------------------------------------------------------
//	stop (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceDiscoveryAgent::stop()
{
	Q_D(BluetoothServiceDiscoveryAgent);
	d->stopDiscovery.store(true);
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

