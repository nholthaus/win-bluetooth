#define NOMINMAX

//------------------------------
//	INCLUDES
//------------------------------

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
#include <QCommandLineParser>

//------------------------------
//	GLOBAL VARS
//------------------------------

QString LOCAL_PC_NAME;
QString LOCAL_PC_ADDRESS;
QString REMOTE_PC_NAME;
QString REMOTE_PC_ADDRESS;

//------------------------------
//	MACROS
//------------------------------

#define STR(s) s.toStdString().c_str()

//--------------------------------------------------------------------------------------------------
//	TEST FIXTURES
//--------------------------------------------------------------------------------------------------

class BluetoothServerTest : public ::testing::Test 
{
protected:

	BluetoothServerTest() = default;
	virtual ~BluetoothServerTest() = default;
	void SetUp() override {};
	void TearDown() override {};
};

class BluetoothClientTest : public ::testing::Test 
{
protected:

	BluetoothClientTest() = default;
	virtual ~BluetoothClientTest() = default;
	void SetUp() override {};
	void TearDown() override {};
};

//--------------------------------------------------------------------------------------------------
//	SERVER TEST CASES
//--------------------------------------------------------------------------------------------------

TEST_F(BluetoothServerTest, systemTimeToDateTime)
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

TEST_F(BluetoothServerTest, OBEXheaderId)
{
	OBEXHeader hdr(OBEXHeader::Name);
	ASSERT_EQ(hdr.headerId(), OBEXHeader::Name);
}

TEST_F(BluetoothServerTest, OBEXdataType)
{
	OBEXHeader count(OBEXHeader::Count);
	OBEXHeader name(OBEXHeader::Name);
	OBEXHeader time(OBEXHeader::Time);

	ASSERT_EQ(count.dataType(), OBEXHeader::FOUR_BYTES);
	ASSERT_EQ(name.dataType(), OBEXHeader::UNICODE);
	ASSERT_EQ(time.dataType(), OBEXHeader::BINARY);
}

TEST_F(BluetoothServerTest, OBEXvalue)
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

TEST_F(BluetoothServerTest, OBEXstream)
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

TEST_F(BluetoothServerTest, OBEXconnect)
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

TEST_F(BluetoothServerTest, OBEXfromRawData)
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

TEST_F(BluetoothServerTest, BluetoothAddress)
{
	BluetoothAddress addr("00:15:83:ED:9E:4C");
	ASSERT_EQ(addr, (unsigned long long)92407701068);
	ASSERT_STREQ(STR(QString(addr)), STR(QString("00:15:83:ED:9E:4C")));
}

TEST_F(BluetoothServerTest, BluetoothUuid)
{
	ASSERT_STREQ(STR(BluetoothUuid(Protocol::RFCOMM).toString()),	"{00000003-0000-1000-8000-00805F9B34FB}");
	ASSERT_STREQ(STR(BluetoothUuid().toString()),					"{00000000-0000-0000-0000-000000000000}");
}

TEST_F(BluetoothServerTest, name)
{
	ASSERT_STREQ(STR(Bluetooth::name(Bluetooth::localRadio().address())), STR(QHostInfo::localHostName().toUpper()));
}

TEST_F(BluetoothServerTest, BluetoothDeviceInfo)
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

TEST_F(BluetoothServerTest, exceptionFromHresult)
{
	BluetoothException ex(ERROR_NO_MORE_ITEMS);
	ASSERT_STREQ(ex.what(), "No more data is available.");
}

TEST_F(BluetoothServerTest, exceptionFromString)
{
	constexpr const char* const msg = "Unknown Bluetooth Error";
	BluetoothException ex(msg);
	ASSERT_STREQ(ex.what(), msg);
}

TEST_F(BluetoothServerTest, enumerateLocalRadios)
{
	ASSERT_FALSE(Bluetooth::localRadios().empty());
}

TEST_F(BluetoothServerTest, discoverable)
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

TEST_F(BluetoothServerTest, connectable)
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

TEST_F(BluetoothServerTest, radioInfo)
{
	QHash<QString, BluetoothAddress> addresses;

	// all the test computers have to be added to this list :(
	addresses[LOCAL_PC_NAME]	= BluetoothAddress(LOCAL_PC_ADDRESS);

	ASSERT_TRUE(addresses.count(Bluetooth::localRadio().name())) << "This radio doesn't seem to be in the list of known addresses. Add it?";
	ASSERT_EQ(addresses[Bluetooth::localRadio().name()], Bluetooth::localRadio().address());

	std::cout << "LOCAL RADIOS:" << std::endl;
	for (const auto& [name, radio] : Bluetooth::localRadios())
		std::cout << "    " << name.toStdString() << std::endl;
}

TEST_F(BluetoothServerTest, deviceInfo)
{
	std::unordered_map<QString, BluetoothAddress> addresses;

	// all the test computers have to be added to this list :(
	addresses[REMOTE_PC_NAME] = BluetoothAddress(REMOTE_PC_ADDRESS);

	for (auto& [name, address] : addresses)
	{
		ASSERT_STREQ(STR(name), STR(Bluetooth::remoteDevice(name).name())) << "Did not find remote device: " << name.data();
		ASSERT_EQ(address, Bluetooth::remoteDevice(name).address());
	}

	std::cout << "REMOTE DEVICES:" << std::endl;
	for (const auto& [name, device] : Bluetooth::remoteDevices())
		std::cout << "    " << name.toStdString() << std::endl;
}

TEST_F(BluetoothServerTest, serviceInfo)
{
	QEventLoop eventLoop;

	BluetoothServiceDiscoveryAgent agent(REMOTE_PC_NAME);
	QObject::connect(&agent, &BluetoothServiceDiscoveryAgent::finished, &eventLoop, &QEventLoop::quit);
	
	agent.start(BluetoothServiceDiscoveryAgent::FullDiscovery);

	eventLoop.exec();

	QSet<int> knownChannels;
	qDebug() << "DEVICE:" << REMOTE_PC_NAME;
	for (const auto& service : agent.discoveredServices())
	{
		qDebug() << "    " << service.serviceName() << service.serviceDescription() << service.serviceClassUuids();
		EXPECT_TRUE(service.isComplete());
		EXPECT_TRUE(service.isValid());
		EXPECT_FALSE(service.serviceClassUuids().isEmpty());
		EXPECT_FALSE(knownChannels.contains(service.serverChannel())) << "This means there's a duplicate entry in the service list";
		knownChannels.insert(service.serverChannel());
	}	
}

TEST_F(BluetoothServerTest, registerService)
{
	// register a service
	BluetoothServiceInfo service;
	QString serviceName = "win-bluetooth test service";
	QString serviceDescription = "Test service for test purposes";
	BluetoothUuid uuid = BluetoothUuid(ServiceClass::MSDNBluetoothConnectionExample);
	service.setServiceUuid(uuid);
	service.setServiceName(serviceName);
	service.setServiceDescription(serviceDescription);
	service.setServiceProvider("Menari Softworks");
	service.registerService();

	EXPECT_TRUE(service.isRegistered()); 

	QEventLoop eventLoop;

	// check that it's there
	BluetoothServiceDiscoveryAgent agent;
	QObject::connect(&agent, &BluetoothServiceDiscoveryAgent::finished, &eventLoop, &QEventLoop::quit);

	agent.start(BluetoothServiceDiscoveryAgent::FullDiscovery);

	eventLoop.exec();

	bool foundTheTestService = false;
	qDebug() << "DEVICE:" << QHostInfo::localHostName();
	for (const auto& service : agent.discoveredServices())
	{
		if(service.serviceName() == serviceName)
		{
			qDebug() << "    " << service.serviceName() << service.serviceDescription() << service.serviceClassUuids();
			EXPECT_TRUE(service.isComplete());
			EXPECT_TRUE(service.isValid());
			EXPECT_EQ(uuid, service.serviceClassUuids().first());
			EXPECT_STREQ(STR(serviceName), STR(service.serviceName()));
			EXPECT_STREQ(STR(serviceDescription), STR(service.serviceDescription()));
			foundTheTestService = true;
		}
	}

	EXPECT_TRUE(foundTheTestService);

	// unregister the service
	service.unregisterService();
	EXPECT_FALSE(service.isRegistered());

	agent.start(BluetoothServiceDiscoveryAgent::FullDiscovery);
	eventLoop.exec();

	if (!agent.discoveredServices().isEmpty())
		foundTheTestService = false;

	for (const auto& service : agent.discoveredServices())
	{
		if (service.serviceName() == serviceName)
			foundTheTestService = true;
	}

	EXPECT_FALSE(foundTheTestService);
}

TEST_F(BluetoothServerTest, server)
{
	QEventLoop eventLoop;

	// won't pass/complete unless another PC is running in client mode
	BluetoothServer server;
	BluetoothSocket* connectedSocket = nullptr;

	EXPECT_TRUE(server.listen(QHostInfo::localHostName(), 5)) << STR(server.errorString());

	QObject::connect(&server, QOverload<BluetoothServer::Error>::of(&BluetoothServer::error), &eventLoop,
		[&server](BluetoothServer::Error err)
	{
		ADD_FAILURE();
		qDebug() << server.errorString();
	}, Qt::QueuedConnection);

	QObject::connect(&server, &BluetoothServer::newConnection, &eventLoop, [&]()
	{
		// check for a connection
		EXPECT_TRUE(server.hasPendingConnections());
		connectedSocket = server.nextPendingConnection();
		connectedSocket->setParent(&eventLoop);
		EXPECT_TRUE(connectedSocket);
		EXPECT_FALSE(server.hasPendingConnections());

		// send a message to the client
		EXPECT_NO_THROW(connectedSocket->write("Server says, why hello there!"));

		eventLoop.quit();
	}, Qt::QueuedConnection);

	int retVal = eventLoop.exec();

	EXPECT_EQ(0, retVal) << STR(server.errorString());

	// receive a message from the client
	QObject::connect(connectedSocket, &BluetoothSocket::readyRead, &eventLoop, [&]()
	{
		QString message = connectedSocket->readAll();
		qDebug() << message;
		EXPECT_STREQ("Client says hi!", STR(message));
	}, Qt::QueuedConnection);

	QObject::connect(connectedSocket, QOverload<BluetoothSocket::SocketError>::of(&BluetoothSocket::error), &eventLoop,
		[&, connectedSocket](BluetoothSocket::SocketError err)
	{
		ADD_FAILURE();
		qDebug() << connectedSocket->errorString();
		eventLoop.exit(1);
	}, Qt::QueuedConnection);

	QObject::connect(connectedSocket, &BluetoothSocket::disconnected, &eventLoop, [&, connectedSocket]()
	{
		eventLoop.exit();
	}, Qt::QueuedConnection);

	retVal = eventLoop.exec();
}

TEST_F(BluetoothServerTest, transferManager)
{
	// for this test to succeed, the receiving PC needs to be in a state where it can accept
	// incoming bluetooth files.
	QEventLoop eventLoop;
	int transfered = 0;

	QSharedPointer<QFile> file(new QFile(":/res/words.txt"));

	// Create a transfer manager
	BluetoothTransferManager *transferManager = new BluetoothTransferManager;

	// Create the transfer request and file to be sent
	BluetoothAddress remoteAddress(REMOTE_PC_NAME);
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

//--------------------------------------------------------------------------------------------------
//	CLIENT TEST CASES
//--------------------------------------------------------------------------------------------------

TEST_F(BluetoothClientTest, client)
{
	BluetoothAddress address(REMOTE_PC_NAME);
	BluetoothSocket socket;

	EXPECT_EQ(BluetoothSocket::SocketState::UnconnectedState, socket.state());

	bool gotConnectedSignal = false;
	bool gotStateChangedSignal = false;
	bool gotReadyReadSignal = false;
	bool gotDisconnectedSignal = false;

	QEventLoop eventLoop;
	QObject::connect(&socket, &BluetoothSocket::connected, &eventLoop, [&]()
	{
		EXPECT_TRUE(socket.state() == BluetoothSocket::SocketState::ConnectedState);
		gotConnectedSignal = true;
	}, Qt::QueuedConnection);

	QObject::connect(&socket, &BluetoothSocket::stateChanged, &eventLoop, [&](BluetoothSocket::SocketState state)
	{
		gotStateChangedSignal = true;
	}, Qt::QueuedConnection);
	
	QObject::connect(&socket, QOverload<BluetoothSocket::SocketError>::of(&BluetoothSocket::error), &eventLoop,
		[&socket](BluetoothSocket::SocketError err)
	{
		ADD_FAILURE();
		qDebug() << socket.errorString();
	}, Qt::QueuedConnection);

	QObject::connect(&socket, &BluetoothSocket::readyRead, &eventLoop, [&]()
	{
		gotReadyReadSignal = true;

		// receive a message from the server
		QString message = socket.readAll();
		EXPECT_STREQ("Server says, why hello there!", STR(message));

		message = "Client says hi!";
		EXPECT_EQ(message.size(), socket.write(message.toLocal8Bit()));

		socket.disconnectFromService();
	}, Qt::QueuedConnection);

	QObject::connect(&socket, &BluetoothSocket::disconnected, &eventLoop, [&]()
	{
		gotDisconnectedSignal = true;
		eventLoop.quit();
	}, Qt::QueuedConnection);

	// start the connection once the event loop starts
	QTimer::singleShot(100, [&socket, address]()
	{
		// connect to the remote server via port #
		socket.connectToService(address, 5);
	});
	int retVal = eventLoop.exec();
	
	EXPECT_EQ(0, retVal);

	EXPECT_TRUE(gotConnectedSignal);
	EXPECT_TRUE(gotStateChangedSignal);
	EXPECT_TRUE(gotReadyReadSignal);
	EXPECT_TRUE(gotDisconnectedSignal);
}

//--------------------------------------------------------------------------------------------------
//	MAIN
//--------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("win-bluetooth-test");
	QCoreApplication::setApplicationVersion("1.0.0");

	QCommandLineParser parser;
	parser.setApplicationDescription("Test application for the win-bluetooth library");
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addPositionalArgument("LocalPCName", "Hostname of the local PC");
	parser.addPositionalArgument("LocalPCAddress", "Bluetooth address of the local PC, as a string", "XX:XX:XX:XX:XX:XX");
	parser.addPositionalArgument("RemotePCName", "Hostname of the remote PC");
	parser.addPositionalArgument("RemotePCAddress", "Bluetooth address of the remote PC, as a string", "XX:XX:XX:XX:XX:XX");
	QCommandLineOption runAsClientOption("c", "Run the unit tests in client mode. By default, test are run in server mode");
	parser.addOption(runAsClientOption);

	parser.process(app);

	const QStringList args = parser.positionalArguments();
	LOCAL_PC_NAME = args.at(0);
	LOCAL_PC_ADDRESS = args.at(1);
	REMOTE_PC_NAME = args.at(2);
	REMOTE_PC_ADDRESS = args.at(3);

	if(parser.isSet(runAsClientOption))
		::testing::GTEST_FLAG(filter) = "*Client*";
	else
		::testing::GTEST_FLAG(filter) = "*Server*";

	return RUN_ALL_TESTS();
}
