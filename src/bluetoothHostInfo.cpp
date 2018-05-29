#include "bluetoothHostInfo.h"

#include <bluetooth.h>
#include <bluetoothAddress.h>

//--------------------------------------------------------------------------------------------------
//	BluetoothHostInfoPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothHostInfoPrivate
{
public:

	QString name;
	BluetoothAddress address;

};

//--------------------------------------------------------------------------------------------------
//	BluetoothHostInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothHostInfo::BluetoothHostInfo()
	: d_ptr(new BluetoothHostInfoPrivate)
{
	d_ptr->name = Bluetooth::localRadio().name();
	d_ptr->address = Bluetooth::localRadio().address();
}

//--------------------------------------------------------------------------------------------------
//	BluetoothHostInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothHostInfo::BluetoothHostInfo(const BluetoothHostInfo& other)
	: d_ptr(new BluetoothHostInfoPrivate)
{
	d_ptr->name = other.name();
	d_ptr->address = other.address();
}


//--------------------------------------------------------------------------------------------------
//	~BluetoothHostInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothHostInfo::~BluetoothHostInfo()
{
	delete d_ptr;
}

//--------------------------------------------------------------------------------------------------
//	address (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothHostInfo::address() const
{
	return d_ptr->address;
}

//--------------------------------------------------------------------------------------------------
//	name (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothHostInfo::name() const
{
	return d_ptr->name;
}

//--------------------------------------------------------------------------------------------------
//	setAddress (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothHostInfo::setAddress(const BluetoothAddress &address)
{
	d_ptr->address = address;
}

//--------------------------------------------------------------------------------------------------
//	setName (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothHostInfo::setName(const QString &name)
{
	d_ptr->name = name;
}

//--------------------------------------------------------------------------------------------------
//	operator== (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothHostInfo::operator==(const BluetoothHostInfo &other) const
{
	return d_ptr->address == other.address();
}

//--------------------------------------------------------------------------------------------------
//	operator= (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothHostInfo & BluetoothHostInfo::operator=(const BluetoothHostInfo &other)
{
	d_ptr->name = other.name();
	d_ptr->address = other.address();
	return *this;
}

//--------------------------------------------------------------------------------------------------
//	operator!= (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothHostInfo::operator!=(const BluetoothHostInfo &other) const
{
	return !(*this == other);
}

