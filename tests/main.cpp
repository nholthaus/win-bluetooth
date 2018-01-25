#include <gtest/gtest.h>
#include <win-bluetooth>
#include <windows.h>
#include <unordered_map>

class BluetoothTest : public ::testing::Test {
protected:

	BluetoothTest() = default;
	virtual ~BluetoothTest() = default;
	void SetUp() override {};
	void TearDown() override {};
};

TEST_F(BluetoothTest, exceptionFromHresult)
{
	BluetoothException ex(ERROR_NO_MORE_ITEMS);
	ASSERT_STREQ(ex.what(), "No more data is available.\r\n");
}

TEST_F(BluetoothTest, exceptionFromString)
{
	constexpr const char* const msg = "Unknown Bluetooth Error";
	BluetoothException ex(msg);
	ASSERT_STREQ(ex.what(), msg);
}

TEST_F(BluetoothTest, enumerateLocalRadios)
{
	Bluetooth r;
	ASSERT_FALSE(r.localRadios().empty());
}

TEST_F(BluetoothTest, radioInfo)
{
	std::unordered_map<std::wstring, unsigned long long> addresses;

	// all the test computers have to be added to this list :(
	addresses[L"DAUNTLESS"] = 92407701068;
	addresses[L"NIC-PC"] = 71340216032535;

	Bluetooth r;
	ASSERT_TRUE(addresses.count(r.localRadio().name())) << "This radio doesn't seem to be in the list of known addresses. Add it?";
	ASSERT_EQ(addresses[r.localRadio().name()], r.localRadio().address());
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
