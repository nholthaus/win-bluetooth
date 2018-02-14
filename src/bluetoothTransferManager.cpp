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
BluetoothTransferReply* BluetoothTransferManager::put(const BluetoothTransferRequest& request, QSharedPointer<QIODevice> data)
{
	auto* reply  = new BluetoothTransferReply;
	reply->setManager(this);
	reply->setRequest(request);

	// run the put operation in a thread because it blocks
	QtConcurrent::run([=]()
	{
		BluetoothSocket sock;
		QDataStream sockStream(&sock);

		// done lambda
		auto done = [=]()
		{
			reply->m_finished = true;
			reply->finished(reply);
			emit finished(reply);
		};

		// error lambda
		auto setError = [=](QString errorString, BluetoothTransferReply::TransferError error)
		{
			reply->m_errorString = errorString;
			reply->m_error = error;
			reply->error(reply->m_error);
			done();
		};

		// connect socket
		sock.connectToService(request.address(), BluetoothUuid(ServiceClass::OPP));
		if (sock.state() != BluetoothSocket::SocketState::ConnectedState)
		{
			setError(QString("Failed to connect to remote device at address %1. %2")
				.arg(QString(request.address()))
				.arg(sock.errorString()),
				BluetoothTransferReply::TransferError::HostNotFoundError);
			return reply;
		}

		OBEXConnect connectRequest;
		OBEXConnectResponse connectResponse;

		// send the connection request
		try
		{
			sockStream << connectRequest;
			if (!sock.waitForReadyRead(READ_TIMEOUT_MS))
				throw BluetoothException("Connection timed out.");
			sockStream >> connectResponse;
			if (!connectResponse.isValid())
				throw BluetoothException(QString("Invalid connection response from remote device at address %1. %2")
					.arg(QString(request.address()))
					.arg(sock.errorString()));
		}
		catch (BluetoothException& e)
		{
			setError(e.what(), BluetoothTransferReply::TransferError::UnknownError);
		}
		catch (...)
		{
			setError(sock.errorString(), BluetoothTransferReply::TransferError::UnknownError);
		}

		// read the data
		bool open = data->open(QIODevice::ReadOnly);
		QByteArray ba = data->readAll();
		if (ba.isEmpty() || !open)
			setError("IO Device could not be opened for reading or was empty", BluetoothTransferReply::TransferError::IODeviceNotReadableError);

		// write the data to the socket
		try
		{
			OBEXPutResponse putResponse;
			OBEXPut putRequest(connectResponse.maxPacketLength());
			quint32 length = (quint32)ba.size();
			for (const auto& header : request.attributes())
			{
				putRequest.addOptionalHeader(header);
			}
			if (auto[hasAttr, itr] = putRequest.optionalHeaders().contains(OBEXHeader::Length); !hasAttr)
				putRequest.addOptionalHeader(OBEXHeader::Length, length);

			while (putRequest.setBody(ba) && putResponse.continueSending() && !reply->m_abort)
			{
				sockStream << putRequest;
				if (!sock.waitForReadyRead(READ_TIMEOUT_MS))
					throw BluetoothException("Bluetooth response timed out.");
				sockStream >> putResponse;
				reply->transferProgress(length - ba.size(), length);
			}
		}
		catch (...)
		{
			setError(sock.errorString(), BluetoothTransferReply::TransferError::SessionError);
		}

		// disconnect the session
		try
		{		
			OBEXDisconnect disconnectRequest;
			OBEXDisconnectResponse disconnectResponse;

			sockStream << disconnectRequest;
			if (!sock.waitForReadyRead(READ_TIMEOUT_MS))
				throw BluetoothException("Disconnection timed out.");
			sockStream >> disconnectResponse;
		}
		catch (...)
		{
			// if this fails who cares we're destroying the socket anyway
		}
		
		if (reply->m_abort)
		{
			setError("User canceled transfer", BluetoothTransferReply::TransferError::UserCanceledTransferError);
		}

		// done!
		done();
	});

	return reply;
}

