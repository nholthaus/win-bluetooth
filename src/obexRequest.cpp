#include <obexRequest.h>
#include <algorithm>
#include <numeric>
#include <execution>
#include <cassert>
#include <QBuffer>
#include <QDebug>

//--------------------------------------------------------------------------------------------------
//	OBEXOperation () []
//--------------------------------------------------------------------------------------------------
OBEXRequest::OBEXRequest(OBEXOpCode opcode) 
	: m_opcode(opcode)
{

}

//--------------------------------------------------------------------------------------------------
//	opcode (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXRequest::setOpcode(OBEXOpCode val)
{
	m_opcode = val;
}

//--------------------------------------------------------------------------------------------------
//	setFinal (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXRequest::setFinal()
{
	m_opcode = (OBEXOpCode)(0x80 | (quint8)m_opcode);	// set the 'final' bit
}

//--------------------------------------------------------------------------------------------------
//	opcode () []
//--------------------------------------------------------------------------------------------------
OBEXRequest::OBEXOpCode OBEXRequest::opcode() const noexcept
{
	return m_opcode;
}

//--------------------------------------------------------------------------------------------------
//	length (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
quint16 OBEXRequest::length() const noexcept
{
	return BASE_LENGTH + lengthOfRequestData() + lengthOfOptionalHeaders();	
}

//--------------------------------------------------------------------------------------------------
//	requestData (public ) []
//--------------------------------------------------------------------------------------------------
QByteArray OBEXRequest::requestData() const noexcept
{
	return m_requestData;
}

//--------------------------------------------------------------------------------------------------
//	addOptionalHeader (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXRequest::addOptionalHeader(const OBEXHeader& header)
{
	m_optionalHeaders.emplace_back(header);
}

//--------------------------------------------------------------------------------------------------
//	addOptionalHeader (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXRequest::addOptionalHeader(OBEXHeader&& header)
{
	m_optionalHeaders.emplace_back(std::move(header));
}

//--------------------------------------------------------------------------------------------------
//	optionalHeaders (public ) []
//--------------------------------------------------------------------------------------------------
const OBEXOptionalHeaders& OBEXRequest::optionalHeaders() const
{
	return m_optionalHeaders;
}

//--------------------------------------------------------------------------------------------------
//	operator QByteArray (public ) []
//--------------------------------------------------------------------------------------------------
OBEXRequest::operator QByteArray() const
{
	QByteArray ba;
	QDataStream s(&ba, QIODevice::WriteOnly);
	s << *this;
	return ba;
}

//--------------------------------------------------------------------------------------------------
//	setRequestData (protected ) [virtual ]
//--------------------------------------------------------------------------------------------------
void OBEXRequest::setRequestData(const QByteArray& data)
{
	m_requestData = data;
}

//--------------------------------------------------------------------------------------------------
//	setRequestData (protected ) []
//--------------------------------------------------------------------------------------------------
void OBEXRequest::setRequestData(QByteArray&& data)
{
	m_requestData = std::move(data);
}

//--------------------------------------------------------------------------------------------------
//	lengthOfRequestData (protected ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXRequest::lengthOfRequestData() const
{
	return m_requestData.length();
}

//--------------------------------------------------------------------------------------------------
//	lengthOfOptionalHeaders (protected ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXRequest::lengthOfOptionalHeaders() const
{
	static const auto addLength = [](unsigned short s, const OBEXHeader& h) { return s + h.length(); };

	quint16 len = std::accumulate
	(
		m_optionalHeaders.cbegin(),
		m_optionalHeaders.cend(),
		0,
		addLength
	);

	return len;
}

//--------------------------------------------------------------------------------------------------
//	operator<< (public ) []
//--------------------------------------------------------------------------------------------------
QDataStream& operator<<(QDataStream &out, const OBEXRequest &op)
{
	// don't stream the parameters 1 at a time because they need to be written to the socket as a packet
	QByteArray ba;
	QDataStream s(&ba, QIODevice::WriteOnly);

	s << (quint8)op.m_opcode;
	s << op.length();
	for (quint8 c : op.m_requestData)
		s << c;
	for(const auto& header : op.m_optionalHeaders)
		s << header;

	out.writeRawData(ba.constData(), ba.length());

	return out;
}

//--------------------------------------------------------------------------------------------------
//	OBEXConnect (public ) []
//--------------------------------------------------------------------------------------------------
OBEXConnect::OBEXConnect(quint16 maxPacketLength /*= 65535*/)
	: OBEXRequest(OBEXRequest::OBEXOpCode::Connect)
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

//--------------------------------------------------------------------------------------------------
//	OBEXPut (public ) []
//--------------------------------------------------------------------------------------------------
OBEXPut::OBEXPut(quint16 maxPacketLength)
	: OBEXRequest(OBEXRequest::OBEXOpCode::Put)
	, m_maxPacketLength(maxPacketLength)
{
	// no request specific data
}

//--------------------------------------------------------------------------------------------------
//	setBody (public ) []
//--------------------------------------------------------------------------------------------------
quint64 OBEXPut::setBody(QByteArray& data)
{
	if (data.isEmpty())
		return 0;

	if (auto[hasBody, itr] = m_optionalHeaders.contains(OBEXHeader::Body); hasBody)
		m_optionalHeaders.erase(itr);

	quint16 maxData = m_maxPacketLength - this->length() - BASE_BODY_HEADER_SIZE;
	quint64 length = data.length();

	// last one
	if (data.size() < maxData)
	{
		setFinal();
		addOptionalHeader(OBEXHeader::EndOfBody, data);
		data.clear();
	}
	else
	{
		addOptionalHeader(OBEXHeader::Body, data.left(maxData));
		data.remove(0, maxData);
	}

	return length;
}
