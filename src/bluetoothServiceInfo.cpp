#include <bluetoothServiceinfo.h>

//--------------------------------------------------------------------------------------------------
//	BluetoothServiceInfoPrivate
//--------------------------------------------------------------------------------------------------

class BluetoothServiceInfoPrivate
{
public:

	BluetoothServiceInfoPrivate() = default;
	virtual ~BluetoothServiceInfoPrivate() = default;
	
	
private:

	QMap<quint16, QVariant>		m_attributes;

};