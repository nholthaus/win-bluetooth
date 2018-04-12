#include "bluetoothSocket_p.h"
#include <QCoreApplication>

//------------------------------
//	MACROS
//------------------------------

#define ERR HRESULT_FROM_WIN32(GetLastError())

//--------------------------------------------------------------------------------------------------
//	BLUETOOTH SOCKET PRIVATE
//--------------------------------------------------------------------------------------------------
BluetoothSocketPrivate::BluetoothSocketPrivate(BluetoothSocket* q, SOCKET socketDescriptor /*= INVALID_SOCKET*/) : q_ptr(q)
{
	QObject::connect(this, &BluetoothSocketPrivate::connected, q, &BluetoothSocket::connected, Qt::QueuedConnection);
	QObject::connect(this, &BluetoothSocketPrivate::disconnected, q, &BluetoothSocket::disconnected, Qt::QueuedConnection);
	QObject::connect(this, &BluetoothSocketPrivate::stateChanged, q, &BluetoothSocket::stateChanged, Qt::QueuedConnection);
	QObject::connect(this, &BluetoothSocketPrivate::readyRead, q, &BluetoothSocket::readyRead, Qt::QueuedConnection);
	// Ask for Winsock version 2.2.
	WSADATA WSAData = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &WSAData))
		throw BluetoothException("Unable to initialize Winsock version 2.2");

	// means we're using a default and need to create a new socket
	if (socketDescriptor == INVALID_SOCKET)
	{
		socket = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

		// if it's still invalid that's a problem
		if (socket == INVALID_SOCKET)
			setError(BluetoothSocket::SocketError::UnknownSocketError);
	}
	// already have a socket descriptor
	else
	{
		socket = socketDescriptor;
	}

	btAddress.addressFamily = AF_BTH;

	// set encryption
	ULONG bEncrypt = TRUE;
	if (SOCKET_ERROR == ::setsockopt(socket, SOL_RFCOMM, SO_BTH_ENCRYPT, (const char*)&bEncrypt, sizeof(ULONG)))
		setError(BluetoothSocket::SocketError::OperationError, "Failed to set socket encryption.");

	// set encryption
	ULONG bAuthenticate = TRUE;
	if (SOCKET_ERROR == ::setsockopt(socket, SOL_RFCOMM, SO_BTH_AUTHENTICATE, (const char*)&bAuthenticate, sizeof(ULONG)))
		setError(BluetoothSocket::SocketError::OperationError, "Failed to set socket encryption.");

	// set non-blocking
	unsigned long buf = 1;
	unsigned long outBuf;
	DWORD sizeWritten = 0;
	if (SOCKET_ERROR == ::WSAIoctl(socket, FIONBIO, &buf, sizeof(unsigned long), &outBuf, sizeof(unsigned long), &sizeWritten, nullptr, nullptr))
		setError(BluetoothSocket::SocketError::UnknownSocketError);

	// initialize the event handles
	readEvent = WSACreateEvent();
	joinEvent = WSACreateEvent();
	readCompleteEvent = WSACreateEvent();

	// start the ready-read thread
	readyReadThread = std::thread([this]()
	{
		Q_Q(BluetoothSocket);

		forever
		{
			HANDLE readOrJoin[2];
			readOrJoin[0] = readEvent;
			readOrJoin[1] = joinEvent;

			// wait for data to be ready to read.
			WSAEventSelect(socket, readEvent, FD_READ);
			auto ret = WSAWaitForMultipleEvents(2, readOrJoin, FALSE, WSA_INFINITE, TRUE);
			if (ret - WSA_WAIT_EVENT_0 == 0)
			{
				emit this->readyRead();
				qApp->processEvents();
				readyReadCondition.wakeAll();
			}
			if (ret - WSA_WAIT_EVENT_0 == 1)
			{
				break;
			}

			// wait for the data to be read
			ret = WSAWaitForMultipleEvents(1, &readCompleteEvent, TRUE, WSA_INFINITE, FALSE);

			// check for join event
			ret = WSAWaitForMultipleEvents(1, &joinEvent, TRUE, 0, FALSE);
			if ((ret != WSA_WAIT_FAILED) && (ret != WSA_WAIT_TIMEOUT))
			{
				break;
			}

			WSAResetEvent(readEvent);
			WSAResetEvent(joinEvent);
			WSAResetEvent(readCompleteEvent);
		}
	});
}

BluetoothSocketPrivate::~BluetoothSocketPrivate()
{
	// set the events that could block the readyRead loop
	WSASetEvent(joinEvent);
	WSASetEvent(readCompleteEvent);

	closeSocket();
	readyReadThread.join();

	WSACloseEvent(readEvent);
	WSACloseEvent(joinEvent);
	WSACloseEvent(readCompleteEvent);

	WSACleanup();
}

void BluetoothSocketPrivate::closeSocket()
{
	Q_Q(BluetoothSocket);

	bool disconnected = false;
	if (state == BluetoothSocket::SocketState::ConnectedState)
		disconnected = true;

	setState(BluetoothSocket::SocketState::ClosingState);

	if (socket != INVALID_SOCKET)
	{
		if (SOCKET_ERROR == ::closesocket(socket))
		{
			setError(BluetoothSocket::SocketError::OperationError);
		}
	}

	// it's closed
	setState(BluetoothSocket::SocketState::UnconnectedState);

	if (disconnected)
		emit this->disconnected();
}

void BluetoothSocketPrivate::setError(BluetoothSocket::SocketError error, QString errorString /*= QString()*/)
{
	Q_Q(BluetoothSocket);

	this->err = error;
	if (!errorString.isEmpty())
	{
		if (*errorString.end() != QChar('.'))
		{
			errorString += '.';
		}
		errorString += ' ';
	}
	errorString += BluetoothException(ERR).what();	// don't throw, just get the message
	this->errorString = errorString;
	emit this->error(this->err);
}

void BluetoothSocketPrivate::setState(BluetoothSocket::SocketState state)
{
	Q_Q(BluetoothSocket);

	this->state = state;
	emit stateChanged(this->state);
}

void BluetoothSocketPrivate::setReadComplete()
{
	WSASetEvent(readCompleteEvent);
}
