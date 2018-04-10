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
/// @file	bluetoothEnums.h
/// @brief	
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothEnums_h__
#define bluetoothEnums_h__

//------------------------------
//	INCLUDES
//------------------------------

#include <QFlags>

//------------------------------
//	ENUMS
//------------------------------

// make sure the protocol enums match this list: 
// https://www.bluetooth.com/specifications/assigned-numbers/service-discovery
enum class Protocol
{
	SDP			= 0x0001,
	RFCOMM		= 0x0003,
	OBEX		= 0x0008,
	L2CAP		= 0x0100,
};

enum class ServiceClass
{
	OPP			= 0x1105,
	FTP			= 0x1106,
	SerialPort	= 0x1101,
	MSDNBluetoothConnectionExample = 0xFFFF,
};

enum Security
{
	NoSecurity			= 0x00,
	Authorization		= 0x01,
	Authentication		= 0x02,
	Encryption			= 0x04,
	Secure				= 0x08,
};
Q_DECLARE_FLAGS(SecurityFlags, Security);

#endif // bluetoothEnums_h__
