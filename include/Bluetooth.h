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
/// @file	
/// @brief	
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetooth_h__
#define bluetooth_h__


//-------------------------
//	INCLUDES
//-------------------------

#include <vector>

#include <bluetoothRadio.h>
#include <bluetoothDevice.h>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------


//--------------------------------------------------------------------------------------------------
//	CLASS BLUETOOTH
//--------------------------------------------------------------------------------------------------
class Bluetooth
{
public:

	BluetoothRadio&						localRadio(unsigned int index = 0);
	BluetoothRadio&						localRadio(const std::wstring_view& name, bool refreshList = false);
	const BluetoothRadio&				localRadio(unsigned int index = 0) const;
	const BluetoothRadio&				localRadio(const std::wstring_view& name, bool refreshList = false) const;
	std::vector<BluetoothRadio>&		localRadios(bool refreshList = false);
	const std::vector<BluetoothRadio>&	localRadios(bool refreshList = false) const;

	BluetoothDevice&					remoteDevice(unsigned int index = 0);
	BluetoothDevice&					remoteDevice(const std::wstring_view& name, bool refreshList = false);
	const BluetoothDevice&				remoteDevice(unsigned int index = 0) const;
	const BluetoothDevice&				remoteDevice(const std::wstring_view& name, bool refreshList = false) const;
	std::vector<BluetoothDevice>&		remoteDevices(bool refreshList = false);
	const std::vector<BluetoothDevice>& remoteDevices(bool refreshList = false) const;

protected:

	bool enumerateLocalRadios(bool refreshList = false) const;
	bool enumerateRemoteDevices(bool refreshList = false) const;

private:

	mutable std::vector<BluetoothRadio> m_localRadios;
	mutable std::vector<BluetoothDevice> m_remoteDevices;
	BluetoothRadio m_invalidRadio;
	BluetoothDevice m_invalidDevice;
};


#endif // bluetooth_h__
