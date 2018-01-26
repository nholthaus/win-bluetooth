#include <bluetoothUtils.h>
#include <sstream>
#include <iomanip>

std::string systemTimeToString(const SYSTEMTIME& time)
{
	char buff[24];
	sprintf_s(buff, 
		"%02d-%02d-%d %02d:%02d:%02d.%03d",
		time.wMonth,
		time.wDay,
		time.wYear,
		time.wHour,
		time.wMinute,
		time.wSecond,
		time.wMilliseconds);
	return std::string(buff);
}

std::wstring bluetoothAddressToString(unsigned long long address)
{
	std::wstringstream ss;
	ss << std::setfill(L'0') << std::setw(12) << std::hex << address;
	std::wstring s = ss.str();
	ss.str(L"");
	for (int i = 0; i < s.size(); i++) {
		if (i % 2 == 0 && i) ss << ':';
			ss << s[i];
	}
	return ss.str();
}

