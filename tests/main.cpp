#include <gtest/gtest.h>
#include <win-bluetooth>
#include <windows.h>
#include <bluetoothUtils.h>

#include <iostream>
#include <QHash>

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

TEST_F(BluetoothUtils, systemTimeToString)
{
	SYSTEMTIME st;
	st.wYear = 1987;
	st.wMonth = 5;
	st.wDay = 23;
	st.wHour = 5;
	st.wMinute = 36;
	st.wSecond = 02;
	st.wMilliseconds = 657;

	ASSERT_STREQ(systemTimeToString(st).c_str(), "05-23-1987 05:36:02.657");
}

TEST_F(BluetoothTest, BluetoothAddress)
{
	BluetoothAddress addr("00:15:83:ED:9E:4C");
	ASSERT_EQ(addr, (unsigned long long)92407701068);
	ASSERT_STREQ(STR(QString(addr)), STR(QString("00:15:83:ED:9E:4C")));
}

TEST_F(BluetoothTest, BluetoothUuid)
{
	ASSERT_STREQ(STR(BluetoothUuid(Protocol::RFCOMM).toString()), "{00030000-0000-1000-8000-00805F9B34FB}");
	
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
	addresses["DAUNTLESS"] = 92407701068;
	addresses["NIC-PC"] = 71340216032535;

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
	addresses["SAMSUNG-SM-G935V"] = 163237606836978;

	for (auto& [name, address] : addresses)
	{
		ASSERT_STREQ(STR(name), STR(Bluetooth::remoteDevice(name).name())) << "Did not find remote device: " << name.data();
		ASSERT_EQ(address, Bluetooth::remoteDevice(name).address());
	}

	std::cout << "REMOTE DEVICES:" << std::endl;
	for (const auto& [name, address] : Bluetooth::remoteDevices())
		std::cout << "    " << name.toStdString() << std::endl;
}

 TEST_F(BluetoothTest, connect)
{
	ASSERT_TRUE(Bluetooth::localRadio().connectTo(Bluetooth::remoteDevice("RELENTLESS")));
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
