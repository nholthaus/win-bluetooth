#include <bluetoothAddress.h>
#include <bluetooth.h>

#include <algorithm>
#include <QRegularExpression>

BluetoothAddress::BluetoothAddress(uint64_t address)
	: m_address(address)
{

}

BluetoothAddress::BluetoothAddress(const QString& nameOrMac)
{
	addressFromString(nameOrMac);
}

//--------------------------------------------------------------------------------------------------
//	BluetoothAddress (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress::BluetoothAddress(const char* nameOrMac)
{
	addressFromString(nameOrMac);
}

//--------------------------------------------------------------------------------------------------
//	addressFromString (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothAddress::addressFromString(const QString& nameOrMac)
{
	QRegularExpression macRx(R"(^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$)");
	if (nameOrMac.contains(macRx))
	{
		// it's a MAC address
		QString str = nameOrMac;
		str.remove(':');
		m_address = str.toULongLong(nullptr, 16);
	}
	else if (const auto& radio = Bluetooth::localRadio(nameOrMac); radio.isValid())
	{
		// it's a host name
		m_address = radio.address();
	}
	else if (const auto& device = Bluetooth::remoteDevice(nameOrMac); device.isValid())
	{
		// it's a host name
		m_address = device.address();
	}
}

void BluetoothAddress::clear()
{
	m_address = 0;
}

bool BluetoothAddress::isNull() const
{
	return !m_address;
}

QString BluetoothAddress::toString() const
{
	return *this;
}

quint64 BluetoothAddress::toUInt64() const
{
	return *this;
}

BluetoothAddress::operator uint64_t() const
{
	return m_address;
}

bool BluetoothAddress::operator==(uint64_t other) const
{
	return m_address == other;
}

bool BluetoothAddress::operator==(const QString& other) const
{
	return QString(*this) == other.toUpper();
}

bool BluetoothAddress::operator<(const BluetoothAddress& other) const
{
	return m_address < other.m_address;
}

bool BluetoothAddress::operator!=(const BluetoothAddress& other) const
{
	return !(*this == other);
}

bool BluetoothAddress::operator==(const BluetoothAddress& other) const
{
	return m_address == other.m_address;
}

BluetoothAddress::operator QString() const
{
	QString s = QString("%1").arg(m_address, 12, 16, QChar('0'));
	for (int i = 2; i < s.size(); i += 2 + 1)
		s.insert(i, QChar(':'));
	return s.toUpper();
}

