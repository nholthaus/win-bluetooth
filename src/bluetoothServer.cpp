#include <bluetoothServer.h>
#include <bluetooth.h>
#include <bluetoothServiceinfo.h>
#include <bluetoothSocket.h>
#include <bluetooth.h>
#include <bluetoothException.h>

#include <QCoreApplication>
#include <QVariant>
#include <QMutex>
#include <QWaitCondition>

#include <WinSock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>

#include <thread>

//------------------------------
//	MACROS
//------------------------------

#define ERR HRESULT_FROM_WIN32(GetLastError())

//--------------------------------------------------------------------------------------------------
//	BluetoothServerPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothServerPrivate
{
public:

	Q_DECLARE_PUBLIC(BluetoothServer);
	BluetoothServerPrivate(BluetoothServer* parent);
	virtual ~BluetoothServerPrivate();

	bool listen(const SOCKADDR_BTH& sockaddrBth);

protected:

	BluetoothServer* q_ptr;
	BluetoothSocket* listeningSocket = nullptr;
	QList<BluetoothSocket*> pendingConnections;
	BluetoothServiceInfo::Protocol serverType = BluetoothServiceInfo::UnknownProtocol;
	SecurityFlags securityFlags;

	bool isListening = false;
	unsigned int maxPendingConnections = 30;	// Windows max. https://msdn.microsoft.com/en-us/library/windows/desktop/aa362899(v=vs.85).aspx

	BluetoothServer::Error error = BluetoothServer::Error::NoError;
	QString errorString = "No Error";

	std::thread						acceptThread;
	HANDLE							acceptEvent;
	HANDLE							joinEvent;

	mutable QMutex					pendingConnectionsMutex;
	mutable QMutex					acceptMutex;
	QWaitCondition					acceptCondition;
};

//--------------------------------------------------------------------------------------------------
//	BluetoothServerPrivate (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServerPrivate::BluetoothServerPrivate(BluetoothServer* parent) 
	: q_ptr(parent)
{

}

//--------------------------------------------------------------------------------------------------
//	~BluetoothServerPrivate (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
BluetoothServerPrivate::~BluetoothServerPrivate()
{
	WSASetEvent(joinEvent);
	acceptThread.join();
}

//--------------------------------------------------------------------------------------------------
//	listen (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServerPrivate::listen(const SOCKADDR_BTH& sockaddrBth)
{
	Q_Q(BluetoothServer);

	if (q->isListening())
	{
		q->setError(BluetoothServer::Error::InputOutputError, "Server is already listening");
		return false;
	}

	// get a new socket
	auto socket = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (socket == INVALID_SOCKET)
	{
		q->setError(BluetoothServer::Error::UnknownError, BluetoothException(ERR).what());
		return false;
	}

	// bind the socket to the desired address
	if (SOCKET_ERROR == ::bind(socket, (struct sockaddr *) &sockaddrBth, sizeof(SOCKADDR_BTH)))
	{
		q->setError(BluetoothServer::Error::UnknownError, QString("Failed to bind socket: %1").arg(BluetoothException(ERR).what()));
		return false;
	}

	// make a BluetoothSocket out of it
	delete listeningSocket;
	listeningSocket = new BluetoothSocket;
	listeningSocket->setSocketDescriptor((int)socket, BluetoothServiceInfo::RfcommProtocol);

	// set the socket to listen
	if (SOCKET_ERROR == ::listen(socket, maxPendingConnections))
	{
		q->setError(BluetoothServer::Error::UnknownError, QString("Socket failed to listen: %1").arg(BluetoothException(ERR).what()));
		return false;
	}

	// accept incoming requests (async)
	// initialize the event handles
	acceptEvent = WSACreateEvent();
	joinEvent = WSACreateEvent();

	// start the ready-read thread
	acceptThread = std::thread([this]()
	{
		Q_Q(BluetoothServer);

		forever
		{
			HANDLE acceptOrJoin[2];
			acceptOrJoin[0] = acceptEvent;
			acceptOrJoin[1] = joinEvent;

			// wait for data to be ready to read.
			WSAEventSelect(listeningSocket->socketDescriptor(), acceptEvent, FD_ACCEPT);
			auto ret = WSAWaitForMultipleEvents(2, acceptOrJoin, FALSE, WSA_INFINITE, TRUE);
			if (ret - WSA_WAIT_EVENT_0 == 0)
			{
				SOCKADDR_BTH btAddr;
				int size = sizeof(SOCKADDR_BTH);
				SOCKET newSock = accept(listeningSocket->socketDescriptor(), (struct sockaddr*)&btAddr, &size);

				if (INVALID_SOCKET == newSock)
					q->setError(BluetoothServer::Error::InputOutputError, "Socket accept failed");
				else
				{
					BluetoothSocket* newBtSock = new BluetoothSocket;
					newBtSock->setSocketDescriptor(newSock, BluetoothServiceInfo::RfcommProtocol);
					newBtSock->moveToThread(qApp->thread());

					pendingConnectionsMutex.lock();
					pendingConnections.append(newBtSock);
					pendingConnectionsMutex.unlock();

					emit q->newConnection();
					acceptCondition.wakeAll();
				}
			}
			if (ret - WSA_WAIT_EVENT_0 == 1)
			{
				// asked to join, quit the loop!
				break;
			}

			// check for join event
			ret = WSAWaitForMultipleEvents(1, &joinEvent, TRUE, 0, FALSE);
			if ((ret != WSA_WAIT_FAILED) && (ret != WSA_WAIT_TIMEOUT))
			{
				break;
			}

			WSAResetEvent(acceptEvent);
			WSAResetEvent(joinEvent);
		}
	});

	return true;
}

//--------------------------------------------------------------------------------------------------
//	BluetoothServer (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServer::BluetoothServer(BluetoothServiceInfo::Protocol serverType /*= Protocol::RFCOMM*/, QObject* parent /*= nullptr*/)
	: QObject(parent)
	, d_ptr(new BluetoothServerPrivate(this))
{
	Q_D(BluetoothServer);

	// windows only supports RFCOMM for now
	if (serverType != BluetoothServiceInfo::RfcommProtocol)
		setError(Error::UnsupportedProtocolError, QString("Bluetooth server does not support protocol type '%1'.")
			.arg(QVariant::fromValue(serverType).toString()));
	else
		d->serverType = serverType;
}

//--------------------------------------------------------------------------------------------------
//	setError (protected ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServer::setError(Error error, QString errorString)
{
	Q_D(BluetoothServer);
	d->error = error;
	d->errorString = errorString;
	emit this->error(error);
}

//--------------------------------------------------------------------------------------------------
//	~BluetoothServer (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServer::~BluetoothServer()
{
	close();
}

//--------------------------------------------------------------------------------------------------
//	close (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServer::close()
{
	Q_D(BluetoothServer); 
	
	if(d->listeningSocket)
	{
		d->listeningSocket->close();
		delete d->listeningSocket;
		d->listeningSocket = nullptr;
	}
	d->isListening = false;
}

//--------------------------------------------------------------------------------------------------
//	error (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServer::Error BluetoothServer::error() const
{
	Q_D(const BluetoothServer);
	return d->error;
}

//--------------------------------------------------------------------------------------------------
//	errorString (public ) []
//--------------------------------------------------------------------------------------------------
QString BluetoothServer::errorString() const
{
	Q_D(const BluetoothServer);
	return d->errorString;
}

//--------------------------------------------------------------------------------------------------
//	hasPendingConnections (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServer::hasPendingConnections() const
{
	Q_D(const BluetoothServer);
	QMutexLocker(&d->pendingConnectionsMutex);
	return !d->pendingConnections.isEmpty();
}

//--------------------------------------------------------------------------------------------------
//	isListening (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServer::isListening() const
{
	Q_D(const BluetoothServer);
	return d->isListening;
}

//--------------------------------------------------------------------------------------------------
//	listen (public ) []
//--------------------------------------------------------------------------------------------------
// BluetoothServiceInfo BluetoothServer::listen(const BluetoothUuid &uuid, const QString &serviceName /*= QString()*/)
// {
// 
// }

//--------------------------------------------------------------------------------------------------
//	listen (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServer::listen(const BluetoothAddress& address /*= BluetoothAddress()*/, quint16 port /*= 0*/)
{
	Q_D(BluetoothServer);

	// setup the socket address
	BluetoothAddress addressToUse = address;
	if (addressToUse == BluetoothAddress())
		addressToUse = Bluetooth::localRadio().address();

	SOCKADDR_BTH btAddr = { 0 };
	btAddr.addressFamily = AF_BTH;
	btAddr.serviceClassId = GUID_NULL;
	btAddr.btAddr = addressToUse;
	btAddr.port = port != 0 ? port : BT_PORT_ANY;

	return d->listen(btAddr);
}

//--------------------------------------------------------------------------------------------------
//	maxPendingConnections (public ) []
//--------------------------------------------------------------------------------------------------
int BluetoothServer::maxPendingConnections() const
{
	Q_D(const BluetoothServer);
	return d->maxPendingConnections;
}

//--------------------------------------------------------------------------------------------------
//	nextPendingConnection (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket* BluetoothServer::nextPendingConnection()
{
	// callers responsibility to delete the socket
	Q_D(BluetoothServer);
	QMutexLocker(&d->pendingConnectionsMutex);
	if (d->pendingConnections.isEmpty())
		return nullptr;
	else
		return d->pendingConnections.takeFirst();
}

//--------------------------------------------------------------------------------------------------
//	securityFlags (public ) []
//--------------------------------------------------------------------------------------------------
SecurityFlags BluetoothServer::securityFlags() const
{
	Q_D(const BluetoothServer);
	if (d->listeningSocket)
		return d->listeningSocket->preferredSecurityFlags();
	else
		return d->securityFlags;
}

//--------------------------------------------------------------------------------------------------
//	serverAddress (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothServer::serverAddress() const
{
	Q_D(const BluetoothServer);
	if (d->listeningSocket)
		return d->listeningSocket->localAddress();
	else
		return BluetoothAddress();
}

//--------------------------------------------------------------------------------------------------
//	serverPort (public ) []
//--------------------------------------------------------------------------------------------------
quint16 BluetoothServer::serverPort() const
{
	Q_D(const BluetoothServer);
	if (d->listeningSocket)
		return d->listeningSocket->localPort();
	else
		return 0;
}

//--------------------------------------------------------------------------------------------------
//	serverType (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::Protocol BluetoothServer::serverType() const
{
	Q_D(const BluetoothServer);
	return d->serverType;
}

//--------------------------------------------------------------------------------------------------
//	setMaxPendingConnections (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServer::setMaxPendingConnections(int numConnections)
{
	Q_D(BluetoothServer);
	if (1 <= numConnections && 30 >= numConnections)
		d->maxPendingConnections = numConnections;
}

//--------------------------------------------------------------------------------------------------
//	setSecurityFlags (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServer::setSecurityFlags(SecurityFlags security)
{
	Q_D(BluetoothServer);
	d->securityFlags = security;
	if (d->listeningSocket)
		d->listeningSocket->setPreferredSecurityFlags(security);
}

//--------------------------------------------------------------------------------------------------
//	waitForConnected (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
bool BluetoothServer::waitForConnected(int msecs /*= 30000*/)
{
	Q_D(BluetoothServer);

	d->acceptMutex.lock();
	bool val = d->acceptCondition.wait(&d->acceptMutex, msecs);
	d->acceptMutex.unlock();
	return val;
}