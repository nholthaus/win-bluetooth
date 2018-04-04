#include <QSharedData>
class BluetoothServiceInfo;

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

	BluetoothServiceInfo * q_ptr;
	QMap<quint16, QVariant>		m_attributes;
	bool						m_isComplete = false;
	BluetoothDeviceInfo			m_device;
};