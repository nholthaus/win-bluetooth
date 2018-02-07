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
	switch (dataType())
	{
	case OBEXHeader::UNICODE:
		return 6 + 2 * m_value.toString().length();	// 1 for id + 2 for length prefix + 1 for null terminator + 2 for the byte order mark + size of string (x2 for unicode) 
	case OBEXHeader::BINARY:
		return 3 + m_value.toByteArray().length();	// 1 for id + 2 for size prefix + length of byte array
	case OBEXHeader::BYTE:
		return 2;									// 1 + 1 for the id
	case OBEXHeader::FOUR_BYTES:
		return 5;									// 4 + 1 for the id
	}

	// if you hit this, somehow the data type is corrupt or someone edited it wrongly. Check the standard.
	assert(false);
	return 0;
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
				val |= data[index++];
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
			str = QString::fromUtf16(reinterpret_cast<const unsigned short*>(&data.data()[index]), length / 2 - 1);	// divide by two because length is in bytes, unicode chars are 2 bytes. Also strip the null terminator, qstring will add one
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

	out << (unsigned char)header.headerId();	// seems like Qt doesn't handle enum classes in the expected way. 
												// The underlying type is `unsigned char` so this cast "should" be unnecessary.
												// In point of fact it is required.

	switch (header.dataType())
	{
	case OBEXHeader::UNICODE:

		// convert the QString to unicode
		static_assert(sizeof(quint16) == sizeof(QChar), "Size mismatch");

		if (!header.value().canConvert<QString>())
			throw BluetoothException("Cannot convert OBEX header value to string");	// this should be caught in `setValue`, but just in case...

		str = header.value().toString();
		str.prepend(QChar::ByteOrderMark);		// necessary for receiver to interpret endianess
		length = (str.size() + 1) * 2;			// +1 is for NULL terminator, *2 is because they are 2-byte unicode chars	
		ba = QByteArray::fromRawData(reinterpret_cast<const char*>(str.constData()), length);

		// don't output the byte array directly or it will prepend it's size and a tag, which doesn't conform to the protocol.
		out << length;
		out.writeRawData(ba.constData(), length);

		break;

	case OBEXHeader::BINARY:
		if (!header.value().canConvert<QByteArray>())
			throw BluetoothException("Cannot convert OBEX header value to byte array");	// this should be caught in `setValue`, but just in case...

		ba = header.value().toByteArray();
		length = (quint16)ba.length();
		out << length;
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
