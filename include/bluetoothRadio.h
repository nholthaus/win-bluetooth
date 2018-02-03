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
/// @file	bluetoothRadio.h
/// @brief	Description of a single bluetooth radio
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothRadio_h__
#define bluetoothRadio_h__

//------------------------------
//	INCLUDES
//------------------------------

#include <memory>
#include <QString>

//------------------------------
//	FORWARD DECLARATIONS
//------------------------------

class BluetoothUuid;
class BluetoothAddress;

//------------------------------
//	TYPE DEFINITIONS
//------------------------------


//--------------------------------------------------------------------------------------------------
//	BLUETOOTH RADIO
//--------------------------------------------------------------------------------------------------
class BluetoothRadio
{
public:

	BluetoothRadio(void* radioHandle = nullptr);
	virtual ~BluetoothRadio();
	BluetoothRadio(const BluetoothRadio& other) = delete;
	BluetoothRadio(BluetoothRadio&& other);
	BluetoothRadio& operator=(const BluetoothRadio& other) = delete;
	BluetoothRadio& operator=(BluetoothRadio&& other);

	void* handle();
	const void* handle() const;
	bool isValid() const;
	unsigned long long address() const;
	QString name() const;
	unsigned long classOfDevice() const;
	unsigned short manufacturer() const;

	bool discoverable() const;
	void setDiscoverable(bool discoverable);

	bool connectable() const;
	void setConnectable(bool connectable);

	bool connectTo(BluetoothAddress address);

	bool operator==(const QString& name) const;
	bool operator==(const unsigned long long address) const;

private:

	void*									m_handle;
	void*									m_radioInfo;
	bool									m_isValid;
	unsigned long long						m_address;
	QString									m_name;
	unsigned long							m_class;
	unsigned short							m_lmpSubversion;
	unsigned short							m_manufacturer;
	static std::unique_ptr<BluetoothUuid>	m_uuid;
};

#endif // bluetoothRadio_h__