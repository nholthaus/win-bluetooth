#include <bluetoothUtils.h>

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

