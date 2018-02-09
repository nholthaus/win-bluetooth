#include <obexResponse.h>
#include <QDataStream>
#include <memory>
#include <bluetoothException.h>
#include <QtEndian>

// because of the way the data is streamed, this macro actually converts the input to 
// little endian
#define LE(var) var = qToBigEndian(var);

//--------------------------------------------------------------------------------------------------
//	operator>> (public ) []
//--------------------------------------------------------------------------------------------------
QDataStream& operator>>(QDataStream& in, OBEXResponse& response)
{
	auto span = response.data();

	// read in the base packet
	in.readRawData(span.begin(), static_cast<int>(span.length()));

	response.m_valid = response.validateAndFixup();

	// read in the optional headers
	auto amountLeft = response.packetLength() - span.length();

	// if there's anything left, it's optional headers
	if(amountLeft)
	{
		auto buffer = new char[amountLeft];		// don't delete, it's going into a QByteArray
		in.readRawData(buffer, static_cast<int>(amountLeft));

		// parse the optional headers
		response.m_optionalHeaders = OBEXHeader::fromByteArray(QByteArray::fromRawData(buffer, static_cast<int>(amountLeft)));
	}

	return in;
}

//--------------------------------------------------------------------------------------------------
//	isValid (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXResponse::isValid() const
{
	return m_valid;
}

//--------------------------------------------------------------------------------------------------
//	Code (public ) []
//--------------------------------------------------------------------------------------------------
OBEXResponse::Code::Code(quint8 value)
	: m_code((Code::Enum)(value & 0x7F))	// clear the MSB
	, m_isFinal(value & 0x80)				// only look at the MSB
{

}

//--------------------------------------------------------------------------------------------------
//	operator quint8 () []
//--------------------------------------------------------------------------------------------------
OBEXResponse::Code::operator quint8() const
{
	return static_cast<quint8>(m_code);
}

//--------------------------------------------------------------------------------------------------
//	isFinal () []
//--------------------------------------------------------------------------------------------------
bool OBEXResponse::Code::isFinal() const
{
	return m_isFinal;
}

//--------------------------------------------------------------------------------------------------
//	operator!= (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXResponse::Code::operator!=(const Code& other) const
{
	return !(*this == other);
}

//--------------------------------------------------------------------------------------------------
//	operator== (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXResponse::Code::operator==(const Code& other) const
{
	// disregard the MSB
	return (((quint8)m_code & 0x7F) == ((quint8)other.m_code & 0x7F));
}

//--------------------------------------------------------------------------------------------------
//	data (public ) []
//--------------------------------------------------------------------------------------------------
gsl::string_span OBEXConnectResponse::data()
{
	return gsl::string_span(reinterpret_cast<char*>(&m_data), sizeof(Data));
}

//--------------------------------------------------------------------------------------------------
//	packetLength (public ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXConnectResponse::packetLength()
{
	return m_data.length;
}

//--------------------------------------------------------------------------------------------------
//	validateAndFixup (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXConnectResponse::validateAndFixup()
{
	LE(m_data.maxPacketLength);
	LE(m_data.length);

	return (m_data.code == Code::Enum::SUCCESS);
}

//--------------------------------------------------------------------------------------------------
//	maxPacketLength (public ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXConnectResponse::maxPacketLength() const
{
	return m_data.maxPacketLength;
}

//--------------------------------------------------------------------------------------------------
//	data (public ) []
//--------------------------------------------------------------------------------------------------
gsl::string_span OBEXPutResponse::data()
{
	return gsl::string_span(reinterpret_cast<char*>(&m_data), sizeof(Data));
}

//--------------------------------------------------------------------------------------------------
//	packetLength (public ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXPutResponse::packetLength()
{
	return m_data.length;
}

//--------------------------------------------------------------------------------------------------
//	validateAndFixup (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXPutResponse::validateAndFixup()
{
	LE(m_data.length);

	return (m_data.code == Code::Enum::SUCCESS || m_data.code == Code::Enum::CONTINUE);
}

//--------------------------------------------------------------------------------------------------
//	continueSending (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXPutResponse::continueSending() const
{
	return m_data.code == OBEXResponse::Code::Enum::CONTINUE;
}

//--------------------------------------------------------------------------------------------------
//	data (public ) []
//--------------------------------------------------------------------------------------------------
gsl::string_span OBEXDisconnectResponse::data()
{
	return gsl::string_span(reinterpret_cast<char*>(&m_data), sizeof(Data));
}

//--------------------------------------------------------------------------------------------------
//	packetLength (public ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXDisconnectResponse::packetLength()
{
	return m_data.length;
}

//--------------------------------------------------------------------------------------------------
//	validateAndFixup (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXDisconnectResponse::validateAndFixup()
{
	LE(m_data.length);

	return (m_data.code == OBEXResponse::Code::Enum::SUCCESS || m_data.code == OBEXResponse::Code::Enum::SERVICEUNAVAILABLE);
}
