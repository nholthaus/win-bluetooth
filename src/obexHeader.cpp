#include <obexHeader.h>
#include <bluetoothException.h>
#include <QDateTime>
#include <QDataStream>
#include <QtEndian>
#include <cassert>
#include <QDebug>
#include <stdlib.h>

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
OBEXHeader::OBEXHeader(HeaderIdentifier id, char value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader () []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, unsigned int value)
	: m_headerId(id)
{
	setValue(value);
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, const char* value)
	: m_headerId(id)
{
	setValue(QString(value));
}

//--------------------------------------------------------------------------------------------------
//	OBEXHeader (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader::OBEXHeader(HeaderIdentifier id, const char* value, int length)
	: m_headerId(id)
{
	setValue(value, length);
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
//	length (public ) []
//--------------------------------------------------------------------------------------------------
quint16 OBEXHeader::length() const noexcept
{
	quint16 length = 0;

	switch (dataType())
	{
	case OBEXHeader::UNICODE:
		length = 7 + 2 * m_value.toString().length();	// 1 for id + 2 for length prefix + 2 for null terminator + 2 for the byte order mark + size of string (x2 for unicode) 
		break;
	case OBEXHeader::BINARY:
		length = 3 + m_value.toByteArray().length();	// 1 for id + 2 for size prefix + length of byte array
		break;
	case OBEXHeader::BYTE:
		length = 2;										// 1 + 1 for the id
		break;
	case OBEXHeader::FOUR_BYTES:
		length = 5;										// 4 + 1 for the id
		break;
	}
	
	return length;
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
void OBEXHeader::setValue(char value)
{
	if (dataType() == BYTE)
		m_value = value;
	else
		throw BluetoothException("OBEX value type does not match header id. This is an error on the part of the programmer.");
}

//--------------------------------------------------------------------------------------------------
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(unsigned int value)
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
//	setValue (public ) []
//--------------------------------------------------------------------------------------------------
void OBEXHeader::setValue(const char* value, int length)
{
	if (length)
		setValue(QByteArray::fromRawData(value, length));
}

//--------------------------------------------------------------------------------------------------
//	operator!= (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXHeader::operator!=(const OBEXHeader& other) const
{
	return !(*this == other);
}

//--------------------------------------------------------------------------------------------------
//	operator== (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXHeader::operator==(const OBEXHeader& other) const
{
	return (this->m_headerId == other.m_headerId);
}

//--------------------------------------------------------------------------------------------------
//	operator< (public ) []
//--------------------------------------------------------------------------------------------------
bool OBEXHeader::operator<(const OBEXHeader& other) const
{
	return (this->m_headerId < other.m_headerId);
}

//--------------------------------------------------------------------------------------------------
//	operator+ (public ) []
//--------------------------------------------------------------------------------------------------
unsigned short OBEXHeader::operator+(const OBEXHeader& other)
{
	return length() + other.length();
}

//--------------------------------------------------------------------------------------------------
//	operator QByteArray (public ) []
//--------------------------------------------------------------------------------------------------
OBEXHeader::operator QByteArray() const
{
	QByteArray ba;
	QDataStream s(&ba, QIODevice::WriteOnly);
	s << *this;
	return ba;
}

//--------------------------------------------------------------------------------------------------
//	fromByteArray (public ) [static ]
//--------------------------------------------------------------------------------------------------
std::vector<OBEXHeader> OBEXHeader::fromByteArray(const QByteArray& data)
{
	std::vector<OBEXHeader> optionalHeaders;

	int i = 0;
	quint16 index = 0;
	quint16 length = 0;
	unsigned int val = 0;
	ushort ch;
	QString str;

	while (index < data.size())	// TODO: add some time of timeout or safety to this loop
	{
		HeaderIdentifier id = static_cast<HeaderIdentifier>(data.at(index++));	// if you think this doesn't work, lookup the difference between i++ and ++i.
		HeaderDataType type = (HeaderDataType)(DATA_TYPE_MASK & id);
		switch (type)
		{
		case BYTE:
			optionalHeaders.emplace_back(id, data.at(index++));
			break;
		case FOUR_BYTES:
			// convert to little endian numbers
			for (i = 0; i < sizeof(int); ++i)
			{
				val <<= 8;
				val |= (unsigned char)data[index++];	// the case is necessary to prevent sign extension
			}
			optionalHeaders.emplace_back(id, val);
			break;
		case BINARY:
			// convert to little endian numbers
			for (i = 0; i < sizeof(length); ++i)
			{
				length <<= 8;
				length |= data[index++];
			}
			length -= 3;	// accounts for the id byte and 2 length bytes
			optionalHeaders.emplace_back(id, (char*)&data.data()[index], length);
			index += length;
			break;
		case UNICODE:
			// convert to little endian numbers
			for (i = 0; i < sizeof(length); ++i)
			{
				length <<= 8;
				length |= data[index++];
			}
			length -= 3;	// accounts for the id byte and 2 length bytes
			str = QString::fromUtf16(reinterpret_cast<const unsigned short*>(&data.data()[index]), length  / 2 - 1);	// divide by two because length is in bytes, unicode chars are 2 bytes. Also strip the null terminator, qstring will add one
			optionalHeaders.emplace_back(id, str);
			index += length;
			break;
		}
	}

	return optionalHeaders;
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
	[[maybe_unused]] quint16 length;
	[[maybe_unused]] QVector<ushort> buff;
	[[maybe_unused]] int index = 0;
	[[maybe_unused]] const ushort* utf16;

	out << (unsigned char)header.headerId();	// seems like Qt doesn't handle enum classes in the expected way. 
												// The underlying type is `unsigned char` so this cast "should" be unnecessary.
												// In point of fact it is required.

	switch (header.dataType())
	{
	case OBEXHeader::UNICODE:

		static_assert(sizeof(quint16) == sizeof(QChar), "Size mismatch");

		if (!header.value().canConvert<QString>())
			throw BluetoothException("Cannot convert OBEX header value to string");	// this should be caught in `setValue`, but just in case...

		// convert to big-endian UTF-16
		str = header.value().toString();
		buff.resize(str.size() + 2);	// + null term and BOM
		utf16 = str.utf16();

		// prepend byte order mark
		buff[0] = 0xFFFE;

		// swap the endianess
		index = 1;
		while (utf16[index - 1] != 0x0000)
		{
			buff[index] = _byteswap_ushort(utf16[index - 1]);
			++index;
		}

		// append the null terminator
		buff[index] = 0x00;

		length = buff.size() * 2;		// *2 is because they are 2-byte unicode chars	
		ba = QByteArray::fromRawData(reinterpret_cast<const char*>(buff.constData()), length);

		// don't output the byte array directly or it will prepend it's size and a tag, which doesn't conform to the protocol.
		out << quint16(length + 3);					// + 3 is the name id + 2 byte length field. annoyingly the +3 does an implicit int conversion, so a cast is required.
		out.writeRawData(ba.constData(), length);	// write raw data instead of stream or else it will prepend an int32 length to the bytes.

		break;

	case OBEXHeader::BINARY:
		if (!header.value().canConvert<QByteArray>())
			throw BluetoothException("Cannot convert OBEX header value to byte array");	// this should be caught in `setValue`, but just in case...

		ba = header.value().toByteArray();
		length = (quint16)ba.length();
		out << (quint16)(length + 3);	// account for the id and length bytes too!
		out.writeRawData(ba.constData(), length);

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
