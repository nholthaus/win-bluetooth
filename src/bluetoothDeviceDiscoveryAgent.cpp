#include "bluetoothDeviceDiscoveryAgent.h"

#include <bluetooth.h>
#include <bluetoothLocalDevice.h>
#include <BluetoothDeviceInfo.h>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceDiscoveryAgentPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothDeviceDiscoveryAgentPrivate
{
public:
	
	BluetoothDeviceDiscoveryAgent::Error error = BluetoothDeviceDiscoveryAgent::NoError;
	QString errorString = "";
	BluetoothDeviceDiscoveryAgent::InquiryType inquiryType = BluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
	BluetoothDeviceDiscoveryAgent::DiscoveryMethod discoveryMethod = BluetoothDeviceDiscoveryAgent::ClassicMethod;
	QFutureWatcher<bool> theWatcher;
};

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceDiscoveryAgent::BluetoothDeviceDiscoveryAgent(QObject *parent /*= nullptr*/)
	: d_ptr(new BluetoothDeviceDiscoveryAgentPrivate)
	, QObject(parent)
{
	QObject::connect(&d_ptr->theWatcher, &QFutureWatcher<bool>::finished, [this]()
	{
		bool thisWorked = d_ptr->theWatcher.future().result();
		if (!thisWorked)
		{
			d_ptr->error = UnknownError;
			d_ptr->errorString = "Bluetooth device discovery failed";
			emit error(UnknownError);
		}
		else
		{
			for (const auto& info : discoveredDevices())
				emit deviceDiscovered(info);
		}
		emit finished();
	});
}

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceDiscoveryAgent::BluetoothDeviceDiscoveryAgent(const BluetoothAddress &deviceAdapter, QObject *parent /*= nullptr*/)
	: BluetoothDeviceDiscoveryAgent(parent)
{

}

//--------------------------------------------------------------------------------------------------
//	~BluetoothDeviceDiscoveryAgent (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceDiscoveryAgent::~BluetoothDeviceDiscoveryAgent()
{
	delete d_ptr;
}

//--------------------------------------------------------------------------------------------------
//	discoveredDevices (public ) []
//--------------------------------------------------------------------------------------------------
QList<BluetoothDeviceInfo> BluetoothDeviceDiscoveryAgent::discoveredDevices() const
{
	QList<BluetoothDeviceInfo> infoList;
	for (const auto& [name, device] : Bluetooth::remoteDevices())
	{
		infoList << BluetoothDeviceInfo{device.address(), name, device.classOfDevice()};
	}

	return infoList;
}

//--------------------------------------------------------------------------------------------------
//	start (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceDiscoveryAgent::start()
{
	QFuture<bool> future = QtConcurrent::run(&Bluetooth::enumerateRemoteDevices , true);
	d_ptr->theWatcher.setFuture(future);
}

//--------------------------------------------------------------------------------------------------
//	start (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceDiscoveryAgent::start(BluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
{
	start();
}

//--------------------------------------------------------------------------------------------------
//	stop (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceDiscoveryAgent::stop()
{
	Q_ASSERT(false);	// can't touch this!
	emit canceled();
}

//--------------------------------------------------------------------------------------------------
//	error (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceDiscoveryAgent::Error BluetoothDeviceDiscoveryAgent::error() const
{
	return d_ptr->error;
}

//--------------------------------------------------------------------------------------------------
//	errorString (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothDeviceDiscoveryAgent::errorString() const
{
	return d_ptr->errorString;
}

//--------------------------------------------------------------------------------------------------
//	inquiryType (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceDiscoveryAgent::InquiryType BluetoothDeviceDiscoveryAgent::inquiryType() const
{
	return d_ptr->inquiryType;
}

//--------------------------------------------------------------------------------------------------
//	isActive (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothDeviceDiscoveryAgent::isActive() const
{
	return d_ptr->theWatcher.future().isRunning();
}

//--------------------------------------------------------------------------------------------------
//	lowEnergyDiscoveryTimeout (public ) []
//--------------------------------------------------------------------------------------------------
int BluetoothDeviceDiscoveryAgent::lowEnergyDiscoveryTimeout() const
{
	return 0;
}

//--------------------------------------------------------------------------------------------------
//	setInquiryType (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceDiscoveryAgent::setInquiryType(BluetoothDeviceDiscoveryAgent::InquiryType type)
{
	d_ptr->inquiryType = type;
	Q_ASSERT(type == ClassicMethod);	// only the classic method is implemented
}

//--------------------------------------------------------------------------------------------------
//	setLowEnergyDiscoveryTimeout (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothDeviceDiscoveryAgent::setLowEnergyDiscoveryTimeout(int timeout)
{
	Q_ASSERT(false);	// unimplemented
}

//--------------------------------------------------------------------------------------------------
//	supportedDiscoveryMethods (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceDiscoveryAgent::DiscoveryMethods BluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
	return DiscoveryMethod::ClassicMethod;
}

