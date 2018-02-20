#include <bluetoothDeviceInfo.h>
#include <bluetoothAddress.h>
#include <bluetoothUuids.h>


//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceInfoPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothDeviceInfoPrivate
{
	Q_DECLARE_PUBLIC(BluetoothDeviceInfo);

public:

	BluetoothDeviceInfoPrivate(BluetoothDeviceInfo* parent = nullptr) { q_ptr = parent; }
	BluetoothDeviceInfoPrivate(const BluetoothDeviceInfoPrivate& other) = default;
public:

	BluetoothDeviceInfo*					q_ptr;

	BluetoothDeviceInfo::CoreConfigurations	coreConfig = BluetoothDeviceInfo::UnknownCoreConfiguration;
	BluetoothAddress						address;
	BluetoothUuid							uuid;
	QString									name;
	quint32									classOfDevice = 0;
	bool									isCached = false;
	bool									isValid = false;
	quint16									rssi = 0;
	QList<BluetoothUuid>					sericeUuids;
	BluetoothDeviceInfo::DataCompleteness	serviceUuidCompleteness = BluetoothDeviceInfo::DataIncomplete;
};

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::BluetoothDeviceInfo()
	: d_ptr(new BluetoothDeviceInfoPrivate(this))
{

}

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::BluetoothDeviceInfo(const BluetoothAddress& address, const QString& name, quint32 classOfDevice)
	: BluetoothDeviceInfo()
{
	Q_D(BluetoothDeviceInfo);
	d->address = address;
	d->name = name;
	d->classOfDevice = classOfDevice;
}

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::BluetoothDeviceInfo(const BluetoothUuid& uuid, const QString& name, quint32 classOfDevice)
	: BluetoothDeviceInfo()
{
	Q_D(BluetoothDeviceInfo);
	d->uuid = uuid;
	d->name = name;
	d->classOfDevice = classOfDevice;
}

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::BluetoothDeviceInfo(const BluetoothDeviceInfo& other)
	: BluetoothDeviceInfo()
{
	Q_D(BluetoothDeviceInfo);
	*d_ptr = *other.d_ptr;
	d_ptr->q_ptr = this;
}

//--------------------------------------------------------------------------------------------------
//	operator= (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo& BluetoothDeviceInfo::operator=(const BluetoothDeviceInfo& other)
{
	Q_D(BluetoothDeviceInfo);
	*d_ptr = *other.d_ptr;
	d_ptr->q_ptr = this;
	return *this;
}

//--------------------------------------------------------------------------------------------------
//	~BluetoothDeviceInfo (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::~BluetoothDeviceInfo()
{
	delete d_ptr;
	d_ptr = nullptr;
}

//--------------------------------------------------------------------------------------------------
//	address (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothDeviceInfo::address() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->address;
}

//--------------------------------------------------------------------------------------------------
//	CoreConfigurations (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::CoreConfigurations BluetoothDeviceInfo::coreConfigurations() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->coreConfig;
}

//--------------------------------------------------------------------------------------------------
//	deviceUuid (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothUuid BluetoothDeviceInfo::deviceUuid() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->uuid;
}

//--------------------------------------------------------------------------------------------------
//	isCached (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothDeviceInfo::isCached() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->isCached;
}

//--------------------------------------------------------------------------------------------------
//	isValid (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothDeviceInfo::isValid() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->isValid;
}

//--------------------------------------------------------------------------------------------------
//	majorDeviceClass (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::MajorDeviceClass BluetoothDeviceInfo::majorDeviceClass() const
{
	// bits 8-12 of the device class
	const Q_D(BluetoothDeviceInfo);
	return (MajorDeviceClass)((0x00001F00 & d->classOfDevice) >> 8);
}

//--------------------------------------------------------------------------------------------------
//	minorDeviceClass (public ) []
//--------------------------------------------------------------------------------------------------
quint8 BluetoothDeviceInfo::minorDeviceClass() const
{
	// bits 2-7 of the device class
	const Q_D(BluetoothDeviceInfo);
	return (0x000000FC & d->classOfDevice) >> 2;
}

//--------------------------------------------------------------------------------------------------
//	name (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothDeviceInfo::name() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->name;
}

//--------------------------------------------------------------------------------------------------
//	rssi (public ) []
//--------------------------------------------------------------------------------------------------
qint16 BluetoothDeviceInfo::rssi() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->rssi;
}

//--------------------------------------------------------------------------------------------------
//	serviceClasses (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::ServiceClasses BluetoothDeviceInfo::serviceClasses() const
{
	// bits 13-23 of the device class
	const Q_D(BluetoothDeviceInfo);
	return (0x00FFE000 & d->classOfDevice) >> 13 >> 3;	// Qt's enum is shifted an additional 3 btis
}

//--------------------------------------------------------------------------------------------------
//	serviceUuids (public ) []
//--------------------------------------------------------------------------------------------------
QList<BluetoothUuid> BluetoothDeviceInfo::serviceUuids(DataCompleteness* completeness /*= Q_NULLPTR*/) const
{
	const Q_D(BluetoothDeviceInfo);
	return d->sericeUuids;
}

//--------------------------------------------------------------------------------------------------
//	serviceUuidsCompleteness (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo::DataCompleteness BluetoothDeviceInfo::serviceUuidsCompleteness() const
{
	const Q_D(BluetoothDeviceInfo);
	return d->serviceUuidCompleteness;
}

//--------------------------------------------------------------------------------------------------
//	setCached (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceInfo::setCached(bool cached)
{
	Q_D(BluetoothDeviceInfo);
	d->isCached = cached;
}

//--------------------------------------------------------------------------------------------------
//	setCoreConfigurations (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceInfo::setCoreConfigurations(CoreConfigurations coreConfigs)
{
	Q_D(BluetoothDeviceInfo);
	d->coreConfig = coreConfigs;
}

//--------------------------------------------------------------------------------------------------
//	setDeviceUuid (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceInfo::setDeviceUuid(const BluetoothUuid& uuid)
{
	Q_D(BluetoothDeviceInfo);
	d->uuid = uuid;
}

//--------------------------------------------------------------------------------------------------
//	setRssi (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceInfo::setRssi(qint16 signal)
{
	Q_D(BluetoothDeviceInfo);
	d->rssi = signal;
}

//--------------------------------------------------------------------------------------------------
//	setServiceUuids (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceInfo::setServiceUuids(const QList<BluetoothUuid>& uuids, DataCompleteness completeness)
{
	Q_D(BluetoothDeviceInfo);
	d->sericeUuids = uuids;
	d->serviceUuidCompleteness = completeness;
}

//--------------------------------------------------------------------------------------------------
//	operator== (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothDeviceInfo::operator==(const BluetoothDeviceInfo& other) const
{
	return address() == other.address();
}

//--------------------------------------------------------------------------------------------------
//	operator!= (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothDeviceInfo::operator!=(const BluetoothDeviceInfo& other) const
{
	return !(*this == other);
}
