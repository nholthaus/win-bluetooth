#pragma once
//------------------------------
//	INCLUDES
//------------------------------

#define  NOMINMAX

#include <Bluetooth.h>
#include <bluetoothSocket.h>
#include <bluetoothException.h>
#include <bluetoothAddress.h>
#include <bluetoothDeviceInfo.h>
#include <bluetoothServiceinfo.h>

#include <WinSock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>
#include <cassert>
#include <thread>
#include <array>

#include <QtBluetooth/QBluetoothSocket>
#include <QMutex>
#include <QWaitCondition>
#include <QObject>

//--------------------------------------------------------------------------------------------------
//	BluetoothSocketPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothSocketPrivate : public QObject
{
	Q_OBJECT
public:
	Q_DECLARE_PUBLIC(BluetoothSocket)
	BluetoothSocketPrivate(BluetoothSocket* q, SOCKET socketDescriptor = INVALID_SOCKET);
	~BluetoothSocketPrivate();

	void closeSocket();
	void setError(BluetoothSocket::SocketError error, QString errorString = QString());
	void setState(BluetoothSocket::SocketState state);
	void setReadComplete();

signals:

	void readyRead();
	void connected();
	void disconnected();
	void error(BluetoothSocket::SocketError error);
	void stateChanged(BluetoothSocket::SocketState state);

protected:

	bool clientHasDisconnected() const;

public:

	BluetoothSocket*				q_ptr;
	SOCKET							socket = INVALID_SOCKET;
	SOCKADDR_BTH					btAddress;
	BluetoothSocket::SocketState	state = BluetoothSocket::SocketState::UnconnectedState;
	BluetoothSocket::SocketError	err = BluetoothSocket::SocketError::NoSocketError;
	SecurityFlags					securityFlags;
	BluetoothServiceInfo::Protocol	protocol;
	QString							errorString;

	std::thread						checkConnectionThread;
	std::thread						readyReadThread;
	HANDLE							readEvent;
	HANDLE							joinEvent;
	HANDLE							readCompleteEvent;

	QMutex							readyReadMutex;
	QWaitCondition					readyReadCondition;

	QMutex							bytesWrittenMutex;
	QWaitCondition					bytesWrittenCondition;
};


