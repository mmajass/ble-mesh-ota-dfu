# Example Ble Mesh OTA DFU
## Overview

This is an example shows:
 
*  How to use serial interface for a gateway to work with nodes in a bluetooth mesh network
*  How to do DFU over mesh network


## Background
The system architecture:

*  The eartags will advertise the beacon packet
*  Beacon scanner (central devices) will scan it and organize it, and also perform the DFU for all the nodes
*  The gateway can get those information through bluetooth mesh and also perform the DFU for all the beacon scanner

In this example project, we will focus on how gateway exchanges data with beacon scanner, and also how gateway do DFU for the beacon scanner.

## What are inside this example project?
There are two project and one precompiled tool insided:

*  Beacon scanner
    *  The code for beacon scanner
    *  The provisioner can send command to turn on/off the LED1 on beacon scanner
    *  When the user press the button1, the beacon scanner will publish a message with 16 bytes user customized string to the provisioner
*  Serial interface
    *  The code for gateway, also be the provisioner
    *  Need to run with PyACI interface for sending commands to beacon scanners, or receiving the data from beacon scanners
    *  Need to run with nrfutil for doing the DFU to all the beacon scanners
*  nRFUtil for mesh
    *  A special version precompiled executable tool for packaging and distribute the DFU firmware through mesh

## What can this example project do?
In this example project, we demonstrate:

*  Control the beacon scanners through serial interface
	*  Provisioning and configuration
	*  Control beacon scanners
	*  Switch on/off the LED1 on all the beacon scanners
	*  Receive the result from the beacon scanners
*  DFU 
	*  Initial the DFU procedure on the serial interface
	*  Upgrade the firmware on all the beacon scanner in “side-by-side” DFU method

## Suggested reference material

1. [Quick Start Guide for the nRF5 SDK for Mesh](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.meshsdk.v2.2.0/md_doc_getting_started_getting_started.html?cp=4_1_0_1)
2. [Basic Bluetooth Mesh concepts](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.meshsdk.v2.2.0%2Fmd_doc_introduction_basic_concepts.html&cp=4_1_0_0_5) 
3. [Interactive PyACI](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.meshsdk.v2.2.0%2Fmd_scripts_interactive_pyaci_README.html&cp=4_1_0_3_0) 
4. [DFU quick start guide](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.meshsdk.v2.2.0%2Fmd_doc_getting_started_dfu_quick_start.html&cp=4_1_0_1_2)

## Prerequisite hardware:

1. At least two nRF52840-DK
*	One as a serial interface
2. The other(s) as beacon scanners
*	One PC for connecting with the nRF52840-DK which running serial interface

## Install the example project

1. Finish the mesh SDK setup in [here](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.meshsdk.v2.2.0%2Fmd_doc_getting_started_getting_started.html&cp=4_1_0_1) 
2. Create a directory under nrf5_SDK_for_Mesh_v2.2.0_src\examples and clone the project repository
3. Program one or multiple nRF52840-DK with “beacon_scanner”
4. Program one nRF52840-DK with “serial_interface”


## Bluetooth Mesh demonstration
The "beacon scanner" example is based on the "server" in the "light switch" in our mesh SDK v2.2.0 example. Please follow the instruction shows in [here](https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.meshsdk.v2.2.0/md_scripts_interactive_pyaci_doc_demo_configuration.html?cp=4_1_0_3_0_2) for using the serial interface to do provision, configuration and control devices through Bluetooth mesh network.

## DFU demonstration
Both of the "serial_interface" and "beacon_scanner" project have been added with DFU function, which means they can been upgraded the firmware through mesh.
Please note that, this DFU feature we are using in here is a Nordic proprietary feature. It is a “side-by-side” DFU, which means during the DFU procedure, all the devices can still run the normal application, the firmware will collect every pieces of DFU packets, and jump into DFU until all the pieces are received. Also, each device can decide been DFU or not depends on the current DFU packets version.

To make a project possible to run DFU, we must do the following setup

1. Program the softdevice
2. Program the bootloader

	*    All of the API we are using for DFU are provided by bootloader, also the bootloader will help us to reprogram the flash after we collected the entire DFU image

3. Program the application
4. Program the device page

	*    The device page will describe the current version of the application/softdevice and bootloader which current device is using
	*    This can help the device to decide to accept the current received DFU packets or just relay it
	*    We have to use a tool to generate the correct device page, will show in the follow

5. Use a special version of nrfutil to run the inital the DFU on the host side


For running the entire procedure, we can reference the guide in [here](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.meshsdk.v2.2.0%2Fmd_doc_getting_started_dfu_quick_start.html), but with some customized configuration, shows in the following

1. Prepare and program the default firmware for both the beacon scanner and serial interface

    *	Configure the device page in "beacon_scanner" with using 

    	*	"application -id = 1" 
    	*	"application-version = 1"

    *	Configure the device page in "serial_interface" with using 

    	*	"application-id = 2" 
    	*	"application-version = 1"

    *	Program the softdevice, bootloader, application and also device page into device

2. Prepare the DFU firmware for the beacon scanner devices

    *	We can do some tiny modification on the "beacon_scanner"

3. Using the nrfutil to pack the DFU firmware
    e.g.
    ```
    nrfutil dfu genpkg --application .\beacon_scanner_s140_6_0_0.hex --company-id 0x59 --application-id 1 --application-version 2 --key-file .\private_key.txt --sd-req 0xA9 --mesh beacon_scanner_DFU.zip 
    ```
4. Using the nrfutil to transmit the DFU firmware
    e.g.
    ```
    nrfutil.exe --verbose dfu serial -pkg .\beacon_scanner_DFU.zip -p COM10 -b 115200 -fc --mesh -i 200
    ```
5. And we can wait the nrfutil to finish the DFU firmware transportation.

