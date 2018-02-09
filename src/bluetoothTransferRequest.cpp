#include <bluetoothTransferRequest.h>
#include <QDateTime>

//--------------------------------------------------------------------------------------------------
//	BluetoothTransferRequest (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothTransferRequest::BluetoothTransferRequest(const BluetoothAddress& address /*= BluetoothAddress()*/)
	: m_address(address)
{

}

//--------------------------------------------------------------------------------------------------
//	address (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothAddress BluetoothTransferRequest::address() const
{
	return m_address;
}

//--------------------------------------------------------------------------------------------------
//	attribute (public ) []
//--------------------------------------------------------------------------------------------------
QVariant BluetoothTransferRequest::attribute(Attribute code, const QVariant& defaultValue /*= QVariant()*/) const
{
	OBEXHeader::HeaderIdentifier id;

	switch (code)
	{
	case BluetoothTransferRequest::Attribute::DescriptionAttribute:
		id = OBEXHeader::Description;
		break;
	case BluetoothTransferRequest::Attribute::TimeAttribute:
		id = OBEXHeader::Time;
		break;
	case BluetoothTransferRequest::Attribute::TypeAttribute:
		id = OBEXHeader::Type;
		break;
	case BluetoothTransferRequest::Attribute::LengthAttribute:
		id = OBEXHeader::Length;
		break;
	case BluetoothTransferRequest::Attribute::NameAttribute:
		id = OBEXHeader::Name;
		break;
	default:
		return defaultValue;
	}

	if (auto[hasAttribute, itr] = m_attributes.contains(id); hasAttribute)
		return itr->value();
	else
		return defaultValue;
}

//--------------------------------------------------------------------------------------------------
//	attributes (public ) []
//--------------------------------------------------------------------------------------------------
const OBEXOptionalHeaders& BluetoothTransferRequest::attributes() const
{
	return m_attributes;
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothTransferRequest::setAttribute(Attribute code, const QVariant& value)
{
	switch (code)
	{
	case BluetoothTransferRequest::Attribute::DescriptionAttribute:
		if (auto[hasAttribute, itr] = m_attributes.contains(OBEXHeader::Description); hasAttribute)
			itr->setValue(value.toString());
		else
			m_attributes.emplace_back(OBEXHeader::Description, value.toString());
		return;
	case BluetoothTransferRequest::Attribute::TimeAttribute:
		if (auto[hasAttribute, itr] = m_attributes.contains(OBEXHeader::Time); hasAttribute)
			itr->setValue(value.toDateTime());
		else
			m_attributes.emplace_back(OBEXHeader::Time, value.toDateTime());
		return;
	case BluetoothTransferRequest::Attribute::TypeAttribute:
		if (auto[hasAttribute, itr] = m_attributes.contains(OBEXHeader::Type); hasAttribute)
			itr->setValue(value.toString().toLocal8Bit() + '\0');
		else
			m_attributes.emplace_back(OBEXHeader::Type, value.toString().toLocal8Bit() + '\0');
		return;
	case BluetoothTransferRequest::Attribute::LengthAttribute:
		if (auto[hasAttribute, itr] = m_attributes.contains(OBEXHeader::Length); hasAttribute)
			itr->setValue(value.toUInt());
		else
			m_attributes.emplace_back(OBEXHeader::Length, value.toUInt());
		return;
	case BluetoothTransferRequest::Attribute::NameAttribute:
		if (auto[hasAttribute, itr] = m_attributes.contains(OBEXHeader::Name); hasAttribute)
			itr->setValue(value.toString());
		else
			m_attributes.emplace_back(OBEXHeader::Name, value.toString());
		return;
	default:
		return;
	}
}

//--------------------------------------------------------------------------------------------------
//	operator== (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothTransferRequest::operator==(const BluetoothTransferRequest& other) const
{
	return (m_address == other.m_address && m_attributes == other.m_attributes);
}

//--------------------------------------------------------------------------------------------------
//	operator!= (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothTransferRequest::operator!=(const BluetoothTransferRequest& other) const
{
	return !(*this == other);
}

