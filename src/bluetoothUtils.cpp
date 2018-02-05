#include <bluetoothUtils.h>
#include <QDateTime>

QDateTime systemTimeToDateTime(const SYSTEMTIME& time)
{
	QString dt = QString("%1-%2-%3 %4:%5:%6.%7")
		.arg(time.wMonth, 2, 10, QChar('0'))
		.arg(time.wDay, 2, 10, QChar('0'))
		.arg(time.wYear)
		.arg(time.wHour, 2, 10, QChar('0'))
		.arg(time.wMinute, 2, 10, QChar('0'))
		.arg(time.wSecond, 2, 10, QChar('0'))
		.arg(time.wMilliseconds, 3, 10, QChar('0'));
	return QDateTime::fromString(dt, "MM-dd-yyyy hh:mm:ss.zzz");
}
