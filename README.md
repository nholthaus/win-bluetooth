# win-bluetooth
A set of modern c++ classes for using bluetooth on windows

This library is still a work in progress.

The intent is to create a bluetooth library using Qt and winsock to fill the capability gap in Qt 5.10. Class APIs by design will be similar/identical to their Qt counterparts, and a user familiar with QtBluetooth should be able to use win-bluetooth without (much of) a learning curve.

## How to run the tests
To tun the unit tests, you will need two windows PC's with bluetooth capability in range of each other. We'll call them the "server" PC and the "client" PC. 
1. Start the client PC first with the following command line options:

    `win-bluetooth-test.exe [LOCAL PC HOSTNAME] [LOCAL PC BLUETOOTH ADDRESS] [REMOTE PC HOSTNAME] [REMOTE PC BLUETOOTH ADDRESS] -c`

    example:
    > win-bluetooth-test.exe RELENTLESS 9C:B6:D0:DF:20:2C DAUNTLESS 5C:F3:70:84:BD:8F -c

2. Start the server PC next with the following command line options:

    `win-bluetooth-test.exe [LOCAL PC HOSTNAME] [LOCAL PC BLUETOOTH ADDRESS] [REMOTE PC HOSTNAME] [REMOTE PC BLUETOOTH ADDRESS] -c`

    example:
    > win-bluetooth-test.exe DAUNTLESS 5C:F3:70:84:BD:8F RELENTLESS 9C:B6:D0:DF:20:2C

## How to figure out your devices bluetooth address

On windows:
1. Open `Device Mananger`.
2. Find you remote devices name under the `Bluetooth` submenu and double-click it.
3. Select the `Details` tab.
4. Select `Bluetooth device address` from the `Properties` menu.
5. That's the address to use, just remember to insert a colon (`:`) between each set of two hex values.
