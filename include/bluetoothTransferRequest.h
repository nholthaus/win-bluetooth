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
/// @file	bluetoothTransferRequest.h
/// @brief	Class that stores information about a bluetooth data transfer request
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothTransferRequest_h__
#define bluetoothTransferRequest_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <BluetoothAddress.h> 
#include <obexOptionalHeaders.h>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------



//--------------------------------------------------------------------------------------------------
//	BluetoothTransferRequest
//--------------------------------------------------------------------------------------------------

class BluetoothTransferRequest
{
	Q_GADGET

public:

	enum class Attribute
	{
		DescriptionAttribute	= 0,	///< A textual description of the object being transferred.May be displayed in the UI of the remote device.
		TimeAttribute			= 1,	///< Time attribute of the object being transferred.
		TypeAttribute			= 2,	///< MIME type of the object being transferred.
		LengthAttribute			= 3,	///< Length in bytes of the object being transferred.
		NameAttribute			= 4,	///< Name of the object being transferred.May be displayed in the UI of the remote device.
	};
	Q_ENUM(Attribute);

public:

	BluetoothTransferRequest(const BluetoothAddress& address = BluetoothAddress());
	virtual ~BluetoothTransferRequest() = default;
	
	BluetoothAddress address() const;
	QVariant attribute(Attribute code, const QVariant& defaultValue = QVariant()) const;
	const OBEXOptionalHeaders& attributes() const;
	void setAttribute(Attribute code, const QVariant& value);
	bool operator!=(const BluetoothTransferRequest& other) const;
	bool operator==(const BluetoothTransferRequest& other) const;
	
protected:

	BluetoothAddress	m_address;
	OBEXOptionalHeaders	m_attributes;

};

#endif // bluetoothTransferRequest_h__
