#include <bluetoothException.h>
#include <windows.h>

BluetoothException::BluetoothException(HRESULT result)
{
	FormatMessage(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		// allocate buffer on local heap for error text
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		// Important! will fail otherwise, since we're not 
		// (and CANNOT) pass insertion parameters
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		result,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&m_what,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if (!m_what)
		m_what = "Unknown Bluetooth Error";
}

BluetoothException::BluetoothException(const char* what)
{
	strcpy(m_what, what);
}

BluetoothException::~BluetoothException() noexcept
{
	LocalFree(m_what);
}

char const* BluetoothException::what() const
{
	return m_what;
}

