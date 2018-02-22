#include <bluetoothServiceinfo.h>

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfoPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothServiceInfoPrivate
{
public:

	Q_DECLARE_PUBLIC(BluetoothServiceInfo)

	BluetoothServiceInfoPrivate(BluetoothServiceInfo* parent) : q_ptr(parent) {};
	virtual ~BluetoothServiceInfoPrivate() {};
	
	
private:

	BluetoothServiceInfo*		q_ptr;
	QMap<quint16, QVariant>		m_attributes;

};

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfo (public ) []
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::BluetoothServiceInfo()
	: d_ptr(new BluetoothServiceInfoPrivate(this))
{

}

//--------------------------------------------------------------------------------------------------
//	~BluetoothServiceInfo (public ) [virtual ]
//--------------------------------------------------------------------------------------------------
BluetoothServiceInfo::~BluetoothServiceInfo()
{
	delete d_ptr;
}

//--------------------------------------------------------------------------------------------------
//	attribute () []
//--------------------------------------------------------------------------------------------------
QVariant BluetoothServiceInfo::attribute(quint16 attributeId) const
{
	return QVariant();
}

//--------------------------------------------------------------------------------------------------
//	attributes () []
//--------------------------------------------------------------------------------------------------
QList<quint16> BluetoothServiceInfo::attributes() const
{
	return { 0 };
}

//--------------------------------------------------------------------------------------------------
//	contains () []
//--------------------------------------------------------------------------------------------------
bool BluetoothServiceInfo::contains(quint16 attributeId) const
{
	return false;
}
