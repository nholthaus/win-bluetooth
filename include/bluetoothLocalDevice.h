//--------------------------------------------------------------------------------------------------
// 
//	win-bluetooth
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
/// @file	bluetoothLocalDevice.h
/// @brief	
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothLocalDevice_h__
#define bluetoothLocalDevice_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject>
#include <QList>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothAddress;
class BluetoothHostInfo;
class BluetoothLocalDevicePrivate;

//--------------------------------------------------------------------------------------------------
//	BluetoothLocalDevice
//--------------------------------------------------------------------------------------------------

class BluetoothLocalDevice : public QObject
{

	Q_OBJECT

public:

	enum Error
	{
		NoError							= 0,	// No known error
		PairingError					= 1,	// Error in pairing
		UnknownError					= 100,	// Unknown error
	};
	Q_ENUM(Error);

	enum HostMode
	{
		HostPoweredOff					= 0,	// Power off the device
		HostConnectable					= 1,	// Remote Bluetooth devices can connect to the local Bluetooth device if they have previously been paired with it or otherwise know its address.This powers up the device if it was powered off.
		HostDiscoverable				= 2,	// Remote Bluetooth devices can discover the presence of the local Bluetooth device.The device will also be connectable, and powered on.On Android, this mode can only be active for a maximum of 5 minutes.
		HostDiscoverableLimitedInquiry	= 3,	// Remote Bluetooth devices can discover the presence of the local Bluetooth device when performing a limited inquiry.This should be used for locating services that are only made discoverable for a limited period of time.This can speed up discovery between gaming devices, as service discovery can be skipped on devices not in LimitedInquiry mode.In this mode, the device will be connectable and powered on, if required.This mode is is not supported on Android.On macOS, it is not possible to set the hostMode() to HostConnectable or HostPoweredOff.
	};
	Q_ENUM(HostMode);

	enum Pairing
	{
		Unpaired						= 0,	// The Bluetooth devices are not paired.
		Paired							= 1,	// The Bluetooth devices are paired.The system will prompt the user for authorization when the remote device initiates a connection to the local device.
		AuthorizedPaired				= 2,	// The Bluetooth devices are paired.The system will not prompt the user for authorization when the remote device initiates a connection to the local device.
	};
	Q_ENUM(Pairing);

public:

	BluetoothLocalDevice(QObject* parent = nullptr);
	BluetoothLocalDevice(const BluetoothAddress& address, QObject* parent = nullptr);
	virtual ~BluetoothLocalDevice();
	
	BluetoothAddress address() const;
	QList<BluetoothAddress> connectedDevices() const;
	HostMode hostMode() const;
	bool isValid() const;
	QString name() const;
	Pairing pairingStatus(const BluetoothAddress& address) const;
	void powerOn();
	void requestPairing(const BluetoothAddress &address, Pairing pairing);
	void setHostMode(BluetoothLocalDevice::HostMode mode);
	
	static QList<BluetoothHostInfo> allDevices();

public slots:
	
	void pairingConfirmation(bool confirmation);

signals:

	void 	deviceConnected(const BluetoothAddress &address);
	void 	deviceDisconnected(const BluetoothAddress &address);
	void 	error(BluetoothLocalDevice::Error error);
	void 	hostModeStateChanged(BluetoothLocalDevice::HostMode state);
	void 	pairingDisplayConfirmation(const BluetoothAddress &address, QString pin);
	void 	pairingDisplayPinCode(const BluetoothAddress &address, QString pin);
	void 	pairingFinished(const BluetoothAddress &address, BluetoothLocalDevice::Pairing pairing);

private:

	Q_DECLARE_PRIVATE(BluetoothLocalDevice)
	BluetoothLocalDevicePrivate* d_ptr;

};

#endif // bluetoothLocalDevice_h__
