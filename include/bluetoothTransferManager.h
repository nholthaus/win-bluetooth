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
/// @file	bluetoothTransferManager.h
/// @brief	Class to transfer data to another bluetooth device using the Object Push Protocol (OPP)
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothTransferManager_h__
#define bluetoothTransferManager_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothTransferRequest;
class BluetoothTransferReply;
class BluetoothAddress;
class QIODevice;

//--------------------------------------------------------------------------------------------------
//	BluetoothTransferManager
//--------------------------------------------------------------------------------------------------

class BluetoothTransferManager : public QObject
{
	Q_OBJECT

public:

	BluetoothTransferManager(QObject* parent = nullptr);
	virtual ~BluetoothTransferManager() = default;
	
	QSharedPointer<BluetoothTransferReply> put(const BluetoothTransferRequest& request, QSharedPointer<QIODevice> data);

signals:

	void finished(QSharedPointer<BluetoothTransferReply> reply);

protected:



};

#endif // bluetoothTransferManager_h__
