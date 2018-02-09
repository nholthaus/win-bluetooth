#include <bluetoothTransferManager.h>
#include <bluetoothAddress.h>
#include <QIODevice>
#include <QtConcurrent/QtConcurrent>
#include <bluetoothSocket.h>
#include <obexRequest.h>
#include <obexResponse.h>
#include <bluetoothTransferReply.h>
#include <bluetoothException.h>
#include <bluetoothTransferRequest.h>

//--------------------------------------------------------------------------------------------------
//	BluetoothTransferManager (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothTransferManager::BluetoothTransferManager(QObject* parent /*= nullptr*/)
	: QObject(parent)
{

}

//--------------------------------------------------------------------------------------------------
//	put (public ) []
//--------------------------------------------------------------------------------------------------
QSharedPointer<BluetoothTransferReply> BluetoothTransferManager::put(const BluetoothTransferRequest& request, QSharedPointer<QIODevice> data)
{
	QSharedPointer<BluetoothTransferReply> reply;
	reply->setManager(this);
	reply->setRequest(request);

	// run the put operation in a thread because it blocks
	QtConcurrent::run([=]()
	{
		BluetoothSocket sock;
		QDataStream sockStream(&sock);

		sock.connectToService(request.address(), BluetoothUuid(ServiceClass::OPP));
		if (sock.state() != BluetoothSocket::SocketState::ConnectedState)
		{
			reply->m_errorString = QString("Failed to connect to remote device at address %1. %2")
				.arg(QString(request.address()))
				.arg(sock.errorString());
			reply->m_error = BluetoothTransferReply::TransferError::HostNotFoundError;
			reply->m_finished = true;
			reply->finished(reply);
			emit finished(reply);
		}

		OBEXConnect connectRequest(65535);
		OBEXConnectResponse connectResponse;

		// send the connection request
		try
		{
			sockStream << connectRequest;
			sockStream >> connectResponse;
			if (!connectResponse.isValid())
				throw BluetoothException(QString("Invalid connection response from remote device at address %1. %2")
					.arg(QString(request.address()))
					.arg(sock.errorString()));
		}
		catch (BluetoothException& e)
		{
			reply->m_errorString = e.what();
			reply->m_error = BluetoothTransferReply::TransferError::UnknownError;
			reply->m_finished = true;
			reply->finished(reply);
			emit finished(reply);
		}
		catch (...)
		{
			reply->m_errorString = sock.errorString();
			reply->m_error = BluetoothTransferReply::TransferError::UnknownError;
			reply->m_finished = true;
			reply->finished(reply);
			emit finished(reply);
		}

		// read the data
		bool open = data->open(QIODevice::ReadOnly);
		QByteArray ba = data->readAll();
		if (ba.isEmpty() || !open)
		{
			reply->m_errorString = "IO Device could not be opened for reading or was empty";
			reply->m_error = BluetoothTransferReply::TransferError::IODeviceNotReadableError;
			reply->m_finished = true;
			reply->finished(reply);
			emit finished(reply);
		}

		// write the data to the socket
		try
		{
			OBEXPutResponse putResponse;
			OBEXPut putRequest(connectResponse.maxPacketLength());
			for (const auto& header : request.attributes())
			{
				putRequest.addOptionalHeader(header);
			}
			if (auto[hasAttr, itr] = putRequest.optionalHeaders().contains(OBEXHeader::Length); !hasAttr)
				putRequest.addOptionalHeader(OBEXHeader::Length, (quint32)ba.size());

			while (putRequest.setBody(ba) && putResponse.continueSending() && !reply->m_abort)
			{
				sockStream << putRequest;
				sockStream >> putResponse;
			}
		}
		catch (...)
		{
			reply->m_errorString = sock.errorString();
			reply->m_error = BluetoothTransferReply::TransferError::SessionError;
			reply->m_finished = true;
			reply->finished(reply);
			emit finished(reply);
		}

		// disconnect the session
		try
		{		
			OBEXDisconnect disconnectRequest;
			OBEXDisconnectResponse disconnectResponse;

			sockStream << disconnectRequest;
			sockStream >> disconnectResponse;
		}
		catch (...)
		{
			// if this fails who cares
		}
		
		if (reply->m_abort)
		{
			// .......
		}

		// done!
		reply->m_finished = true;
		reply->finished(reply);
		emit finished(reply);
	});

	return reply;
}

