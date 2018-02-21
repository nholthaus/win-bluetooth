//--------------------------------------------------------------------------------------------------
// 
//	WIN-BLUETOOTH
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
/// @file	bluetoothServiceDiscoveryAgent.h
/// @brief	class which queries devices for bluetooth services
///			refer to: https://doc.qt.io/qt-5/qbluetoothservicediscoveryagent.html
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothServiceDiscoveryAgent_h__
#define bluetoothServiceDiscoveryAgent_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject> 
#include <bluetoothUuids.h>
#include <bluetoothServiceinfo.h>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothAddress;
class BluetoothServiceDiscoveryAgentPrivate;

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceDiscoveryAgent
//--------------------------------------------------------------------------------------------------

class BluetoothServiceDiscoveryAgent : public QObject
{
	Q_OBJECT

public:

	enum DiscoveryMode
	{
		MinimalDiscovery		= 0,	///< Performs a minimal service discovery.The QBluetoothServiceInfo objects returned may be incomplete and are only guaranteed to contain device and service UUID information.Since a minimal discovery relies on cached SDP data it may not find a physically existing device until a FullDiscovery is performed.
		FullDiscovery			= 1,	///< Performs a full service discovery.
	};

	enum Error
	{
		NoError,						///< No error has occurred.
		PoweredOffError,				///< The Bluetooth adapter is powered off, power it on before doing discovery.
		InputOutputError,				///< Writing or reading from the device resulted in an error.
		InvalidBluetoothAdapterError,	///< The passed local adapter address does not match the physical adapter address of any local Bluetooth device.This value was introduced by Qt 5.3.
		UnknownError,					///< An unknown error has occurred.
	};

public:

	BluetoothServiceDiscoveryAgent(QObject *parent = Q_NULLPTR);
	BluetoothServiceDiscoveryAgent(const BluetoothAddress &deviceAdapter, QObject *parent = Q_NULLPTR);
	~BluetoothServiceDiscoveryAgent();
	QList<BluetoothServiceInfo> discoveredServices() const;
	Error error() const;
	QString errorString() const;
	bool isActive() const;
	BluetoothAddress remoteAddress() const;
	bool setRemoteAddress(const BluetoothAddress &address);
	void setUuidFilter(const QList<BluetoothUuid> &uuids);
	void setUuidFilter(const BluetoothUuid &uuid);
	QList<BluetoothUuid> uuidFilter() const;
	
public slots:

	void clear();
	void start(DiscoveryMode mode = MinimalDiscovery);
	void stop();

signals:

	void canceled();
	void error(BluetoothServiceDiscoveryAgent::Error error);
	void finished();
	void serviceDiscovered(const BluetoothServiceInfo &info);

private:

	Q_DECLARE_PRIVATE(BluetoothServiceDiscoveryAgent);
	QScopedPointer<BluetoothServiceDiscoveryAgentPrivate> d_ptr;

};


#endif // bluetoothServiceDiscoveryAgent_h__
