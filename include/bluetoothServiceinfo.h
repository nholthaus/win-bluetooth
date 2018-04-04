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
/// @file	bluetoothServiceinfo.h
/// @brief	class to enable access to the attributes of a Bluetooth service.
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothServiceinfo_h__
#define bluetoothServiceinfo_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QList>
#include <QVariant> 
#include <QObject>
#include <QSharedDataPointer>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothServiceInfoPrivate;
class BluetoothDeviceInfo;

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfo
//--------------------------------------------------------------------------------------------------

class BluetoothServiceInfo
{
	Q_GADGET;

public:

	friend BluetoothServiceInfoPrivate;

	class Alternative : public QList<QVariant>
	{
		Alternative() = default;
		Alternative(const QList<QVariant> &list)
		{
			for (const auto& item : list)
				this->append(item);
		}
	};

	class Sequence : public QList<QVariant>
	{
		Sequence() = default;
		Sequence(const QList<QVariant> &list)
		{
			for (const auto& item : list)
				this->append(item);
		}
	};

public:

	enum AttributeId
	{
		ServiceRecordHandle					= 0x0000,	///< Specifies a service record from which attributes can be retrieved.
		ServiceClassIds						= 0x0001,	///< UUIDs of service classes that the service conforms to.The most common service classes are defined in(QBluetoothUuid::ServiceClassUuid)
		ServiceRecordState					= 0x0002,	///< Attribute changes when any other service attribute is added, deleted or modified.
		ServiceId							= 0x0003,	///< UUID that uniquely identifies the service.
		ProtocolDescriptorList				= 0x0004,	///< List of protocols used by the service.The most common protocol Uuids are defined in QBluetoothUuid::ProtocolUuid
		BrowseGroupList						= 0x0005,	///< List of browse groups the service is in.
		LanguageBaseAttributeIdList			= 0x0006,	///< List of language base attribute IDs to support human - readable attributes.
		ServiceInfoTimeToLive				= 0x0007,	///< Number of seconds for which the service record is expected to remain valid and unchanged.
		ServiceAvailability					= 0x0008,	///< Value indicating the availability of the service.
		BluetoothProfileDescriptorList		= 0x0009,	///< List of profiles to which the service conforms.
		DocumentationUrl					= 0x000A,	///< URL that points to the documentation on the service..
		ClientExecutableUrl					= 0x000B,	///< URL that refers to the location of an application that can be used to utilize the service.
		IconUrl								= 0x000C,	///< URL to the location of the icon representing the service.
		AdditionalProtocolDescriptorList	= 0x000D,	///< Additional protocols used by the service.This attribute extends ProtocolDescriptorList.
		PrimaryLanguageBase					= 0x0100,	///< Base index for primary language text descriptors.
	};
	Q_ENUM(AttributeId);

	enum Protocol
	{
		UnknownProtocol						= 0,		///< The service uses an unknown socket protocol.
		L2capProtocol						= 1,		///< The service uses the L2CAP socket protocol. This protocol is not supported for direct socket connections on Android.
		RfcommProtocol						= 2,		///< The service uses the RFCOMM socket protocol.
	};
	Q_ENUM(Protocol);

public:

	BluetoothServiceInfo();
	virtual ~BluetoothServiceInfo();
	BluetoothServiceInfo(const BluetoothServiceInfo& other);

	QVariant attribute(quint16 attributeId) const;
	QList<quint16> attributes() const;
	bool contains(quint16 attributeId) const;
	BluetoothDeviceInfo device() const;

	void setAttribute(quint16 attributeId, const QVariant& value);

	void setDevice(const BluetoothDeviceInfo& device);

private:

	QSharedDataPointer<BluetoothServiceInfoPrivate>	d;

};

Q_DECLARE_TYPEINFO(BluetoothServiceInfo, Q_MOVABLE_TYPE);

#endif // bluetoothServiceinfo_h__
