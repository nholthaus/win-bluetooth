//--------------------------------------------------------------------------------------------------
// 
//	
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
/// @file	bluetoothSocket.h
/// @brief	A Socket/QIODevice wrapper around winsock bluetooth sockets
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothSocket_h__
#define bluetoothSocket_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QIODevice>
#include <QScopedPointer>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothSocketPrivate;

//--------------------------------------------------------------------------------------------------
//	BluetoothSocket
//--------------------------------------------------------------------------------------------------

class BluetoothSocket : public QIODevice
{
	Q_OBJECT

public:

	BluetoothSocket();
	virtual ~BluetoothSocket() = default;
	
	virtual bool isSequential() const override;
	virtual qint64 bytesAvailable() const override;
	virtual bool waitForReadyRead(int msecs) override;
	virtual bool waitForBytesWritten(int msecs) override;
	virtual void close() override;

signals:



protected:

	virtual qint64 readData(char *data, qint64 maxlen) override;
	virtual qint64 writeData(const char *data, qint64 len) override;

private:

	QScopedPointer<BluetoothSocketPrivate>	d_ptr;

};

#endif // bluetoothSocket_h__
