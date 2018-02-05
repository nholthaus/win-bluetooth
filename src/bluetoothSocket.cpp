//------------------------------
//	INCLUDES
//------------------------------

#include <Bluetooth.h>
#include <bluetoothSocket.h>
#include <bluetoothException.h>
#include <bluetoothAddress.h>

#include <WinSock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>
#include <cassert>

#include <QtBluetooth/QBluetoothSocket>

//------------------------------
//	MACROS
//------------------------------

#define ERR HRESULT_FROM_WIN32(GetLastError())

//--------------------------------------------------------------------------------------------------
//	BluetoothSocketPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothSocketPrivate
{
public:
	Q_DECLARE_PUBLIC(BluetoothSocket)

	BluetoothSocketPrivate(BluetoothSocket* q)
		: q_ptr(q)
	{
		if (!winsockInitialized)
		{
			// Ask for Winsock version 2.2.
			WSADATA WSAData = { 0 };
			if (WSAStartup(MAKEWORD(2, 2), &WSAData))
				throw BluetoothException("Unable to initialize Winsock version 2.2");

			winsockInitialized = true;
		}

		socket = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

		if (socket == INVALID_SOCKET)
			setError(BluetoothSocket::SocketError::UnknownSocketError);

		btAddress.addressFamily = AF_BTH;

		// set encryption
		ULONG bEncrypt = TRUE;
		if (SOCKET_ERROR == ::setsockopt(socket, SOL_RFCOMM, SO_BTH_ENCRYPT, (const char*)&bEncrypt, sizeof(ULONG)))
			setError(BluetoothSocket::SocketError::HostNotFoundError);
	}

	~BluetoothSocketPrivate()
	{
		closeSocket();	
	}

	void closeSocket()
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
			emit q->disconnected();
	}

	void setError(BluetoothSocket::SocketError error)
	{
		Q_Q(BluetoothSocket);

		this->error = error;
		errorString = BluetoothException(ERR).what();	// don't throw, just get the message
		emit q->error(this->error);
	}

	void setState(BluetoothSocket::SocketState state)
	{
		Q_Q(BluetoothSocket);

		this->state = state;
		emit q->stateChanged(this->state);
	}

	BluetoothSocket*				q_ptr;
	SOCKET							socket = INVALID_SOCKET;
	SOCKADDR_BTH					btAddress;
	static bool						winsockInitialized;
	BluetoothSocket::SocketState	state = BluetoothSocket::SocketState::UnconnectedState;
	BluetoothSocket::SocketError	error = BluetoothSocket::SocketError::NoSocketError;
	Protocol						protocol;
	QString							errorString;
};

bool BluetoothSocketPrivate::winsockInitialized = false;

//--------------------------------------------------------------------------------------------------
//	BluetoothSocket (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket::BluetoothSocket(QObject* parent)
	: QIODevice(parent)
	, d_ptr(new BluetoothSocketPrivate(this))
{

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

	if (SOCKET_ERROR == ::connect(d->socket, (struct sockaddr *) &d->btAddress, sizeof(SOCKADDR_BTH)))
	{
		d->setState(SocketState::UnconnectedState);
		d->setError(SocketError::HostNotFoundError);
		return;
	}

	d->setState(SocketState::ConnectedState);
	emit connected();

	QIODevice::setOpenMode(openMode);
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
	throw std::logic_error("The method or operation is not implemented.");
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
Protocol BluetoothSocket::socketType() const
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
	return d->error;
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

	if (auto ret = ::send(d->socket, data, maxlen, 0); ret == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAENETDOWN:
			d->setError(SocketError::NetworkError);
		case WSAETIMEDOUT:
			d->setError(SocketError::HostNotFoundError);
		default:
			d->setError(SocketError::UnknownSocketError);
		}

		return 0;
	}
	else if (!ret)
	{
		return 0;
	}
	else
		return ret;
}

//--------------------------------------------------------------------------------------------------
//	writeData () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::writeData(const char *data, qint64 len)
{
	Q_D(BluetoothSocket);
	
	if (auto ret = ::send(d->socket, data, len, 0); ret == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSAENETDOWN:
			d->setError(SocketError::NetworkError);
		case WSAETIMEDOUT:
			d->setError(SocketError::HostNotFoundError);
		default:
			d->setError(SocketError::UnknownSocketError);
		}

		return 0;
	}
	else
		return ret;
}

