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
/// @file		obexOperation.h
/// @brief		Class to describe an OBEX operation per chapter 3.3 of the standard
/// @details	includes derived classes for:
///				- connection
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef obexOperation_h__
#define obexOperation_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject>
#include <QDataStream>
#include <obexHeader.h>
#include <vector>

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------


//--------------------------------------------------------------------------------------------------
//	OBEXOperation
//--------------------------------------------------------------------------------------------------
class OBEXOperation
{
	Q_GADGET

public:

	// Enums 0x10 to 0x1F are user definable if need be
	enum class OBEXOpCode : quint8
	{
		Connect			= 0x80,
		Disconnect		= 0x81,
		Put				= 0x02,
		Get				= 0x03,
		SetPath			= 0x85,
		Session			= 0x87,
		Abort			= 0xFF,
	};
	Q_ENUM(OBEXOpCode);

	friend QDataStream& operator<<(QDataStream &out, const OBEXOperation &op);

public:

	OBEXOperation(OBEXOpCode opcode);
	virtual ~OBEXOperation() = default;
	
	OBEXOperation::OBEXOpCode opcode() const noexcept;
	void setOpcode(OBEXOperation::OBEXOpCode val);

	quint16 length() const noexcept;
	QByteArray requestData() const noexcept;

	// prefer the templated version, which is able to construct the headers inplace
	template<class... Args>
	void addOptionalHeader(Args... args)
	{
		m_optionalHeaders.emplace_back(std::forward<Args>(args)...);
	}

	void addOptionalHeader(const OBEXHeader& header);
	void addOptionalHeader(OBEXHeader&& header);

	operator QByteArray() const;

protected:

	// if derived classes require more than just an opcode, length, and headers,
	// us this function to set the additional data
	void setRequestData(const QByteArray& data);
	void setRequestData(QByteArray&& data);

	quint16 lengthOfRequestData() const;
	quint16 lengthOfOptionalHeaders() const;

private:

	OBEXOpCode					m_opcode;
	QByteArray					m_requestData;
	std::vector<OBEXHeader>		m_optionalHeaders;

private:

	// CONSTANTS
	static constexpr unsigned short SIZE_OF_OPCODE = sizeof(unsigned char);
	static constexpr unsigned short SIZE_OF_PACKET_LENGTH = sizeof(unsigned short);
	static constexpr unsigned short BASE_LENGTH = SIZE_OF_OPCODE + SIZE_OF_PACKET_LENGTH;

};

QDataStream& operator<<(QDataStream &out, const OBEXOperation &op);

//--------------------------------------------------------------------------------------------------
//	OBEXConnect
//--------------------------------------------------------------------------------------------------

class OBEXConnect : public OBEXOperation
{
public:

	OBEXConnect(quint16 maxPacketLength = 65535);	// default is max permitted by the standard
	virtual ~OBEXConnect() = default;

protected:

	static constexpr quint8 OBEX_VERSION	= 0x10;
	static constexpr quint8 OBEX_FLAGS		= 0x00;
};

#endif // obexOperation_h__