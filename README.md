# win-bluetooth
A set of modern c++ classes for using bluetooth on windows. Based on the Qt Bluetooth interface. C++17.

[![Windows build](https://ci.appveyor.com/api/projects/status/5ym4bmwl8fbutgrl?svg=true&branch=master)](https://ci.appveyor.com/project/nholthaus/win-bluetooth) ![license](https://img.shields.io/badge/license-MIT-orange.svg) ![copyright](https://img.shields.io/badge/%C2%A9-Nic_Holthaus-orange.svg) ![language](https://img.shields.io/badge/language-c++-blue.svg) ![c++](https://img.shields.io/badge/std-c++17-blue.svg) ![msvc2017](https://img.shields.io/badge/MSVC-2017-ff69b4.svg)

# Follows the Qt Bluetooth class interface
The intent is to create a bluetooth library using Qt and winsock to fill the capability gap in Qt 5.10. Class APIs by design will be similar/identical to their Qt counterparts, and a user familiar with QtBluetooth should be able to use win-bluetooth without (much of) a learning curve. 

win-bluetooth uses the same class structure, just without the `Q` to start the class names. E.g. the equivalent for `QBluetoothSocket` would be `BluetoothSocket`.

[Qt Bluetooth Documentation](http://doc.qt.io/qt-5/qtbluetooth-module.html)

## How to run the tests
To tun the unit tests, you will need two windows PC's with bluetooth capability in range of each other. We'll call them the "server" PC and the "client" PC. 
1. Start the client PC first with the following command line options:

    `win-bluetooth-test.exe [LOCAL PC HOSTNAME] [LOCAL PC BLUETOOTH ADDRESS] [REMOTE PC HOSTNAME] [REMOTE PC BLUETOOTH ADDRESS] -c`

    example:
    > win-bluetooth-test.exe RELENTLESS 9C:B6:D0:DF:20:2C DAUNTLESS 5C:F3:70:84:BD:8F -c

2. Start the server PC next with the following command line options:

    `win-bluetooth-test.exe [LOCAL PC HOSTNAME] [LOCAL PC BLUETOOTH ADDRESS] [REMOTE PC HOSTNAME] [REMOTE PC BLUETOOTH ADDRESS]`

    example:
    > win-bluetooth-test.exe DAUNTLESS 5C:F3:70:84:BD:8F RELENTLESS 9C:B6:D0:DF:20:2C

## How to figure out your devices bluetooth address

On windows:
1. Open `Device Mananger`.
2. Find you remote devices name under the `Bluetooth` submenu and double-click it.
3. Select the `Details` tab.
4. Select `Bluetooth device address` from the `Properties` menu.
5. That's the address to use, just remember to insert a colon (`:`) between each set of two hex values.
