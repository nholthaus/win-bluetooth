#include <bluetoothServiceinfo.h>
#include <bluetoothDeviceInfo.h>
#include <bluetoothUuids.h>
#include <QSharedData>

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfoPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothServiceInfoPrivate : public QSharedData
{
public:

	Q_DECLARE_PUBLIC(BluetoothServiceInfo)

	BluetoothServiceInfoPrivate(BluetoothServiceInfo* parent) : q_ptr(parent) {};
	virtual ~BluetoothServiceInfoPrivate() {};
	
	
private:

	BluetoothServiceInfo*		q_ptr;
	QMap<quint16, QVariant>		m_attributes;
	bool						m_isComplete = false;
	BluetoothDeviceInfo			m_device;
};

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::BluetoothServiceInfo()
	: d(new BluetoothServiceInfoPrivate(this))
{

}

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::BluetoothServiceInfo(const BluetoothServiceInfo& other)
	: d(other.d)
{

}

//--------------------------------------------------------------------------------------------------
//	~BluetoothServiceInfo (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::~BluetoothServiceInfo()
{

}

//--------------------------------------------------------------------------------------------------
//	attribute () []
//--------------------------------------------------------------------------------------------------
QVariant BluetoothServiceInfo::attribute(quint16 attributeId) const
{
	if (d->m_attributes.contains(attributeId))
	{
		return *d->m_attributes.constFind(attributeId);
	}
	else
		return QVariant();
}

//--------------------------------------------------------------------------------------------------
//	attributes () []
//--------------------------------------------------------------------------------------------------
QList<quint16> BluetoothServiceInfo::attributes() const
{
	return d->m_attributes.keys();
}

//--------------------------------------------------------------------------------------------------
//	contains () []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::contains(quint16 attributeId) const
{
	return d->m_attributes.contains(attributeId);
}

//--------------------------------------------------------------------------------------------------
//	device (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothDeviceInfo BluetoothServiceInfo::device() const
{
	return d->m_device;
}

//--------------------------------------------------------------------------------------------------
//	isComplete (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::isComplete() const
{
	return d->m_attributes.keys().contains(ProtocolDescriptorList);
}

//--------------------------------------------------------------------------------------------------
//	isValid (public ) []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::isValid() const
{
	return !d->m_attributes.isEmpty();
}

//--------------------------------------------------------------------------------------------------
//	protocolDescriptor (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::Sequence BluetoothServiceInfo::protocolDescriptor(BluetoothUuid protocol) const
{
	Sequence retVal;

// 	if(protocol == BluetoothUuid::)
	return retVal;
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const QVariant& value)
{
	d->m_attributes.insert(attributeId, value);
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const Sequence& value)
{
	d->m_attributes.insert(attributeId, value);
}

//--------------------------------------------------------------------------------------------------
//	setAttribute (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setAttribute(quint16 attributeId, const Alternative& value)
{
	d->m_attributes.insert(attributeId, value);
}

//--------------------------------------------------------------------------------------------------
//	setDevice (public ) []
//--------------------------------------------------------------------------------------------------
void BluetoothServiceInfo::setDevice(const BluetoothDeviceInfo& device)
{
	d->m_device = device;
}
