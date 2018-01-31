//--------------------------------------------------------------------------------------------------
// 
///	@project WIN-BLUETOOTH
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2018 Nic Holthaus
// 
//--------------------------------------------------------------------------------------------------
//
// ATTRIBUTION:
//
//
//--------------------------------------------------------------------------------------------------
//
/// @file	bluetoothAddress.h
/// @brief	Description of a single bluetooth address
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothAddress_h__
#define bluetoothAddress_h__

#include <cstdint>
#include <QString>

class BluetoothAddress
{
public:

	BluetoothAddress() = default;
	BluetoothAddress(uint64_t address);
	BluetoothAddress(const QString& nameOrMac);
	BluetoothAddress(const char* nameOrMac);

	void clear();
	bool isNull() const;

	operator QString() const;
	operator uint64_t() const;
	
	bool operator==(uint64_t other) const;
	bool operator==(const QString& other) const;
	bool operator==(const BluetoothAddress& other) const;
	bool operator!=(const BluetoothAddress& other) const;
	bool operator<(const BluetoothAddress& other) const;

private:

	void addressFromString(const QString& nameOrMac);

private:

	uint64_t	m_address = 0;
};

#endif // bluetoothAddress_h__