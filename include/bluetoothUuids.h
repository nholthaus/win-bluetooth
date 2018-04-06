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
/// @file	bluetoothUuids.h
/// @brief	Known bluetooth service class Uuids
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothUuids_h__
#define bluetoothUuids_h__

//------------------------------
//	INCLDUES
//------------------------------

#include <QObject>
#include <QUuid>
#include <bluetoothEnums.h>
#include <unordered_map>

//--------------------------------------------------------------------------------------------------
//	BLUETOOTH UUID
//--------------------------------------------------------------------------------------------------
class BluetoothUuid
{
public:
	
	BluetoothUuid();
	BluetoothUuid(QUuid uuid);
	BluetoothUuid(quint16 uuid);
	BluetoothUuid(quint32 uuid);
	BluetoothUuid(Protocol protocol);
	BluetoothUuid(ServiceClass serviceClass);

	QString toString() const;

	operator QUuid() const;
	operator GUID() const;
	bool operator==(const BluetoothUuid& other) const;

private:

	static std::unordered_map<Protocol, QUuid>		m_protocolUuids;
	static std::unordered_map<ServiceClass, QUuid>	m_serviceClassUuids;
	
	QUuid m_uuid;
};

Q_DECLARE_METATYPE(BluetoothUuid)

#endif // bluetoothUuids_h__
