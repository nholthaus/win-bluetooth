//------------------------------
//	INCLUDES
//------------------------------

#include <bluetoothSocket.h>
#include <bluetoothException.h>

#include <WinSock2.h>
#include <bluetoothapis.h>
#include <ws2bth.h>

//--------------------------------------------------------------------------------------------------
//	BluetoothSocketPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothSocketPrivate
{
public:
	BluetoothSocketPrivate(BluetoothSocket* q)
		: q_ptr(q)
	{
		if (!m_winsockInitialized)
		{
			// Ask for Winsock version 2.2.
			WSADATA WSAData = { 0 };
			if (WSAStartup(MAKEWORD(2, 2), &WSAData))
				throw BluetoothException("Unable to initialize Winsock version 2.2");

			m_winsockInitialized = true;
		}

		m_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

		if (m_socket == INVALID_SOCKET)
			throw BluetoothException(HRESULT_FROM_WIN32(WSAGetLastError()));

		btAddress.addressFamily = AF_BTH;
	}

	~BluetoothSocketPrivate()
	{
		if (m_socket != INVALID_SOCKET)
			closesocket(m_socket);
	}

	BluetoothSocket*	q_ptr;
	SOCKET				m_socket = INVALID_SOCKET;
	SOCKADDR_BTH		btAddress;
	static bool			m_winsockInitialized;
};

bool BluetoothSocketPrivate::m_winsockInitialized = false;

//--------------------------------------------------------------------------------------------------
//	BluetoothSocket (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket::BluetoothSocket(QObject* parent)
	: QIODevice(parent)
{
// 	if (!address)
// 		return false;
// 
// 	SOCKADDR_BTH btAddress;
// 	btAddress.addressFamily = AF_BTH;
// 	btAddress.serviceClassId = (*m_uuid)[Protocol::MSDNBluetoothConnectionExample];
// 	btAddress.port = 0;
// 	btAddress.btAddr = address;
// 
// 	// connect it to the remote device
// 	if (SOCKET_ERROR == connect(sock, (struct sockaddr *) &btAddress, sizeof(SOCKADDR_BTH)))
// 		throw BluetoothException("Bluetooth connection failed");
// 
// 	return true;
}

//--------------------------------------------------------------------------------------------------
//	connectToService (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::connectToService(const BluetoothAddress& address, const BluetoothUuid& uuid, OpenMode openMode /*= ReadWrite*/)
{
	Q_D(BluetoothSocket);

	d->btAddress.serviceClassId = uuid;
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
//	close (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothSocket::close()
{
	throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
//	readData () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::readData(char *data, qint64 maxlen)
{
	throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
//	writeData () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::writeData(const char *data, qint64 len)
{
	throw std::logic_error("The method or operation is not implemented.");
}

