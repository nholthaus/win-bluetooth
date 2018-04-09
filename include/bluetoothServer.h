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
/// @file	bluetoothServer.h
/// @brief	
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothServer_h__
#define bluetoothServer_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <bluetoothServiceInfo.h>

#include <QObject>
#include <QScopedPointer>
#include <bluetoothEnums.h>
#include <bluetoothAddress.h>
#include <memory>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothUuid;
class BluetoothServerPrivate;
class BluetoothSocket;
class BluetoothServiceInfo;

//--------------------------------------------------------------------------------------------------
//	BluetoothServer
//--------------------------------------------------------------------------------------------------

class BluetoothServer : public QObject
{
	Q_OBJECT

public:

	enum class Error
	{
		NoError,
		UnknownError,
		PoweredOffError,
		InputOutputError,
		ServiceAlreadyRegisteredError,
		UnsupportedProtocolError,
	};
	Q_ENUM(Error);

public:

	BluetoothServer(BluetoothServiceInfo::Protocol serverType = BluetoothServiceInfo::RfcommProtocol, QObject* parent = nullptr);
	virtual ~BluetoothServer();

	void close();
	Error error() const;
	QString errorString() const;
	bool hasPendingConnections() const;
	bool isListening() const;
	bool listen(const BluetoothAddress& address = BluetoothAddress(), quint16 port = 0);
	BluetoothServiceInfo listen(const BluetoothUuid &uuid, const QString &serviceName = QString());
	int maxPendingConnections() const;
	BluetoothSocket* nextPendingConnection();
	SecurityFlags securityFlags() const;
	BluetoothAddress serverAddress() const;
	quint16 serverPort() const;
	BluetoothServiceInfo::Protocol serverType() const;
	void setMaxPendingConnections(int numConnections);
	void setSecurityFlags(SecurityFlags security);

public:

	virtual bool waitForConnected(int msecs = 30000);

signals:

	void error(Error error);
	void newConnection();

protected:

	void setError(Error error, QString errorString);

protected:

	Q_DECLARE_PRIVATE(BluetoothServer);
	Q_DISABLE_COPY(BluetoothServer);
	QScopedPointer<BluetoothServerPrivate> d_ptr;

};

#endif // bluetoothServer_h__
