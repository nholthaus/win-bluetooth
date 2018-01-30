//------------------------------
//	INCLUDES
//------------------------------

#include <bluetoothSocket.h>
#include <WinSock2.h>

//--------------------------------------------------------------------------------------------------
//	BluetoothSocketPrivate
//--------------------------------------------------------------------------------------------------

struct BluetoothSocketPrivate
{
	SOCKET socket;
};

//--------------------------------------------------------------------------------------------------
//	BluetoothSocket (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothSocket::BluetoothSocket()
{
	if (!address)
		return false;

	SOCKADDR_BTH btAddress;
	btAddress.addressFamily = AF_BTH;
	btAddress.serviceClassId = (*m_uuid)[Protocol::MSDNBluetoothConnectionExample];
	btAddress.port = 0;
	btAddress.btAddr = address;

	// create a socket
	SOCKET sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (sock == INVALID_SOCKET)
		throw BluetoothException(HRESULT_FROM_WIN32(WSAGetLastError()));

	// connect it to the remote device
	if (SOCKET_ERROR == connect(sock, (struct sockaddr *) &btAddress, sizeof(SOCKADDR_BTH)))
		throw BluetoothException("Bluetooth connection failed");

	return true;
}

//--------------------------------------------------------------------------------------------------
//	isSequential (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::isSequential() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
//	bytesAvailable () []
//--------------------------------------------------------------------------------------------------
qint64 BluetoothSocket::bytesAvailable() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
//	waitForReadyRead () []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::waitForReadyRead(int msecs)
{
	throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
//	waitForBytesWritten () []
//--------------------------------------------------------------------------------------------------
bool BluetoothSocket::waitForBytesWritten(int msecs)
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

