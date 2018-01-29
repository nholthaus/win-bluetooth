#include "bluetoothAddress.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

BluetoothAddress::BluetoothAddress(uint64_t address)
	: m_address(address)
{

}

BluetoothAddress::BluetoothAddress(const std::wstring& name)
{
	std::wstring str = name;
	str.erase(std::remove_if(str.begin(), str.end(), [](wchar_t x) {return x == L':'; }));
	m_address = (uint64_t)wcstol(str.c_str(), nullptr, 16);
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

BluetoothAddress::operator std::wstring() const
{
	std::wstringstream ss;
	ss << std::setfill(L'0') << std::setw(12) << std::hex << m_address;
	std::wstring s = ss.str();
	ss.str(L"");
	for (int i = 0; i < s.size(); i++) {
		if (i % 2 == 0 && i) ss << ':';
		ss << s[i];
	}
	return ss.str();
}

