#include <bluetoothServiceinfo.h>
#include <bluetoothDeviceInfo.h>
#include <bluetoothUuids.h>
#include <Bluetooth.h>
#include <bluetoothException.h>
#include <QSharedData>
#include <QDebug>

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
//	CONSTANTS
//--------------------------------------------------------------------------------------------------

constexpr unsigned int SERVICE_NAME_OFFSET			= 0x0000;
constexpr unsigned int SERVICE_DESCRIPTION_OFFSET	= 0x0001;
constexpr unsigned int SERVICE_PROVIDER_OFFSET		= 0x0002;

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfoPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothServiceInfoPrivate : public QSharedData
{
public:

	Q_DECLARE_PUBLIC(BluetoothServiceInfo)

	BluetoothServiceInfoPrivate(BluetoothServiceInfo* parent) : q_ptr(parent) {};
	virtual ~BluetoothServiceInfoPrivate() {};
	
	
private:

	BluetoothServiceInfo*		q_ptr;
	QMap<quint16, QVariant>		m_attributes;
	BluetoothDeviceInfo			m_device;
	bool						m_registered = false;
};

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::BluetoothServiceInfo()
	: d(new BluetoothServiceInfoPrivate(this))
{

}

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::BluetoothServiceInfo(const BluetoothServiceInfo& other)
	: d(other.d)
{

}

//--------------------------------------------------------------------------------------------------
//	~BluetoothServiceInfo (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::~BluetoothServiceInfo()
{

}

//--------------------------------------------------------------------------------------------------
//	attribute () []
//--------------------------------------------------------------------------------------------------
QVariant BluetoothServiceInfo::attribute(quint16 attributeId) const
{
	if (d->m_attributes.contains(attributeId))
	{
		return *d->m_attributes.constFind(attributeId);
	}
	else
		return QVariant();
}

//--------------------------------------------------------------------------------------------------
//	attributes () []
//--------------------------------------------------------------------------------------------------
QList<quint16> BluetoothServiceInfo::attributes() const
{
	return d->m_attributes.keys();
}

//--------------------------------------------------------------------------------------------------
//	contains () []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::contains(quint16 attributeId) const
{
	return d->m_attributes.contains(attributeId);
}

//--------------------------------------------------------------------------------------------------
//	device (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo BluetoothServiceInfo::device() const
{
	return d->m_device;
}

//--------------------------------------------------------------------------------------------------
//	isComplete (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::isComplete() const
{
	return d->m_attributes.keys().contains(ProtocolDescriptorList);
}

//--------------------------------------------------------------------------------------------------
//	isRegistered (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::isRegistered() const
{
	return d->m_registered;
}

//--------------------------------------------------------------------------------------------------
//	isValid (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::isValid() const
{
	return !d->m_attributes.isEmpty();
}

//--------------------------------------------------------------------------------------------------
//	protocolDescriptor (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::Sequence BluetoothServiceInfo::protocolDescriptor(BluetoothUuid protocol) const
{
	Sequence retVal;

	if (d->m_attributes.contains(ProtocolDescriptorList))
	{
		Sequence protocolDescriptorList = d->m_attributes[ProtocolDescriptorList].toList();

		for (const auto& protocolDescriptorVar : protocolDescriptorList)
		{
			Sequence protocolDescriptor = protocolDescriptorVar.toList();
			if (!protocolDescriptor.isEmpty())
			{
				if (protocolDescriptor.first().canConvert<QUuid>())
				{
					BluetoothUuid uuid = protocolDescriptor.first().value<QUuid>();
					if (uuid == protocol)
					{
						protocolDescriptor.removeFirst();
						return protocolDescriptor;
					}
				}
			}
		}
	}
	
	return retVal;
}

//--------------------------------------------------------------------------------------------------
//	protocolServiceMultiplexer (public ) []
//--------------------------------------------------------------------------------------------------
int BluetoothServiceInfo::protocolServiceMultiplexer() const
{
	auto pd = protocolDescriptor(BluetoothUuid(::Protocol::L2CAP));
	if (!pd.isEmpty())
		return pd.first().toInt();
	else
		return -1;
}

//--------------------------------------------------------------------------------------------------
//	registerService (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::registerService(const BluetoothAddress &localAdapter /*= BluetoothAddress()*/)
{
	return registerService(localAdapter, false);
}

//--------------------------------------------------------------------------------------------------
//	registerService (private ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::registerService(const BluetoothAddress & localAdapter, bool unregister /*= false*/)
{
	bool ret = true;

	// Register the service with WSASetService
	WSAQUERYSET		wsaQuerySet = { 0 };
	SOCKADDR_BTH	SockAddrBthLocal = { 0 };
	LPCSADDR_INFO	lpCSAddrInfo = new CSADDR_INFO;

	// setup the address info
	SockAddrBthLocal.addressFamily = AF_BTH;
	SockAddrBthLocal.port = BT_PORT_ANY;
	SockAddrBthLocal.btAddr = localAdapter;

	lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
	lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
	lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
	lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;

	// get variables from the class and convert them to MS types
	GUID serviceClass = this->serviceUuid();
	LPSTR serviceName = new char[this->serviceName().size() + 1];
	if (!this->serviceName().isEmpty())
		strcpy_s(serviceName, this->serviceName().size() + 1, this->serviceName().toLatin1().constData());
	serviceName[this->serviceName().size()] = '\0';

	LPSTR serviceDescription = new char[this->serviceDescription().size() + 1];
	if (!this->serviceDescription().isEmpty())
		strcpy_s(serviceDescription, this->serviceDescription().size() + 1, this->serviceDescription().toLatin1().constData());
	serviceDescription[this->serviceDescription().size()] = '\0';

	// Setup the query set
	ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
	wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
	wsaQuerySet.lpServiceClassId = (LPGUID)&serviceClass;
	wsaQuerySet.lpszServiceInstanceName = serviceName;
	wsaQuerySet.lpszComment = serviceDescription;
	wsaQuerySet.dwNameSpace = NS_BTH;
	wsaQuerySet.dwNumberOfCsAddrs = 1;			// Must be 1.
	wsaQuerySet.lpcsaBuffer = lpCSAddrInfo; // Req'd.

	WSAESETSERVICEOP op = unregister ? RNRSERVICE_DELETE : RNRSERVICE_REGISTER;
	if (SOCKET_ERROR == WSASetService(&wsaQuerySet, op, 0))
		ret = false;
	else
	{
		// it worked, set the device
		d->m_registered = !unregister;
		BluetoothDeviceInfo device()
	}


	delete lpCSAddrInfo;
	return ret;
}

//--------------------------------------------------------------------------------------------------
//	removeAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::removeAttribute(quint16 attributeId)
{
	d->m_attributes.remove(attributeId);
}

//--------------------------------------------------------------------------------------------------
//	serverChannel (public ) []
//--------------------------------------------------------------------------------------------------
int BluetoothServiceInfo::serverChannel() const
{
	auto pd = protocolDescriptor(BluetoothUuid(::Protocol::RFCOMM));
	if (!pd.isEmpty())
		return pd.first().toInt();
	else
		return -1;
}

//--------------------------------------------------------------------------------------------------
//	serviceAvailability (public ) []
//--------------------------------------------------------------------------------------------------
quint8 BluetoothServiceInfo::serviceAvailability() const
{
	if (d->m_attributes.contains(ServiceAvailability))
		return d->m_attributes[ServiceAvailability].toUInt();
	else
		return 1;
}

//--------------------------------------------------------------------------------------------------
//	serviceClassUuids (public ) []
//--------------------------------------------------------------------------------------------------
QList<BluetoothUuid> BluetoothServiceInfo::serviceClassUuids() const
{
	QList<BluetoothUuid> ids;
	QList<QVariant> idsVariant = d->m_attributes[ServiceClassIds].toList();
	for (const auto& id : idsVariant)
		ids << BluetoothUuid(id.toUuid());

	return ids;
}

//--------------------------------------------------------------------------------------------------
//	serviceDescription (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothServiceInfo::serviceDescription() const
{
	if (d->m_attributes.contains(PrimaryLanguageBase + SERVICE_DESCRIPTION_OFFSET))
		return d->m_attributes[PrimaryLanguageBase + SERVICE_DESCRIPTION_OFFSET].toString();
	else
		return "";
}

//--------------------------------------------------------------------------------------------------
//	serviceName (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothServiceInfo::serviceName() const
{
	if (d->m_attributes.contains(PrimaryLanguageBase + SERVICE_NAME_OFFSET))
		return d->m_attributes[PrimaryLanguageBase + SERVICE_NAME_OFFSET].toString();
	else
		return "";
}

//--------------------------------------------------------------------------------------------------
//	serviceProvider (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothServiceInfo::serviceProvider() const
{
	if (d->m_attributes.contains(PrimaryLanguageBase + SERVICE_PROVIDER_OFFSET))
		return d->m_attributes[PrimaryLanguageBase + SERVICE_PROVIDER_OFFSET].toString();
	else
		return "";
}

//--------------------------------------------------------------------------------------------------
//	serviceUuid (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothUuid BluetoothServiceInfo::serviceUuid() const
{
	if (d->m_attributes.contains(ServiceId))
		return d->m_attributes[ServiceId].value<QUuid>();
	else
		return BluetoothUuid();
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const QVariant& value)
{
	d->m_attributes.insert(attributeId, value);
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const Sequence& value)
{
	d->m_attributes.insert(attributeId, value);
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const Alternative& value)
{
	d->m_attributes.insert(attributeId, value);
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const BluetoothUuid& value)
{
	d->m_attributes.insert(attributeId, QVariant(value));
}

//--------------------------------------------------------------------------------------------------
//	setDevice (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setDevice(const BluetoothDeviceInfo& device)
{
	d->m_device = device;
}

//--------------------------------------------------------------------------------------------------
//	setServiceAvailability (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setServiceAvailability(quint8 availability)
{
	d->m_attributes[ServiceAvailability] = availability;
}

//--------------------------------------------------------------------------------------------------
//	setServiceDescription (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setServiceDescription(const QString& description)
{
	d->m_attributes[PrimaryLanguageBase + SERVICE_DESCRIPTION_OFFSET] = description;
}

//--------------------------------------------------------------------------------------------------
//	setServiceName (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setServiceName(const QString &name)
{
	d->m_attributes[PrimaryLanguageBase + SERVICE_NAME_OFFSET] = name;
}

//--------------------------------------------------------------------------------------------------
//	setServiceProvider (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setServiceProvider(const QString &provider)
{
	d->m_attributes[PrimaryLanguageBase + SERVICE_PROVIDER_OFFSET] = provider;
}

//--------------------------------------------------------------------------------------------------
//	setServiceUuid (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setServiceUuid(const BluetoothUuid &uuid)
{
	d->m_attributes[ServiceId] = QVariant(uuid);
}

//--------------------------------------------------------------------------------------------------
//	socketProtocol (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::Protocol BluetoothServiceInfo::socketProtocol() const
{
	return RfcommProtocol;
}

//--------------------------------------------------------------------------------------------------
//	unregisterService (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::unregisterService()
{
	return registerService(localAdapter, false);
	d->m_registered = false;









	return false;
}