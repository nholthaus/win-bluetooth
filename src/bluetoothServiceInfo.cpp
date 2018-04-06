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
	if (d->m_registered)
		return false;

	if (!adjustProcessPrivileges())
		return false;

	// find the local radio to register with
	BluetoothRadio* radio = &Bluetooth::localRadio(true);
	for (auto& [localRadioName, localRadio] : Bluetooth::localRadios())
	{
		if (localRadio == static_cast<size_t>(localAdapter))
			radio = &localRadio;
	}

	GUID guid = serviceUuid();

	BLUETOOTH_LOCAL_SERVICE_INFO info;
	ZeroMemory(&info.szName, sizeof(info.szName));
	ZeroMemory(&info.szDeviceString, sizeof(info.szDeviceString));

	info.Enabled = TRUE;
	info.btAddr.ullLong = radio->address();
	serviceName().toWCharArray(info.szName);
	serviceDescription().toWCharArray(info.szDeviceString);

	if (ERROR_SUCCESS != BluetoothSetLocalServiceInfo(radio->handle(), &guid, 1, &info))
		throw BluetoothException(ERR);
	else
	{
		d->m_registered = true;
		return true;
	}
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
	d->m_registered = false;
	return true;
}

//--------------------------------------------------------------------------------------------------
//	adjustProcessPriviliges (private ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::adjustProcessPrivileges()
{
	HANDLE procToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;
	BOOL bRetVal;
	DWORD err;

	bRetVal = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &procToken);
	if (!bRetVal)
	{
		err = GetLastError();
		printf("OpenProcessToken failed, err = %d\n", err);
		return bRetVal;
	}

	bRetVal = LookupPrivilegeValue(nullptr, SE_LOAD_DRIVER_NAME, &luid);
	if (!bRetVal)
	{
		err = GetLastError();
		printf("LookupPrivilegeValue failed, err = %d\n", err);
		CloseHandle(procToken);
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//
	// AdjustTokenPrivileges can succeed even when privileges are not adjusted.
	// In such case GetLastError returns ERROR_NOT_ALL_ASSIGNED.
	//
	// Hence we check for GetLastError in both success and failure case.
	//

	(void)AdjustTokenPrivileges(procToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)nullptr, (PDWORD)nullptr);
	err = GetLastError();

	if (err != ERROR_SUCCESS)
	{
		bRetVal = false;
		printf("AdjustTokenPrivileges failed, err = %d\n", err);
		CloseHandle(procToken);
	}

	return bRetVal;
}