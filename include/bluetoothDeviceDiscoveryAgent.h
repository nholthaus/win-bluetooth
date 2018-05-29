//--------------------------------------------------------------------------------------------------
// 
//	
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
/// @file	bluetoothDeviceDiscoveryAgent.h
/// @brief	The QBluetoothDeviceDiscoveryAgent class discovers the Bluetooth devices nearby.
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothDeviceDiscoveryAgent_h__
#define bluetoothDeviceDiscoveryAgent_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject> 

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothAddress;
class BluetoothDeviceInfo;
class BluetoothDeviceDiscoveryAgentPrivate;

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceDiscoveryAgent
//--------------------------------------------------------------------------------------------------

class BluetoothDeviceDiscoveryAgent : public QObject
{
	Q_OBJECT

public:

	enum DiscoveryMethod
	{
		NoMethod			= 0x00,		// The discovery is not possible.None of the available methods are supported.
		ClassicMethod		= 0x01,		// The discovery process searches for Bluetooth Classic(BaseRate) devices.
		LowEnergyMethod		= 0x02,		// The discovery process searches for Bluetooth Low Energy devices.
	};
	Q_ENUM(DiscoveryMethod);
	Q_DECLARE_FLAGS(DiscoveryMethods, DiscoveryMethod);

	enum Error
	{
		NoError							= 0,	// No error has occurred.
		InputOutputError				= 1,	// Writing or reading from the device resulted in an error.
		PoweredOffError					= 2,	// The Bluetooth adapter is powered off, power it on before doing discovery.
		InvalidBluetoothAdapterError	= 3,	// The passed local adapter address does not match the physical adapter address of any local Bluetooth device.
		UnsupportedPlatformError		= 4,	// Device discovery is not possible or implemented on the current platform.The error is set in response to a call to start().An example for such cases are iOS versions below 5.0 which do not support Bluetooth device search at all.This value was introduced by Qt 5.5.
		UnsupportedDiscoveryMethod		= 5,	// One of the requested discovery methods is not supported by the current platform.This value was introduced by Qt 5.8.
		UnknownError					= 100,	// An unknown error has occurred.
	};
	Q_ENUM(Error);

	enum InquiryType
	{
		GeneralUnlimitedInquiry			= 0,	// A general unlimited inquiry.Discovers all visible Bluetooth devices in the local vicinity.
		LimitedInquiry					= 1,	// A limited inquiry discovers devices that are in limited inquiry mode.
	};
	Q_ENUM(InquiryType);

public:

	BluetoothDeviceDiscoveryAgent(QObject *parent = nullptr);
	BluetoothDeviceDiscoveryAgent(const BluetoothAddress &deviceAdapter, QObject *parent = nullptr);
	virtual ~BluetoothDeviceDiscoveryAgent();
	QList<BluetoothDeviceInfo> discoveredDevices() const;
	Error error() const;
	QString errorString() const;
	InquiryType inquiryType() const;
	bool isActive() const;
	int lowEnergyDiscoveryTimeout() const;
	void setInquiryType(BluetoothDeviceDiscoveryAgent::InquiryType type);
	void setLowEnergyDiscoveryTimeout(int timeout);

	static DiscoveryMethods supportedDiscoveryMethods();

public slots:

	void start();
	void start(BluetoothDeviceDiscoveryAgent::DiscoveryMethods methods);
	void stop();

signals:

	void canceled();
	void deviceDiscovered(const BluetoothDeviceInfo &info);
	void error(BluetoothDeviceDiscoveryAgent::Error error);
	void finished();

protected:

	BluetoothDeviceDiscoveryAgentPrivate* d_ptr;

};
#endif // bluetoothDeviceDiscoveryAgent_h__
