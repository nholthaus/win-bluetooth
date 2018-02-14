//--------------------------------------------------------------------------------------------------
// 
//	WIN-BLUETOOTH
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
/// @file	bluetoothTransferReply.h
/// @brief	Class that stores the response for a data transfer request
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothTransferReply_h__
#define bluetoothTransferReply_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject>
#include <bluetoothTransferManager.h>
#include <bluetoothTransferRequest.h>
#include <atomic>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------


//--------------------------------------------------------------------------------------------------
//	BluetoothTransferReply
//--------------------------------------------------------------------------------------------------

class BluetoothTransferReply : public QObject
{
	Q_OBJECT

public:

	enum TransferError
	{
		NoError						= 0,	///< No error.
		UnknownError				= 1,	///< Unknown error, no better enum available.
		FileNotFoundError			= 2,	///< Unable to open the file specified.
		HostNotFoundError			= 3,	///< Unable to connect to the target host.
		UserCanceledTransferError	= 4,	///< User terminated the transfer.
		IODeviceNotReadableError	= 5,	///< File was not open before initiating the sending command.
		ResourceBusyError			= 6,	///< Unable to access the resource..
		SessionError				= 7,	///< An error occurred during the handling of the session.This enum was introduced by Qt 5.4.
	};
	Q_ENUM(TransferError);

	friend BluetoothTransferManager;

public:

	virtual TransferError error() const;
	virtual QString errorString() const;
	virtual bool isFinished() const;
	virtual bool isRunning() const;
	BluetoothTransferManager* manager() const;
	BluetoothTransferRequest request() const;

public slots:
	
	void abort();

signals:

	void error(BluetoothTransferReply::TransferError errorType);
	void finished(BluetoothTransferReply* reply);
	void transferProgress(qint64 bytesTransfered, qint64 bytesTotal);
	
protected:

	BluetoothTransferReply(QObject* parent = nullptr);
	void setManager(BluetoothTransferManager* manager);
	void setRequest(const BluetoothTransferRequest& request);

private:

	BluetoothTransferManager*	m_manager = nullptr;
	BluetoothTransferRequest	m_request;

	TransferError				m_error = TransferError::NoError;
	QString						m_errorString;
	bool						m_finished = false;
	std::atomic_bool			m_abort = false;
};
#endif // bluetoothTransferReply_h__
