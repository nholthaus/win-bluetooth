#include <obexResponse.h>
#include <QDataStream>
#include <memory>

//--------------------------------------------------------------------------------------------------
//	operator>> (public ) []
//--------------------------------------------------------------------------------------------------
QDataStream& operator>>(QDataStream& in, OBEXResponse& response)
{
	auto span = response.data();

	// read in the base packet
	in.readRawData(span.begin(), span.length());

	// read in the optional headers
	size_t amountLeft = response.packetLength() - span.length();

	// if there's anything left, it's optional headers
	if(amountLeft)
	{
		auto buffer = new char[amountLeft];		// don't delete, it's going into a QByteArray
		in.readRawData(buffer, amountLeft);

		// parse the optional headers
		response.m_optionalHeaders = OBEXHeader::fromByteArray(QByteArray::fromRawData(buffer, amountLeft));
	}

	return in;
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
//	Code (public ) []
//--------------------------------------------------------------------------------------------------
OBEXResponse::Code::Code(quint8 value)
	: m_code((Code::Enum)(value & 0x7F))	// clear the MSB
	, m_isFinal(value & 0x80)			// only look at the MSB
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