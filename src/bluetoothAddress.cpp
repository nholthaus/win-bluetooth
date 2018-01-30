#include "bluetoothAddress.h"

#include <algorithm>

BluetoothAddress::BluetoothAddress(uint64_t address)
	: m_address(address)
{

}

BluetoothAddress::BluetoothAddress(const QString& name)
{
	QString str = name;
	str.remove(':');
	m_address = str.toULongLong(nullptr, 16);
}

void BluetoothAddress::clear()
{
	m_address = 0;
}

bool BluetoothAddress::isNull() const
{
	return !m_address;
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

