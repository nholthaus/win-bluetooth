//--------------------------------------------------------------------------------------------------
// 
//	WIN-BLUETOOTH
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
// and associated documentation files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, copy, modify, merge, publish, 
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or 
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2018 Nic Holthaus
// 
//--------------------------------------------------------------------------------------------------
//
// ATTRIBUTION:
//
//
//--------------------------------------------------------------------------------------------------
//
/// @file	bluetoothDeviceInfo.h
/// @brief	
//
//--------------------------------------------------------------------------------------------------

#pragma once
#ifndef bluetoothDeviceInfo_h__
#define bluetoothDeviceInfo_h__

//-------------------------
//	INCLUDES
//-------------------------

#include <QObject> 

//-------------------------
//	FORWARD DECLARATIONS
//-------------------------

class BluetoothAddress;
class BluetoothDevice;
class BluetoothUuid;
class BluetoothDeviceInfoPrivate;

//--------------------------------------------------------------------------------------------------
//	BluetoothDeviceInfo
//--------------------------------------------------------------------------------------------------

class BluetoothDeviceInfo
{
	Q_GADGET

public:

	enum CoreConfiguration
	{
		UnknownCoreConfiguration				= 0x00,	///<The type of the Bluetooth device cannot be determined.
		BaseRateCoreConfiguration				= 0x02,	///<The device is a standard Bluetooth device.
		BaseRateAndLowEnergyCoreConfiguration	= 0x03,	///<The device is a Bluetooth Smart device with support for standard and Low Energy device.
		LowEnergyCoreConfiguration				= 0x01,	///<The device is a Bluetooth Low Energy device.
	};
	Q_ENUM(CoreConfiguration);
	Q_DECLARE_FLAGS(CoreConfigurations, CoreConfiguration);
	Q_FLAG(CoreConfigurations);

	enum DataCompleteness
	{
		DataComplete							= 0,	///<The data is complete.
		DataIncomplete							= 1,	///<The data is incomplete.Addition datum is available via other interfaces.
		DataUnavailable							= 2,	///<No data is available.
	};

	enum MajorDeviceClass
	{
		MiscellaneousDevice						= 0,	///<A miscellaneous device.
		ComputerDevice							= 1,	///<A computer device or PDA.
		PhoneDevice								= 2,	///<A telephone device.
		LANAccessDevice							= 3,	///<A device that provides access to a local area network.
		AudioVideoDevice						= 4,	///<A device capable of playback or capture of audio and/or video.
		PeripheralDevice						= 5,	///<A peripheral device such as a keyboard, mouse, and so on.
		ImagingDevice							= 6,	///<An imaging device such as a display, printer, scanner or camera.
		WearableDevice							= 7,	///<A wearable device such as a watch or pager.
		ToyDevice								= 8,	///<A toy.
		HealthDevice							= 9,	///<A health related device such as heart rate or temperature monitor.
		UncategorizedDevice						= 31,	///<A device that does not fit into any of the other device classes.
	};
	enum MinorAudioVideoClass
	{
		UncategorizedAudioVideoDevice			= 0,	///<An uncategorized audio / video device.
		WearableHeadsetDevice					= 1,	///<A wearable headset device.
		HandsFreeDevice							= 2,	///<A handsfree device.
		Microphone								= 4,	///<A microphone.
		Loudspeaker								= 5,	///<A loudspeaker.
		Headphones								= 6,	///<Headphones.
		PortableAudioDevice						= 7,	///<A portable audio device.
		CarAudio								= 8,	///<A car audio device.
		SetTopBox								= 9,	///<A settop box.
		HiFiAudioDevice							= 10,	///<A HiFi audio device.
		Vcr										= 11,	///<A video cassette recorder.
		VideoCamera								= 12,	///<A video camera.
		Camcorder								= 13,	///<A video camera.
		VideoMonitor							= 14,	///<A video monitor.
		VideoDisplayAndLoudspeaker				= 15,	///<A video display with built - in loudspeaker.
		VideoConferencing						= 16,	///<A video conferencing device.
		GamingDevice							= 18,	///<A gaming device.
	};

	enum MinorComputerClass
	{
		UncategorizedComputer					= 0,	///<An uncategorized computer device.
		DesktopComputer							= 1,	///<A desktop computer.
		ServerComputer							= 2,	///<A server computer.
		LaptopComputer							= 3,	///<A laptop computer.
		HandheldClamShellComputer				= 4,	///<A clamshell handheld computer or PDA.
		HandheldComputer						= 5,	///<A handheld computer or PDA.
		WearableComputer						= 6,	///<A wearable computer.
	};

	enum MinorHealthClass
	{
		UncategorizedHealthDevice				= 0x0,	///<An uncategorized health device.
		HealthBloodPressureMonitor				= 0x1,	///<A blood pressure monitor.
		HealthThermometer						= 0x2,	///<A Thermometer.
		HealthWeightScale						= 0x3,	///<A scale.
		HealthGlucoseMeter						= 0x4,	///<A glucose meter.
		HealthPulseOximeter						= 0x5,	///<A blood oxygen saturation meter.
		HealthDataDisplay						= 0x7,	///<A data display.
		HealthStepCounter						= 0x8,	///<A pedometer.
	};

	enum MinorImagingClass
	{
		UncategorizedImagingDevice				= 0x00,	///<An uncategorized imaging device.
		ImageDisplay							= 0x04,	///<A device capable of displaying images.
		ImageCamera								= 0x08,	///<A camera.
		ImageScanner							= 0x10,	///<An image scanner.
		ImagePrinter							= 0x20,	///<A printer.
	};

	enum MinorMiscellaneousClass
	{
		UncategorizedMiscellaneous				= 0x00,	///<An uncategorized miscellaneous device.
	};

	enum MinorNetworkClass
	{
		NetworkFullService						= 0x00,	///< 100 % of the total bandwidth is available.
		NetworkLoadFactorOne					= 0x08,	///< 0 - 17 % of the total bandwidth is currently being used.
		NetworkLoadFactorTwo					= 0x10,	///< 17 - 33 % of the total bandwidth is currently being used.
		NetworkLoadFactorThree					= 0x18,	///< 33 - 50 % of the total bandwidth is currently being used.
		NetworkLoadFactorFour					= 0x20,	///< 50 - 67 % of the total bandwidth is currently being used.
		NetworkLoadFactorFive					= 0x28,	///< 67 - 83 % of the total bandwidth is currently being used.
		NetworkLoadFactorSix					= 0x30,	///< 83 - 99 % of the total bandwidth is currently being used.
		NetworkNoService						= 0x38,	///< No network service available.
	};

	enum MinorPeripheralClass
	{
		UncategorizedPeripheral					= 0x00,	///< An uncategorized peripheral device.
		KeyboardPeripheral						= 0x10,	///< A keyboard.
		PointingDevicePeripheral				= 0x20,	///< A pointing device, for example a mouse.
		KeyboardWithPointingDevicePeripheral	= 0x30,	///< A keyboard with built - in pointing device.
		JoystickPeripheral						= 0x01,	///< A joystick.
		GamepadPeripheral						= 0x02,	///< A game pad.
		RemoteControlPeripheral					= 0x03,	///< A remote control.
		SensingDevicePeripheral					= 0x04,	///< A sensing device.
		DigitizerTabletPeripheral				= 0x05,	///< A digitizer tablet peripheral.
		CardReaderPeripheral					= 0x06,	///< A card reader peripheral.
	};

	enum MinorPhoneClass
	{
		UncategorizedPhone						= 0,	///<An uncategorized phone device.
		CellularPhone							= 1,	///<A cellular phone.
		CordlessPhone							= 2,	///<A cordless phone.
		SmartPhone								= 3,	///<A smart phone.
		WiredModemOrVoiceGatewayPhone			= 4,	///<A wired modem or voice gateway.
		CommonIsdnAccessPhone					= 5,	///<A device that provides ISDN access.
	};

	enum MinorToyClass
	{
		UncategorizedToy						= 0,	///<An uncategorized toy.
		ToyRobot								= 1,	///<A toy robot.
		ToyVehicle								= 2,	///<A toy vehicle.
		ToyDoll									= 3,	///<A toy doll or action figure.
		ToyController							= 4,	///<A controller.
		ToyGame									= 5,	///<A game.
	};

	enum MinorWearableClass
	{
		UncategorizedWearableDevice				= 0,	///<An uncategorized wearable device.
		WearableWristWatch						= 1,	///<A wristwatch.
		WearablePager							= 2,	///<A pager.
		WearableJacket							= 3,	///<A jacket.
		WearableHelmet							= 4,	///<A helmet.
		WearableGlasses							= 5,	///<A pair of glasses.
	};

	enum ServiceClass
	{
		NoService								= 0x0000,	///< The device does not provide any services.
		PositioningService						= 0x0001,	///<The device provides positioning services.
		NetworkingService						= 0x0002,	///<The device provides networking services.
		RenderingService						= 0x0004,	///<The device provides rendering services.
		CapturingService						= 0x0008,	///<The device provides capturing services.
		ObjectTransferService					= 0x0010,	///<The device provides object transfer services.
		AudioService							= 0x0020,	///<The device provides audio services.
		TelephonyService						= 0x0040,	///<The device provides telephony services.
		InformationService						= 0x0080,	///<The device provides information services.
		AllServices								= 0x07ff,	///< The device provides services of all types.
	};
	Q_ENUM(ServiceClass);
	Q_DECLARE_FLAGS(ServiceClasses, ServiceClass);
	Q_FLAG(ServiceClasses);

public:

	BluetoothDeviceInfo();
	BluetoothDeviceInfo(const BluetoothDevice& other);
	BluetoothDeviceInfo(const BluetoothAddress& address, const QString& name, quint32 classOfDevice);
	BluetoothDeviceInfo(const BluetoothUuid& uuid, const QString& name, quint32 classOfDevice);
	BluetoothDeviceInfo(const BluetoothDeviceInfo& other);
	BluetoothDeviceInfo& operator=(const BluetoothDeviceInfo& other);
	virtual ~BluetoothDeviceInfo();
	
	BluetoothAddress address() const;
	CoreConfigurations coreConfigurations() const;
	BluetoothUuid deviceUuid() const;
	bool isCached() const;
	bool isValid() const;
	MajorDeviceClass majorDeviceClass() const;
	quint8 minorDeviceClass() const;
	QString name() const;
	qint16 rssi() const;
	ServiceClasses serviceClasses() const;
	QList<BluetoothUuid> serviceUuids(DataCompleteness* completeness = Q_NULLPTR) const;
	DataCompleteness serviceUuidsCompleteness() const;
	void setCached(bool cached);
	void setCoreConfigurations(CoreConfigurations coreConfigs);
	void setDeviceUuid(const BluetoothUuid& uuid);
	void setRssi(qint16 signal);
	void setServiceUuids(const QList<BluetoothUuid>& uuids, DataCompleteness completeness);
	bool operator!=(const BluetoothDeviceInfo& other) const;
	bool operator==(const BluetoothDeviceInfo& other) const;
	
private:

	Q_DECLARE_PRIVATE(BluetoothDeviceInfo);
	BluetoothDeviceInfoPrivate*	d_ptr;

};

#endif // bluetoothDeviceInfo_h__
