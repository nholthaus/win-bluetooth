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
/// @file	obexHeader.h
/// @brief	Header definitions for the object exchange protocol. Obex headers use the network byte
///			order.
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef obexHeader_h__
#define obexHeader_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QVariant>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class QString;
class QByteArray;
class QDateTime;

//--------------------------------------------------------------------------------------------------
//	OBEXHeader
//--------------------------------------------------------------------------------------------------
class OBEXHeader
{
	Q_GADGET

public:

	enum HeaderDataType : unsigned char
	{
		UNICODE					= 0x00, ///< null terminated Unicode text, length prefixed with 2 byte unsigned integer
		BINARY					= 0x40, ///< byte sequence, length prefixed with 2 byte unsigned integer
		BYTE					= 0x80, ///< 1 byte quantity
		FOUR_BYTES				= 0xC0, ///< 4 byte quantity – transmitted in network byte order(high byte first)
	};
	Q_ENUM(HeaderDataType);

	enum HeaderIdentifier : unsigned char
	{
		Count					= 0xC0, ///< Number of objects (used by Connect)
		Name					= 0x01, ///< name of the object (often a file name)
		Type					= 0x42, ///< type of object - e.g. text, html, binary, manufacturer specific
		Length					= 0xC3, ///< the length of the object in bytes
		Time					= 0x44, ///< date/time stamp – ISO 8601 version
		Description				= 0x05, ///< text description of the object
		Target					= 0x46, ///< name of service that operation is targeted to
		HTTP					= 0x47, ///< an HTTP 1.x header
		Body					= 0x48, ///< a chunk of the object body
		EndOfBody				= 0x49, ///< the final chunk of the object body
		Who						= 0x4A, ///< identifies the OBEX application, used to tell if talking to a peer
		ConnectionId			= 0xCB, ///< an identifier used for OBEX connection multiplexing
		AppParameters			= 0x4C, ///< extended application request & response information
		AuthChallenge			= 0x4D, ///< authentication digest-challenge
		AuthResponse			= 0x4E, ///< authentication digest-response
		CreatorId				= 0xCF, ///< indicates the creator of an object
		WANUUID					= 0x50, ///< uniquely identifies the network client(OBEX server)
		ObjectClass				= 0x51, ///< OBEX Object class of object
		SessionParameters		= 0x52, ///< Parameters used in session commands/responses
		SessionSequenceNumber	= 0x93, ///< Sequence number used in each OBEX packet for reliability
		UserDefined_0			= 0x30, ///< 
		UserDefined_1			= 0x31, ///< 
		UserDefined_2			= 0x32, ///< 
		UserDefined_3			= 0x33, ///< 
		UserDefined_4			= 0x34, ///< 
		UserDefined_5			= 0x35, ///< 
		UserDefined_6			= 0x36, ///< 
		UserDefined_7			= 0x37, ///< 
		UserDefined_8			= 0x38, ///< 
		UserDefined_9			= 0x39, ///< 
		UserDefined_A			= 0x3A, ///< 
		UserDefined_B			= 0x3B, ///< 
		UserDefined_C			= 0x3C, ///< 
		UserDefined_D			= 0x3D, ///< 
		UserDefined_E			= 0x3E, ///< 
		UserDefined_F			= 0x3F, ///< 
	};
	Q_ENUM(HeaderIdentifier);

public:

	OBEXHeader(HeaderIdentifier id) noexcept;
	OBEXHeader(HeaderIdentifier id, const QString& value);
	OBEXHeader(HeaderIdentifier id, const QByteArray& value);
	OBEXHeader(HeaderIdentifier id, const QDateTime& value);
	OBEXHeader(HeaderIdentifier id, unsigned char value);
	OBEXHeader(HeaderIdentifier id, int value);

	HeaderDataType dataType() const noexcept;
	HeaderIdentifier headerId() const noexcept;
	QVariant value() const noexcept;

	// there is purposely no QVariant interface, so that each setValue function can do a sanity
	// check against the header data type.
	void setValue(const QString& value);
	void setValue(const QByteArray& value);
	void setValue(const QDateTime& value);
	void setValue(unsigned char value);
	void setValue(int value);
	
private:

	HeaderIdentifier	m_headerId;
	QVariant			m_value;

};

#endif // obexHeader_h__
