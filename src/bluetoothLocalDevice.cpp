#include <bluetoothLocalDevice.h>
#include <bluetoothAddress.h>
#include <bluetoothRadio.h>
#include <bluetoothHostInfo.h>
#include <bluetooth.h>

#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

//--------------------------------------------------------------------------------------------------
//	BluetoothLocalDevicePrivate
//--------------------------------------------------------------------------------------------------

class BluetoothLocalDevicePrivate
{
public:

	BluetoothLocalDevicePrivate(BluetoothLocalDevice* q = nullptr)
		: q_ptr(q)
		, radio(Bluetooth::localRadio(true))
		, address(radio.address())
	{
		connectTheWatcher();
	}

		BluetoothLocalDevicePrivate(const BluetoothAddress& address, BluetoothLocalDevice* q = nullptr)
		: q_ptr(q)
		, radio(Bluetooth::localRadio(Bluetooth::name(address)))
		, address(address)
	{
		connectTheWatcher();
	}

	virtual ~BluetoothLocalDevicePrivate()
	{}
	
	void connectTheWatcher()
	{
		QObject::connect(&this->theWatcher, &QFutureWatcher<bool>::finished, [this]()
		{
			Q_Q(BluetoothLocalDevice);
			bool thisWorked = theWatcher.future().result();
			if (thisWorked)
				emit q->pairingFinished(this->address, BluetoothLocalDevice::AuthorizedPaired);
			else
				emit q->pairingFinished(this->address, BluetoothLocalDevice::Unpaired);
		});
	}
	
protected:

	Q_DECLARE_PUBLIC(BluetoothLocalDevice)
	BluetoothLocalDevice* q_ptr;
	BluetoothRadio& radio;
	BluetoothAddress address;
	QHash<unsigned long long, QFuture<bool>> paired;
	QFutureWatcher<bool> theWatcher;	// https://www.youtube.com/watch?v=jgBpC8sbBtU
};

//--------------------------------------------------------------------------------------------------
//	BluetoothLocalDevice () []
//--------------------------------------------------------------------------------------------------
BluetoothLocalDevice::BluetoothLocalDevice(QObject* parent /*= nullptr*/) 
	: QObject(parent)
	, d_ptr(new BluetoothLocalDevicePrivate(this))
{

}

//--------------------------------------------------------------------------------------------------
//	BluetoothLocalDevice () []
//--------------------------------------------------------------------------------------------------
BluetoothLocalDevice::BluetoothLocalDevice(const BluetoothAddress& address, QObject* parent /*= nullptr*/)
	: QObject(parent)
	, d_ptr(new BluetoothLocalDevicePrivate(address, this))
{

}

//--------------------------------------------------------------------------------------------------
//	~BluetoothLocalDevice () []
//--------------------------------------------------------------------------------------------------
BluetoothLocalDevice::~BluetoothLocalDevice()
{
	delete d_ptr;
}

//--------------------------------------------------------------------------------------------------
//	address () []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothLocalDevice::address() const
{
	Q_D(const BluetoothLocalDevice);
	return d->address;
}

//--------------------------------------------------------------------------------------------------
//	connectedDevices () []
//--------------------------------------------------------------------------------------------------
QList<BluetoothAddress> BluetoothLocalDevice::connectedDevices() const
{
	Q_D(const BluetoothLocalDevice);

	QList<BluetoothAddress> connectedAddresses;
	
	for (const auto& [name, device] : Bluetooth::remoteDevices())
	{
		// safe to const cast here, only non-const because of a cache refresh
		if (const_cast<BluetoothDevice*>(&device)->connected())
			connectedAddresses << device.address();
	}

	return connectedAddresses;
}

//--------------------------------------------------------------------------------------------------
//	hostMode () []
//--------------------------------------------------------------------------------------------------
BluetoothLocalDevice::HostMode BluetoothLocalDevice::hostMode() const
{
	Q_D(const BluetoothLocalDevice);
	if (d->radio.discoverable())
		return HostDiscoverable;
	else if (d->radio.connectable())
		return HostConnectable;
	else
		return HostPoweredOff;
}

//--------------------------------------------------------------------------------------------------
//	isValid () []
//--------------------------------------------------------------------------------------------------
bool BluetoothLocalDevice::isValid() const
{
	Q_D(const BluetoothLocalDevice);
	return d->radio.isValid();
}

//--------------------------------------------------------------------------------------------------
//	name () []
//--------------------------------------------------------------------------------------------------
QString BluetoothLocalDevice::name() const
{
	Q_D(const BluetoothLocalDevice);
	return d->radio.name();
}

//--------------------------------------------------------------------------------------------------
//	pairingStatus () []
//--------------------------------------------------------------------------------------------------
BluetoothLocalDevice::Pairing BluetoothLocalDevice::pairingStatus(const BluetoothAddress& address) const
{
	Q_ASSERT("this is unimplemented");
	return Unpaired;
}

//--------------------------------------------------------------------------------------------------
//	powerOn () []
//--------------------------------------------------------------------------------------------------
void BluetoothLocalDevice::powerOn()
{
	Q_ASSERT(false);	// Windows doesn't expose this feature. Don't call this function.
}

//--------------------------------------------------------------------------------------------------
//	requestPairing () []
//--------------------------------------------------------------------------------------------------
void BluetoothLocalDevice::requestPairing(const BluetoothAddress &address, Pairing pairing)
{
	Q_D(BluetoothLocalDevice);
	d->paired[address] = QtConcurrent::run(&d->radio, &BluetoothRadio::connectTo, address);
	d->theWatcher.setFuture(d->paired[address]);
}

//--------------------------------------------------------------------------------------------------
//	setHostMode () []
//--------------------------------------------------------------------------------------------------
void BluetoothLocalDevice::setHostMode(HostMode mode)
{
	Q_D(BluetoothLocalDevice);
	switch (mode)
	{
	case BluetoothLocalDevice::HostPoweredOff:
		return;	// can't power the thing off from the windows API
		break;
	case BluetoothLocalDevice::HostConnectable:
		d->radio.setConnectable(true);
		break;
	case BluetoothLocalDevice::HostDiscoverable:
		d->radio.setDiscoverable(true);
		break;
	case BluetoothLocalDevice::HostDiscoverableLimitedInquiry:
		d->radio.setDiscoverable(true);
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------------
//	allDevices (public ) [static ]
//--------------------------------------------------------------------------------------------------
QList<BluetoothHostInfo> BluetoothLocalDevice::allDevices()
{
	QList<BluetoothHostInfo> infoList;
	BluetoothHostInfo info;

	for (const auto& [name, device] : Bluetooth::remoteDevices(true))
	{	
		info.setName(name);
		info.setAddress(device.address());
		infoList << info;
	}

	return infoList;
}

//--------------------------------------------------------------------------------------------------
//	pairingConfirmation () []
//--------------------------------------------------------------------------------------------------
void BluetoothLocalDevice::pairingConfirmation(bool confirmation)
{

}

