#include <obexHeader.h>
#include <bluetoothException.h>
#include <QDateTime>

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

