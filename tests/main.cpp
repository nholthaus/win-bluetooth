#define NOMINMAX

#include <gtest/gtest.h>
#include <win-bluetooth>
#include <windows.h>
#include <bluetoothUtils.h>
#include <bluetoothException.h>
#include <obexHeader.h>
#include <obexRequest.h>
#include <obexResponse.h>

#include <iostream>
#include <QHash>
#include <QHostInfo>
#include <QBuffer>
#include <QDataStream>
#include <array>
#include <QCoreApplication>
#include <QTimer>
#include <QFile>

#define STR(s) s.toStdString().c_str()

class BluetoothUtils : public ::testing::Test {
protected:

	BluetoothUtils() = default;
	virtual ~BluetoothUtils() = default;
	void SetUp() override {};
	void TearDown() override {};
};

class BluetoothTest : public ::testing::Test {
protected:

	BluetoothTest() = default;
	virtual ~BluetoothTest() = default;
	void SetUp() override {};
	void TearDown() override {};
};

class OBEX : public ::testing::Test {
protected:

	OBEX() = default;
	virtual ~OBEX() = default;
	void SetUp() override {};
	void TearDown() override {};
};

TEST_F(BluetoothUtils, systemTimeToDateTime)
{
	SYSTEMTIME st;
	st.wYear = 1987;
	st.wMonth = 5;
	st.wDay = 23;
	st.wHour = 5;
	st.wMinute = 36;
	st.wSecond = 02;
	st.wMilliseconds = 657;

	ASSERT_STREQ(STR(systemTimeToDateTime(st).toString("MM-dd-yyyy hh:mm:ss.zzz")), "05-23-1987 05:36:02.657");
}

TEST_F(OBEX, headerId)
{
	OBEXHeader hdr(OBEXHeader::Name);
	ASSERT_EQ(hdr.headerId(), OBEXHeader::Name);
}

TEST_F(OBEX, dataType)
{
	OBEXHeader count(OBEXHeader::Count);
	OBEXHeader name(OBEXHeader::Name);
	OBEXHeader time(OBEXHeader::Time);

	ASSERT_EQ(count.dataType(), OBEXHeader::FOUR_BYTES);
	ASSERT_EQ(name.dataType(), OBEXHeader::UNICODE);
	ASSERT_EQ(time.dataType(), OBEXHeader::BINARY);
}

TEST_F(OBEX, value)
{
	OBEXHeader count(OBEXHeader::Count);
	OBEXHeader name(OBEXHeader::Name);
	OBEXHeader time(OBEXHeader::Time);

	ASSERT_THROW(count.setValue(QString("this should fail")), BluetoothException);
	ASSERT_NO_THROW(count.setValue(32u));
	ASSERT_EQ(count.value().toInt(), 32u);

	ASSERT_THROW(name.setValue(0u), BluetoothException);
	ASSERT_NO_THROW(name.setValue(QString("filename")));
	ASSERT_EQ(name.value().toString(), "filename");

	QDateTime now = QDateTime::currentDateTime();
	ASSERT_THROW(time.setValue(QString("this should fail")), BluetoothException);
	ASSERT_NO_THROW(time.setValue(now));
	ASSERT_EQ(time.value(), now.toUTC().toString(Qt::ISODate));
}

TEST_F(OBEX, stream)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::ReadWrite);

	ASSERT_EQ(ba.size(), 0);

	OBEXHeader count(OBEXHeader::Count, 33u);
	std::array<char, 5> countData{0xC0, 0x00, 0x00, 0x00, 0x21};
	out << count;

	EXPECT_EQ(ba.size(), countData.size());

	for (int i = 0; i < countData.size(); ++i)	
		EXPECT_EQ(ba.at(i), countData.at(i)) << "at index " << i;
}

TEST_F(OBEX, connect)
{
	OBEXConnect c(8192);
	c.addOptionalHeader(OBEXHeader::Count,	4u);
	c.addOptionalHeader(OBEXHeader::Length,	0xF483u);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::ReadWrite);

	out << c;

	// from section 3.3.1.9 of the OBEX standard
	std::array<char, 17> truth{ 0x80, 0x00, 0x11, 0x10, 0x00, 0x20, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x04, 0xC3, 0x00, 0x00, 0xF4, 0x83};

	EXPECT_EQ(ba.size(), truth.size());

	for (int i = 0; i < truth.size(); ++i)
		EXPECT_EQ(ba.at(i), truth.at(i)) << "at index " << i;

	OBEXConnect c2(8192);
	c2.addOptionalHeader(OBEXHeader::Name, "hi.txt");
	c2.addOptionalHeader(OBEXHeader::Type, "text\0", 5);

	QByteArray ba2;
	QDataStream out2(&ba2, QIODevice::ReadWrite);

	out2 << c2;
	std::array<char, 34> truth2{ 0x80, 0x00, 0x22, 0x10, 0x00, 0x20, 0x00, 0x01, 0x00, 0x13, 0xFE, 0xFF, 0x00, 0x68, 0x00, 0x69, 0x00, 0x2E, 0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0x42, 0x00, 0x08, 0x74, 0x65, 0x78, 0x74, 0x00 };

	EXPECT_EQ(ba2.size(), truth2.size());

	for (int i = 0; i < truth2.size(); ++i)
		EXPECT_EQ(ba2.at(i), truth2.at(i)) << "at index " << i;
}

TEST_F(OBEX, fromRawData)
{
	std::array<char, 37> truth{ 0xC0, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x13, 0xFE, 0xFF, 0x00, 0x68, 0x00, 0x69, 0x00, 0x2E, 0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0xC3, 0x00, 0x00, 0xF4, 0x83, 0x42, 0x00, 0x08, 0x74, 0x65, 0x78, 0x74, 0x00 };
	auto headers = OBEXHeader::fromByteArray(QByteArray::fromRawData(&truth[0], truth.size()));

	EXPECT_EQ(headers.at(0).headerId(), OBEXHeader::Count);
	EXPECT_EQ(headers.at(0).value().toUInt(), 4);

	EXPECT_EQ(headers.at(1).headerId(), OBEXHeader::Name);
	EXPECT_EQ(headers.at(1).value().toString(), QString("hi.txt"));

	EXPECT_EQ(headers.at(2).headerId(), OBEXHeader::Length);
	EXPECT_EQ(headers.at(2).value().toUInt(), 0xF483u);

	EXPECT_EQ(headers.at(3).headerId(), OBEXHeader::Type);
	EXPECT_STREQ(headers.at(3).value().toByteArray(), "text");
}

TEST_F(BluetoothTest, BluetoothAddress)
{
	BluetoothAddress addr("00:15:83:ED:9E:4C");
	ASSERT_EQ(addr, (unsigned long long)92407701068);
	ASSERT_STREQ(STR(QString(addr)), STR(QString("00:15:83:ED:9E:4C")));
}

TEST_F(BluetoothTest, BluetoothUuid)
{
	ASSERT_STREQ(STR(BluetoothUuid(Protocol::RFCOMM).toString()),	"{00000003-0000-1000-8000-00805F9B34FB}");
	ASSERT_STREQ(STR(BluetoothUuid().toString()),					"{00000000-0000-0000-0000-000000000000}");
}

TEST_F(BluetoothTest, name)
{
	ASSERT_STREQ(STR(Bluetooth::name(Bluetooth::localRadio().address())), STR(QHostInfo::localHostName().toUpper()));
}

TEST_F(BluetoothTest, BluetoothDeviceInfo)
{
	BluetoothAddress addr("00:15:83:ED:9E:4C");
	QString name = "Test Device";
	BluetoothDeviceInfo devInfo(addr, "Test Device", 0x22010C);

	EXPECT_EQ(devInfo.address(), addr);
	EXPECT_STREQ(STR(devInfo.name()), STR(name));
	EXPECT_EQ(devInfo.majorDeviceClass(), BluetoothDeviceInfo::ComputerDevice);
	EXPECT_EQ(devInfo.minorDeviceClass(), BluetoothDeviceInfo::LaptopComputer);
	EXPECT_EQ(devInfo.serviceClasses(), BluetoothDeviceInfo::AudioService | BluetoothDeviceInfo::NetworkingService);
}

TEST_F(BluetoothTest, exceptionFromHresult)
{
	BluetoothException ex(ERROR_NO_MORE_ITEMS);
	ASSERT_STREQ(ex.what(), "No more data is available.");
}

TEST_F(BluetoothTest, exceptionFromString)
{
	constexpr const char* const msg = "Unknown Bluetooth Error";
	BluetoothException ex(msg);
	ASSERT_STREQ(ex.what(), msg);
}

TEST_F(BluetoothTest, enumerateLocalRadios)
{
	ASSERT_FALSE(Bluetooth::localRadios().empty());
}

TEST_F(BluetoothTest, discoverable)
{
	ASSERT_FALSE(Bluetooth::localRadios().empty());

	Bluetooth::localRadio().setDiscoverable(true);
	ASSERT_TRUE(Bluetooth::localRadio().discoverable());

#ifndef _WIN32
	// windows doesn't let you turn off discovery
	Bluetooth::localRadio().setDiscoverable(false);
	ASSERT_FALSE(Bluetooth::localRadio().discoverable());
#endif
}

TEST_F(BluetoothTest, connectable)
{
	ASSERT_FALSE(Bluetooth::localRadios().empty());

	Bluetooth::localRadio().setConnectable(true);
	ASSERT_TRUE(Bluetooth::localRadio().connectable());

#ifndef _WIN32
	// windows doesn't let you turn off connectability
	Bluetooth::localRadio().setConnectable(false);
	ASSERT_FALSE(Bluetooth::localRadio().connectable());
#endif
}

TEST_F(BluetoothTest, radioInfo)
{
	QHash<QString, BluetoothAddress> addresses;

	// all the test computers have to be added to this list :(
	addresses["DAUNTLESS"]	= 102200634555791;
	addresses["NIC-PC"]		= 71340216032535;

	ASSERT_TRUE(addresses.count(Bluetooth::localRadio().name())) << "This radio doesn't seem to be in the list of known addresses. Add it?";
	ASSERT_EQ(addresses[Bluetooth::localRadio().name()], Bluetooth::localRadio().address());

	std::cout << "LOCAL RADIOS:" << std::endl;
	for (const auto& [name, radio] : Bluetooth::localRadios())
		std::cout << "    " << name.toStdString() << std::endl;
}

TEST_F(BluetoothTest, deviceInfo)
{
	std::unordered_map<QString, BluetoothAddress> addresses;

	// all the test computers have to be added to this list :(
	addresses["BSR36"] = 107442151553;

	for (auto& [name, address] : addresses)
	{
		ASSERT_STREQ(STR(name), STR(Bluetooth::remoteDevice(name).name())) << "Did not find remote device: " << name.data();
		ASSERT_EQ(address, Bluetooth::remoteDevice(name).address());
	}

	std::cout << "REMOTE DEVICES:" << std::endl;
	for (const auto& [name, device] : Bluetooth::remoteDevices())
		std::cout << "    " << name.toStdString() << std::endl;
}

TEST_F(BluetoothTest, serviceInfo)
{
	QEventLoop eventLoop;

	BluetoothServiceDiscoveryAgent agent("RELENTLESS");
	QObject::connect(&agent, &BluetoothServiceDiscoveryAgent::finished, &eventLoop, &QEventLoop::quit);
	
	agent.start(BluetoothServiceDiscoveryAgent::FullDiscovery);

	eventLoop.exec();

	for (const auto& service : agent.discoveredServices())
	{
		EXPECT_TRUE(service.isComplete());
		EXPECT_TRUE(service.isValid());
	}

}

TEST_F(BluetoothTest, transferManager)
{
	// for this test to succeed, the receiving PC needs to be in a state where it can accept
	// incoming bluetooth files.
	QEventLoop eventLoop;
	int transfered = 0;

	QSharedPointer<QFile> file(new QFile(":/res/words.txt"));

	// Create a transfer manager
	BluetoothTransferManager *transferManager = new BluetoothTransferManager;

	// Create the transfer request and file to be sent
	BluetoothAddress remoteAddress("RELENTLESS");
	BluetoothTransferRequest request(remoteAddress);
	request.setAttribute(BluetoothTransferRequest::Attribute::NameAttribute, "words.txt");

	// Ask the transfer manager to send it
	QTime duration;
	duration.start();
	auto reply = transferManager->put(request, file);

	int transmittedPackets = 0;
	int readyReadsReceived = 0;
	if (reply->error() == BluetoothTransferReply::NoError) 
	{
		QObject::connect(reply, &BluetoothTransferReply::transferProgress, &eventLoop,
			[&transfered](qint64 sent, qint64 total)
		{
			transfered = sent * 100 / total;
		}, Qt::QueuedConnection);
		QObject::connect(reply, &BluetoothTransferReply::finished, &eventLoop, [&eventLoop, &reply]()
		{
			reply->deleteLater();
			eventLoop.exit();
		}, Qt::QueuedConnection);
	}
	else 
	{
		eventLoop.exit();
		qWarning() << "Cannot push testfile.txt:" << reply->errorString();
	}

	eventLoop.exec();
	auto ms = duration.elapsed();

	double sizeBytes = file->size();
	double sizeMegaBits = sizeBytes * 8.0 / 1'000'000.0;
	double seconds = ms / 1'000.0;
	double throughput = sizeMegaBits / seconds;
	qDebug() << "Throughput:" << throughput << "Mbps";
	EXPECT_EQ(reply->error(), BluetoothTransferReply::NoError);
	EXPECT_EQ(transfered, 100);
}

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
