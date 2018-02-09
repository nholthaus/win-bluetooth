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
// https://github.com/martinmoene/gsl-lite/releases
//
//--------------------------------------------------------------------------------------------------
//
/// @file	obexResponse.h
/// @brief	OBEX server response classes
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef obexResponse_h__
#define obexResponse_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject>
#include <gsl-lite.h> 
#include <obexHeader.h>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class QDataStream;

//--------------------------------------------------------------------------------------------------
//	OBEXResponse
//--------------------------------------------------------------------------------------------------
class OBEXResponse
{
	Q_GADGET

public:

	friend QDataStream& operator>>(QDataStream& in, OBEXResponse& response);

	// OBEX operation response codes. See OBEX standard chapter 3.2.1
	class Code
	{
	public:

		enum class Enum : quint8
		{
			INVALID					= 0x00,
			CONTINUE				= 0x90,
			SUCCESS					= 0xA0,
			SERVICEUNAVAILABLE		= 0xD3,
		};

		Code(quint8 value);
		operator quint8() const;
		bool isFinal() const;
		bool operator==(const Code& other) const;
		bool operator!=(const Code& other) const;

	private:

		Code::Enum	m_code;
		bool		m_isFinal = false;
	};	

public:

	OBEXResponse() = default;
	virtual ~OBEXResponse() = default;
	virtual gsl::string_span data() = 0;	// returns a span representing the response-specific data storage
	virtual quint16 packetLength() = 0;		// packet length value from the response data
	virtual bool validateAndFixup() = 0;	// called at the end of operator>>. Used to validate code fields, and convert
											// large types from big to little endian. Returns true on success.
	bool isValid() const;

protected:

	bool m_valid = false;
	std::vector<OBEXHeader>	m_optionalHeaders;
};

QDataStream& operator>>(QDataStream& in, OBEXResponse& response);

//--------------------------------------------------------------------------------------------------
//	OBEXConnectResponse
//--------------------------------------------------------------------------------------------------

class OBEXConnectResponse : public OBEXResponse
{
public:

	OBEXConnectResponse() = default;
	virtual ~OBEXConnectResponse() = default;
	virtual gsl::string_span data() override;
	virtual quint16 packetLength() override;	// the return value will generally not be valid until after data has been streamed into the class.
	virtual bool validateAndFixup() override;
	quint16 maxPacketLength() const;

private:

#pragma pack(push, 1)
	struct Data
	{
		OBEXResponse::Code::Enum    code			= Code::Enum::INVALID;
		quint16						length			= 0;
		quint8						version			= 0;
		quint8						flags			= 0;
		quint16						maxPacketLength = 0;
	} m_data;
#pragma pack(pop)
};

//--------------------------------------------------------------------------------------------------
//	OBEXDisconnectResponse
//--------------------------------------------------------------------------------------------------

class OBEXDisconnectResponse : public OBEXResponse
{
public:

	OBEXDisconnectResponse() = default;
	virtual ~OBEXDisconnectResponse() = default;
	
	virtual gsl::string_span data() override;
	virtual quint16 packetLength() override;
	virtual bool validateAndFixup() override;

#pragma pack(push, 1)
	struct Data
	{
		OBEXResponse::Code::Enum    code = Code::Enum::SUCCESS;
		quint16						length = 0;
	} m_data;
#pragma pack(pop)
};

//--------------------------------------------------------------------------------------------------
//	OBEXPutResponse
//--------------------------------------------------------------------------------------------------

class OBEXPutResponse : public OBEXResponse
{
public:

	OBEXPutResponse() = default;
	virtual ~OBEXPutResponse() = default;
	
	virtual gsl::string_span data() override;
	virtual quint16 packetLength() override;
	virtual bool validateAndFixup() override;
	bool continueSending() const;

protected:

#pragma pack(push, 1)
	struct Data
	{
		OBEXResponse::Code::Enum    code = Code::Enum::CONTINUE;	// assume success until proven otherwise
		quint16						length = 0;
	} m_data;
#pragma pack(pop)

};
#endif // obexResponse_h__
