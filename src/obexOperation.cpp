#include <obexOperation.h>
#include <algorithm>
#include <numeric>
#include <execution>
#include <cassert>
#include <QBuffer>

//--------------------------------------------------------------------------------------------------
//	OBEXOperation () []
//--------------------------------------------------------------------------------------------------
OBEXOperation::OBEXOperation(OBEXOpCode opcode) 
	: m_opcode(opcode)
{

}

//--------------------------------------------------------------------------------------------------
//	opcode (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXOperation::setOpcode(OBEXOpCode val)
{
	m_opcode = val;
}

//--------------------------------------------------------------------------------------------------
//	opcode () []
//--------------------------------------------------------------------------------------------------
OBEXOperation::OBEXOpCode OBEXOperation::opcode() const noexcept
{
	return m_opcode;
}

//--------------------------------------------------------------------------------------------------
//	length (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
quint16 OBEXOperation::length() const noexcept
{
	return BASE_LENGTH + lengthOfRequestData() + lengthOfOptionalHeaders();	
}

//--------------------------------------------------------------------------------------------------
//	requestData (public ) []
//--------------------------------------------------------------------------------------------------
QByteArray OBEXOperation::requestData() const noexcept
{
	return m_requestData;
}

//--------------------------------------------------------------------------------------------------
//	addOptionalHeader (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXOperation::addOptionalHeader(const OBEXHeader& header)
{
	m_optionalHeaders.emplace_back(header);
}

//--------------------------------------------------------------------------------------------------
//	addOptionalHeader (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXOperation::addOptionalHeader(OBEXHeader&& header)
{
	m_optionalHeaders.emplace_back(std::move(header));
}

//--------------------------------------------------------------------------------------------------
//	operator QByteArray (public ) []
//--------------------------------------------------------------------------------------------------
OBEXOperation::operator QByteArray() const
{
	QByteArray ba;
	QDataStream s(&ba, QIODevice::WriteOnly);
	s << *this;
	return ba;
}

//--------------------------------------------------------------------------------------------------
//	setRequestData (protected ) [virtual ]
//--------------------------------------------------------------------------------------------------
void OBEXOperation::setRequestData(const QByteArray& data)
{
	m_requestData = data;
}

//--------------------------------------------------------------------------------------------------
//	setRequestData (protected ) []
//--------------------------------------------------------------------------------------------------
void OBEXOperation::setRequestData(QByteArray&& data)
{
	m_requestData = std::move(data);
}

//--------------------------------------------------------------------------------------------------
//	lengthOfRequestData (protected ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXOperation::lengthOfRequestData() const
{
	return m_requestData.length();
}

//--------------------------------------------------------------------------------------------------
//	lengthOfOptionalHeaders (protected ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXOperation::lengthOfOptionalHeaders() const
{
	static const auto addLength = [](unsigned short s, const OBEXHeader& h) { return s + h.length(); };

	return std::accumulate
	(
		m_optionalHeaders.cbegin(),
		m_optionalHeaders.cend(),
		0,
		addLength
	);
}

//--------------------------------------------------------------------------------------------------
//	operator<< (public ) []
//--------------------------------------------------------------------------------------------------
QDataStream& operator<<(QDataStream &out, const OBEXOperation &op)
{
 	out << (quint8)op.m_opcode;
  	out << op.length();
	for (quint8 c : op.m_requestData)
		out << c;
	for(const auto& header : op.m_optionalHeaders)
		out << header;

	return out;
}

//--------------------------------------------------------------------------------------------------
//	OBEXConnect (public ) []
//--------------------------------------------------------------------------------------------------
OBEXConnect::OBEXConnect(quint16 maxPacketLength /*= 65535*/)
	: OBEXOperation(OBEXOperation::OBEXOpCode::Connect)
{
	QByteArray ba;
	QBuffer buff(&ba);
	QDataStream requestData(&buff);
	buff.open(QBuffer::ReadWrite);

	requestData << OBEX_VERSION;
	requestData << OBEX_FLAGS;
	requestData << maxPacketLength;
	assert(ba.size() == 4);

	this->setRequestData(ba);
}