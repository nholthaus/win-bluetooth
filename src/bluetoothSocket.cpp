//------------------------------
//	INCLUDES
//------------------------------

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

#include <bluetoothSocket_p.h>

//------------------------------
//	MACROS
//------------------------------

#define ERR HRESULT_FROM_WIN32(GetLastError())

//--------------------------------------------------------------------------------------------------
//	canReadLine (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::canReadLine() const
{
	return false;
}

//--------------------------------------------------------------------------------------------------
//	BluetoothSocket (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket::BluetoothSocket(QObject* parent)
	: QIODevice(parent)
	, d_ptr(new BluetoothSocketPrivate(this))
{
	connect((QIODevice*)this, &QIODevice::readyRead, this, &BluetoothSocket::readyRead);
}

//--------------------------------------------------------------------------------------------------
//	~BluetoothSocket (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
BluetoothSocket::~BluetoothSocket()
{
	// can't have an in-line destructor because of the PIMPL with a scoped ptr
	// don't put anything in here though: use smart pointers!
}

//--------------------------------------------------------------------------------------------------
//	abort (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::abort()
{
	Q_D(BluetoothSocket);
	d->closeSocket();
}

//--------------------------------------------------------------------------------------------------
//	connectToService (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::connectToService(const BluetoothAddress& address, const BluetoothUuid& uuid, OpenMode openMode /*= ReadWrite*/)
{
	Q_D(BluetoothSocket);
	d->btAddress.serviceClassId = uuid;
	d->btAddress.port = 0;
	connectToService(address, openMode);
}

//--------------------------------------------------------------------------------------------------
//	connectToService (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::connectToService(const BluetoothAddress& address, quint16 port, OpenMode openMode /*= ReadWrite*/)
{
	Q_D(BluetoothSocket);
	d->btAddress.serviceClassId = { 0 };
	d->btAddress.port = port;
	connectToService(address, openMode);
}

//--------------------------------------------------------------------------------------------------
//	connectToService (private ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::connectToService(const BluetoothAddress& address, OpenMode openMode /*= ReadWrite*/)
{
	Q_D(BluetoothSocket);

	d->setState(SocketState::ConnectingState);

	if (!address)
	{
		d->setState(SocketState::UnconnectedState);
		d->setError(SocketError::HostNotFoundError);
		return;
	}

	d->btAddress.btAddr = address;

	int count = 0;
	int count2 = 0;
	forever
	{
		++count;
		int connectResult = ::WSAConnect(d->socket, (struct sockaddr *) &d->btAddress, sizeof(SOCKADDR_BTH), nullptr,nullptr,nullptr,nullptr);
		if (connectResult == SOCKET_ERROR) 
		{
			++count2;
			int err = WSAGetLastError();

			switch (err) 
			{
			case WSANOTINITIALISED:
				//###
				break;
			case WSAEISCONN:
				d->setState(SocketState::ConnectedState);
				break;
			case WSAEWOULDBLOCK: {
				// If WSAConnect returns WSAEWOULDBLOCK on the second
				// connection attempt, we have to check SO_ERROR's
				// value to detect ECONNREFUSED. If we don't get
				// ECONNREFUSED, we'll have to treat it as an
				// unfinished operation.
				int value = 0;
				int valueSize = sizeof(value);
				bool tryAgain = false;
				bool errorDetected = false;
				int tries = 0;
				do 
				{
					if (::getsockopt(d->socket, SOL_SOCKET, SO_ERROR, (char *)&value, &valueSize) == 0) {
						if (value != NOERROR) {
							// MSDN says getsockopt with SO_ERROR clears the error, but it's not actually cleared
							// and this can affect all subsequent WSAConnect attempts, so clear it now.
							const int val = NO_ERROR;
							::setsockopt(d->socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<const char*>(&val), sizeof val);
						}

						if (value == WSAECONNREFUSED) {
							d->setError(SocketError::NetworkError, "Connection refused.");
							d->setState(SocketState::UnconnectedState);
							errorDetected = true;
							break;
						}
						if (value == WSAETIMEDOUT) {
							d->setError(SocketError::NetworkError, "Connection timed out.");
							d->setState(SocketState::UnconnectedState);
							errorDetected = true;
							break;
						}
						if (value == WSAEHOSTUNREACH) {
							d->setError(SocketError::NetworkError, "Destination host unreachable");
							d->setState(SocketState::UnconnectedState);
							errorDetected = true;
							break;
						}
						if (value == WSAEADDRNOTAVAIL) {
							d->setError(SocketError::NetworkError,  "Address not available");
							d->setState(SocketState::UnconnectedState);
							errorDetected = true;
							break;
						}
						if (value == NOERROR) {
							// When we get WSAEWOULDBLOCK the outcome was not known, so a
							// NOERROR might indicate that the result of the operation
							// is still unknown. We try again to increase the chance that we did
							// get the correct result.
							tryAgain = !tryAgain;
						}
					}
					tries++;
				} while (tryAgain && (tries < 2));

				if (errorDetected)
					break;
				Q_FALLTHROUGH();
			}
			case WSAEINPROGRESS:
				continue;
			case WSAEADDRINUSE:
				d->setError(SocketError::NetworkError,  "Address in use.");
				break;
			case WSAECONNREFUSED:
				d->setError(SocketError::NetworkError,  "Connection refused.");
				d->setState(SocketState::UnconnectedState);
				break;
			case WSAETIMEDOUT:
				d->setError(SocketError::NetworkError,  "Connection timed out.");
				break;
			case WSAEACCES:
				d->setError(SocketError::NetworkError,  "Socket access error.");
				d->setState(SocketState::UnconnectedState);
				break;
			case WSAEHOSTUNREACH:
				d->setError(SocketError::NetworkError,  "Destination host unreachable.");
				d->setState(SocketState::UnconnectedState);
				break;
			case WSAENETUNREACH:
				d->setError(SocketError::NetworkError,  "Network unreachable");
				d->setState(SocketState::UnconnectedState);
				break;
			case WSAEINVAL:
			case WSAEALREADY:
				continue;
				break;
			default:
				break;
			}
		}
		break;
	}

	QIODevice::open(openMode);

	d->setState(SocketState::ConnectedState);
	emit connected();
}

//--------------------------------------------------------------------------------------------------
//	connectToService (private ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::connectToService(const BluetoothServiceInfo& service, OpenMode openMode /*= ReadWrite*/)
{
	BluetoothUuid uuidToUse = service.serviceUuid() == BluetoothUuid() ? service.serviceClassUuids().first() : service.serviceUuid();
	this->connectToService(service.device().address(), uuidToUse, openMode);
}

//--------------------------------------------------------------------------------------------------
//	select (private ) []
//--------------------------------------------------------------------------------------------------
int BluetoothSocket::select(int timeout_ms, bool selectForRead /*= true*/) const
{
	const Q_D(BluetoothSocket);

	fd_set fds;

	int ret = 0;

	memset(&fds, 0, sizeof(fd_set));
	fds.fd_count = 1;
	fds.fd_array[0] = (SOCKET)d->socket;

	struct timeval tv;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	if (selectForRead) {
		ret = ::select(0, &fds, nullptr, nullptr, timeout_ms < 0 ? nullptr : &tv);
	}
	else {
		// select for write

		// Windows needs this to report errors when connecting a socket ...
		fd_set fdexception;
		FD_ZERO(&fdexception);
		FD_SET((SOCKET)d->socket, &fdexception);

		ret = ::select(0, nullptr, &fds, &fdexception, timeout_ms < 0 ? nullptr : &tv);

		// ... but if it is actually set, pretend it did not happen
		if (ret > 0 && FD_ISSET((SOCKET)d->socket, &fdexception))
			ret--;
	}

	return ret;
}

//--------------------------------------------------------------------------------------------------
//	disconnectFromService (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::disconnectFromService()
{
	Q_D(BluetoothSocket);
	d->closeSocket();
}

//--------------------------------------------------------------------------------------------------
//	isSequential (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::isSequential() const
{
	return true;
}

//--------------------------------------------------------------------------------------------------
//	bytesAvailable () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::bytesAvailable() const
{
	const Q_D(BluetoothSocket);

	unsigned long  nbytes = 0;
	unsigned long dummy = 0;
	DWORD sizeWritten = 0;
	if (SOCKET_ERROR == ::WSAIoctl(d->socket, FIONREAD, &dummy, sizeof(dummy), &nbytes, sizeof(nbytes), &sizeWritten, nullptr, nullptr))
		return -1;

	return nbytes;
}

//--------------------------------------------------------------------------------------------------
//	peerAddress (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothSocket::peerAddress() const
{
	const Q_D(BluetoothSocket);
	return d->btAddress.btAddr;
}

//--------------------------------------------------------------------------------------------------
//	peerName (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothSocket::peerName() const
{
	const Q_D(BluetoothSocket);
	return Bluetooth::name(d->btAddress.btAddr);
}

//--------------------------------------------------------------------------------------------------
//	peerPort (public ) []
//--------------------------------------------------------------------------------------------------
quint16 BluetoothSocket::peerPort() const
{
	const Q_D(BluetoothSocket);
	return d->btAddress.port;
}

//--------------------------------------------------------------------------------------------------
//	socketType (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::Protocol BluetoothSocket::socketType() const
{
	const Q_D(BluetoothSocket);
	return d->protocol;
}

//--------------------------------------------------------------------------------------------------
//	state (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket::SocketState BluetoothSocket::state() const
{
	const Q_D(BluetoothSocket);
	return d->state;
}

//--------------------------------------------------------------------------------------------------
//	preferredSecurityFlags (public ) []
//--------------------------------------------------------------------------------------------------
SecurityFlags BluetoothSocket::preferredSecurityFlags() const
{
	Q_D(const BluetoothSocket);
	return d->securityFlags;
}

//--------------------------------------------------------------------------------------------------
//	setPreferredSecurityFlags (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::setPreferredSecurityFlags(SecurityFlags flags)
{
	Q_D(BluetoothSocket);
	d->securityFlags = flags;

	if(flags.testFlag(Security::Encryption))
	{
		// set encryption
		ULONG bEncrypt = TRUE;
		if (SOCKET_ERROR == ::setsockopt(d->socket, SOL_RFCOMM, SO_BTH_ENCRYPT, (const char*)&bEncrypt, sizeof(ULONG)))
			d->setError(BluetoothSocket::SocketError::OperationError, "Failed to set socket encryption.");
	}
	if (flags.testFlag(Security::Authentication))
	{
		// set encryption
		ULONG bAuthenticate = TRUE;
		if (SOCKET_ERROR == ::setsockopt(d->socket, SOL_RFCOMM, SO_BTH_AUTHENTICATE, (const char*)&bAuthenticate, sizeof(ULONG)))
			d->setError(BluetoothSocket::SocketError::OperationError, "Failed to set socket encryption.");
	}
}

//--------------------------------------------------------------------------------------------------
//	setSocketDescriptor (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::setSocketDescriptor(int socketDescriptor, BluetoothServiceInfo::Protocol socketType, SocketState socketState /*= SocketState::ConnectedState*/, OpenMode openMode /*= ReadWrite*/)
{
	Q_D(BluetoothSocket);

	// get socket address
	SOCKADDR_BTH btAddress;
	int nameLen = sizeof(SOCKADDR_BTH);
	quint64 socket = socketDescriptor;

	if (SOCKET_ERROR == getsockname((SOCKET)socket, (SOCKADDR*)&btAddress, &nameLen))
	{
		d->setError(BluetoothSocket::SocketError::OperationError, BluetoothException(ERR).what());
		return false;
	}
	else
	{
		// socket is real, set it up!
		d_ptr.reset(new BluetoothSocketPrivate(this, (SOCKET)socketDescriptor));
		d_ptr->protocol = socketType;
		d_ptr->state = socketState;
		d_ptr->btAddress = btAddress;
		this->open(openMode);
		return true;
	}
}

//--------------------------------------------------------------------------------------------------
//	socketDescriptor (public ) []
//--------------------------------------------------------------------------------------------------
int BluetoothSocket::socketDescriptor() const
{
	Q_D(const BluetoothSocket);
	return (int)d->socket;
}

//--------------------------------------------------------------------------------------------------
//	close (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::close()
{
	Q_D(BluetoothSocket);
	return d->closeSocket();
}

//--------------------------------------------------------------------------------------------------
//	error (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket::SocketError BluetoothSocket::error() const
{
	const Q_D(BluetoothSocket);
	return d->err;
}

//--------------------------------------------------------------------------------------------------
//	errorString (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothSocket::errorString() const
{
	const Q_D(BluetoothSocket);
	return d->errorString;
}

//--------------------------------------------------------------------------------------------------
//	localAddress () []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothSocket::localAddress() const
{
	assert(false);
	throw BluetoothException("This method is unimplemented");
}

//--------------------------------------------------------------------------------------------------
//	localName () []
//--------------------------------------------------------------------------------------------------
QString BluetoothSocket::localName() const
{
	assert(false);
	throw BluetoothException("This method is unimplemented");
}

//--------------------------------------------------------------------------------------------------
//	localPort () []
//--------------------------------------------------------------------------------------------------
quint16 BluetoothSocket::localPort() const
{
	assert(false);
	throw BluetoothException("This method is unimplemented");
}

//--------------------------------------------------------------------------------------------------
//	readData () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::readData(char *data, qint64 maxlen)
{
	Q_D(BluetoothSocket);
	Q_ASSERT(this->isOpen());

	qint64 ret = -1;
	WSABUF buf;
	buf.buf = data;
	buf.len = maxlen;
	DWORD flags = 0;
	DWORD bytesRead = 0;
	OVERLAPPED overlapped = { 0 };

	overlapped.hEvent = d->readCompleteEvent;	// this ends up sending the readyRead signal

	if (SOCKET_ERROR == ::WSARecv(d->socket, &buf, 1, &bytesRead, &flags, &overlapped, nullptr))
	{
		int err = WSAGetLastError();

		switch (err) 
		{
		case WSAEWOULDBLOCK:
			ret = -2;
			break;
		case WSAEBADF:
			d->setError(SocketError::NetworkError, "The socket handle is invalid.");
			break;
		case WSAEINVAL:
			d->setError(SocketError::NetworkError, "An invalid argument was supplied.");
			break;
		case WSAECONNRESET:
			d->setError(SocketError::NetworkError, "Connection was closed by the remote host.");
			break;
		case WSAECONNABORTED:
			d->setError(SocketError::NetworkError, "User aborted connection.");
			break;
			ret = 0;
			break;
		default:
			break;
		}
	}
	else 
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
			ret = 0;
		else
		{
			ret = qint64(bytesRead);
		}
	}

	return ret;
}

//--------------------------------------------------------------------------------------------------
//	writeData () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::writeData(const char *data, qint64 len)
{
	Q_D(BluetoothSocket);
	Q_ASSERT(this->isOpen());

	// inspired by qnativesocketengine_win.cpp
	qint64 ret = 0;
	qint64 bytesToSend = len;

	for (;;) 
	{
		WSABUF buf;
		buf.buf = (char*)data + ret;
		buf.len = bytesToSend;
		DWORD flags = 0;
		DWORD bytesWritten = 0;

		int socketRet = ::WSASend(d->socket, &buf, 1, &bytesWritten, flags, nullptr, nullptr);
		
		this->bytesWritten(bytesWritten);
		ret += qint64(bytesWritten);

		int err;
		if (socketRet != SOCKET_ERROR) {
			if (ret == len)
				break;
			else
				continue;
		}
		else if ((err = WSAGetLastError()) == WSAEWOULDBLOCK) {
			break;
		}
		else if (err == WSAENOBUFS) 
		{
			qDebug() << "This thing that wasn't supposed to happen happens";
			// this function used to not send more than 49152 per call to WSASendTo
			// to avoid getting a WSAENOBUFS. However this is a performance regression
			// and we think it only appears with old windows versions. We now handle the
			// WSAENOBUFS and hope it never appears anyway.
			// just go on, the next loop run we will try a smaller number
		}
		else 
		{
			auto err = WSAGetLastError();
			switch (err) {
			case WSAECONNRESET:
			case WSAECONNABORTED:
				ret = -1;
				d->setError(SocketError::NetworkError);
				close();
				break;
			default:
				throw BluetoothException(err);
			}
			break;
		}

		// for next send:
		bytesToSend = len - ret;
	}

	d->bytesWrittenCondition.wakeAll();

	return ret;
}

//--------------------------------------------------------------------------------------------------
//	waitForReadyRead (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::waitForReadyRead(int msecs)
{
	Q_D(BluetoothSocket);

	d->readyReadMutex.lock();
	bool val = d->readyReadCondition.wait(&d->readyReadMutex, msecs);
	d->readyReadMutex.unlock();
	return val;
}

//--------------------------------------------------------------------------------------------------
//	waitForBytesWritten (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::waitForBytesWritten(int msecs)
{
	Q_D(BluetoothSocket);

	d->bytesWrittenMutex.lock();
	bool val = d->bytesWrittenCondition.wait(&d->bytesWrittenMutex, msecs);
	d->bytesWrittenMutex.unlock();
	return val;
}
