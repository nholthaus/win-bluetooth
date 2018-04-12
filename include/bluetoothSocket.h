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
/// @file	bluetoothSocket.h
/// @brief	A Socket/QIODevice wrapper around winsock bluetooth sockets
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothSocket_h__
#define bluetoothSocket_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QIODevice>
#include <QScopedPointer>
#include <QAbstractSocket>

#include <bluetoothUuids.h>
#include <bluetoothEnums.h>
#include <bluetoothServiceinfo.h>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothSocketPrivate;
class BluetoothAddress;

//--------------------------------------------------------------------------------------------------
//	BluetoothSocket
//--------------------------------------------------------------------------------------------------

class BluetoothSocket : public QIODevice
{
	Q_OBJECT

public:

	enum class SocketState
	{
		UnconnectedState		= QAbstractSocket::UnconnectedState,
		ServiceLookupState		= QAbstractSocket::HostLookupState,
		ConnectingState			= QAbstractSocket::ConnectingState,
		ConnectedState			= QAbstractSocket::ConnectedState,
		BoundState				= QAbstractSocket::BoundState,
		ClosingState			= QAbstractSocket::ClosingState,
		ListeningState			= QAbstractSocket::ListeningState,
	};
	Q_ENUM(SocketState);

	enum class SocketError
	{
		UnknownSocketError			= QAbstractSocket::UnknownSocketError,
		NoSocketError				= - 2,
		HostNotFoundError			= QAbstractSocket::HostNotFoundError,
		ServiceNotFoundError		= QAbstractSocket::SocketAddressNotAvailableError,
		NetworkError				= QAbstractSocket::NetworkError,
		UnsupportedProtocolError	= 8,
		OperationError				= QAbstractSocket::OperationError,
		RemoteHostClosedError		= QAbstractSocket::RemoteHostClosedError,
	};
	Q_ENUM(SocketError);

private:

	Q_DISABLE_COPY(BluetoothSocket)

public:

	BluetoothSocket(QObject* parent = nullptr);
	virtual ~BluetoothSocket();

	void abort();
	void connectToService(const BluetoothServiceInfo& service, OpenMode openMode = ReadWrite);
	void connectToService(const BluetoothAddress& address, const BluetoothUuid& uuid, OpenMode openMode = ReadWrite);
	void connectToService(const BluetoothAddress& address, quint16 port, OpenMode openMode = ReadWrite);
	void disconnectFromService();
	SocketError error() const;
	QString errorString() const;
	BluetoothAddress localAddress() const;
	QString localName() const;
	quint16 localPort() const;
	BluetoothAddress peerAddress() const;
	QString peerName() const;
	quint16 peerPort() const;
	SecurityFlags preferredSecurityFlags() const;
	void setPreferredSecurityFlags(SecurityFlags flags);
	bool setSocketDescriptor(int socketDescriptor, BluetoothServiceInfo::Protocol socketType, SocketState socketState = SocketState::ConnectedState, OpenMode openMode = ReadWrite);
	int socketDescriptor() const;
	BluetoothServiceInfo::Protocol socketType() const;
	SocketState state() const;

	
public:

	virtual qint64 bytesAvailable() const override;
	virtual bool canReadLine() const override;
	virtual void close() override;
	virtual bool isSequential() const override;

	virtual bool waitForReadyRead(int msecs) override;
	virtual bool waitForBytesWritten(int msecs) override;

signals:

	void readyRead();
	void connected();
	void disconnected();
	void error(SocketError error);
	void stateChanged(SocketState state);

protected:

	virtual qint64 readData(char *data, qint64 maxlen) override;
	virtual qint64 writeData(const char *data, qint64 len) override;

private:

	void connectToService(const BluetoothAddress& address, OpenMode openMode = ReadWrite);
	int select(int timeout_ms, bool selectForRead = true) const;

private:

	Q_DECLARE_PRIVATE(BluetoothSocket)
	QScopedPointer<BluetoothSocketPrivate> d_ptr;

};

#endif // bluetoothSocket_h__
