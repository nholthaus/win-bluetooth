#include <obexHeader.h>
#include <bluetoothException.h>
#include <QDateTime>
#include <QDataStream>
#include <QtEndian>
#include <cassert>

//--------------------------------------------------------------------------------------------------
//	OBEXHeader (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id) noexcept
	: m_headerId(id)
{

}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader () []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, const QString& value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader () []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, const QByteArray& value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader () []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, const QDateTime& value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader () []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, unsigned char value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader () []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, int value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	dataType (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader::HeaderDataType OBEXHeader::dataType() const noexcept
{
	return HeaderDataType((unsigned char)0xC0 & m_headerId);
}

//--------------------------------------------------------------------------------------------------
//	headerId (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader::HeaderIdentifier OBEXHeader::headerId() const noexcept
{
	return m_headerId;
}

//--------------------------------------------------------------------------------------------------
//	value (public ) []
//--------------------------------------------------------------------------------------------------
QVariant OBEXHeader::value() const noexcept
{
	return m_value;
}

//--------------------------------------------------------------------------------------------------
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(const QString& value)
{
	if (dataType() == UNICODE)
		m_value = value;
	else
		throw BluetoothException("OBEX value type does not match header id. This is an error on the part of the programmer.");
}

//--------------------------------------------------------------------------------------------------
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(const QByteArray& value)
{
	if (dataType() == BINARY)
		m_value = value;
	else
		throw BluetoothException("OBEX value type does not match header id. This is an error on the part of the programmer.");
}

//--------------------------------------------------------------------------------------------------
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(unsigned char value)
{
	if (dataType() == BYTE)
		m_value = value;
	else
		throw BluetoothException("OBEX value type does not match header id. This is an error on the part of the programmer.");
}

//--------------------------------------------------------------------------------------------------
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(int value)
{
	if (dataType() == FOUR_BYTES)
		m_value = value;
	else
		throw BluetoothException("OBEX value type does not match header id. This is an error on the part of the programmer.");
}

//--------------------------------------------------------------------------------------------------
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(const QDateTime& value)
{
	if (headerId() == Time)
		m_value = value.toUTC().toString(Qt::ISODate);
	else
		throw BluetoothException("OBEX value type does not match header id. This is an error on the part of the programmer.");
}

//--------------------------------------------------------------------------------------------------
//	operator<< (public ) []
//--------------------------------------------------------------------------------------------------
QDataStream& operator<<(QDataStream &out, const OBEXHeader &header)
{
	// per the standard, OBEX packet data has to be in network byte order (big-endian)
	// QDataStream uses big-endian by default. Let's not mess with the defaults and
	// everything will be fine. http://doc.qt.io/qt-5/qdatastream.html#setByteOrder
	assert(out.byteOrder() == QDataStream::BigEndian);

	[[maybe_unused]] QString str;
	[[maybe_unused]] QByteArray ba;

	out << header.headerId();

	switch (header.dataType())
	{
	case OBEXHeader::UNICODE:

		static_assert(sizeof(quint16) == sizeof(QChar), "Size mismatch");

		if (!header.value().canConvert<QString>())
			throw BluetoothException("Cannot convert OBEX header value to string");	// this should be caught in `setValue`, but just in case...

		str = header.value().toString();
		out << ((quint16)str.length() + sizeof(QChar));
		out << str.utf16();

		break;

	case OBEXHeader::BINARY:
		if (!header.value().canConvert<QByteArray>())
			throw BluetoothException("Cannot convert OBEX header value to byte array");	// this should be caught in `setValue`, but just in case...

		ba = header.value().toByteArray();
		out << (quint16)ba.length();
		out << ba;

		break;

	case OBEXHeader::BYTE:
		if (!header.value().canConvert<unsigned char>())
			throw BluetoothException("Cannot convert OBEX header value to byte");	// this should be caught in `setValue`, but just in case...

		out << header.value().toChar();

		break;

	case OBEXHeader::FOUR_BYTES:

		static_assert(sizeof(unsigned int) == sizeof(quint32), "Size mismatch");

		if (!header.value().canConvert<quint32>())
			throw BluetoothException("Cannot convert OBEX header value to unsigned int");	// this should be caught in `setValue`, but just in case...

		out << header.value().toUInt();

		break;

	default:
		throw BluetoothException("Unknown OBEX data type");	// this should never happen
		break;
	}
	return out;
}
