/*
 * Copyright (C) 2010 The Android Open Source Project
 * Copyright (C) 2019 Gillou68310
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <thread>

#include "SwitchMtpDatabase.h"
#include "MtpServer.h"
#include "MtpStorage.h"

#include "log.h"

using namespace android;

#ifdef WANT_SYSMODULE
extern "C"
{
#define INNER_HEAP_SIZE 0x80000
    extern u32 __start__;

    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char nx_inner_heap[INNER_HEAP_SIZE];

    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}

u32 __nx_applet_type = AppletType_None;

void __libnx_initheap(void)
{
    void *addr = nx_inner_heap;
    size_t size = nx_inner_heap_size;

    extern char *fake_heap_start;
    extern char *fake_heap_end;

    fake_heap_start = (char *)addr;
    fake_heap_end = (char *)addr + size;
}

void __appInit(void)
{
    smInitialize();
    Result rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }
    fsInitialize();
    hidInitialize();
    fsdevMountSdmc();
}

MtpServer* serverExit = NULL;

void __appExit(void)
{
    serverExit->stop();
    usbExit();
    hidExit();
    fsExit();
    smExit();
}
#endif // WANT_SYSMODULE

static void stop_thread(MtpServer* server)
{
#ifdef WANT_APPLET
    padConfigureInput(8, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeAny(&pad);

    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_B)
        {
            server->stop();
            break;
        }
    }
#endif // WANT_APPLET

#ifdef WANT_SYSMODULE
    serverExit = server;
#endif // WANT_SYSMODULE
}

int main(int argc, char* argv[])
{
    int c;
    struct option long_options[] =
    {
      {"nxlink",  no_argument,       &nxlink, 1},
      {"verbose", required_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

    while(1)
    {
        int option_index = 0;
        c = getopt_long (argc, argv, "v:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case 'v':
              verbose_level = atoi(optarg);
              break;
            default:
              break;
        }
    }

#ifdef WANT_APPLET
    consoleInit(NULL);
    std::cout << "MTP Server is running." << std::endl;
    std::cout << "> Press B to exit.";
#endif // WANT_APPLET

    struct usb_device_descriptor device_descriptor = {
        .bLength = USB_DT_DEVICE_SIZE,
        .bDescriptorType = USB_DT_DEVICE,
        .bcdUSB = 0x0110,
        .bDeviceClass = 0x00,
        .bDeviceSubClass = 0x00,
        .bDeviceProtocol = 0x00,
        .bMaxPacketSize0 = 0x40,
        .idVendor = 0x057e,
        .idProduct = 0x4000,
        .bcdDevice = 0x0100,
        .bNumConfigurations = 0x01
    };

    UsbInterfaceDesc infos[2];
    int num_interface = 0;
    USBMtpInterface *mtp_interface = NULL;
    USBSerialInterface *serial_interface = NULL;

    mtp_interface = new USBMtpInterface(num_interface, &infos[num_interface]);
    num_interface++;

    if(nxlink)
    {
      serial_interface = new USBSerialInterface(num_interface, &infos[num_interface]);
      num_interface++;
    }

    usbInitialize(&device_descriptor, num_interface, infos);
    nxlinkStdioInitialise(serial_interface);

    MtpStorage* storage = new MtpStorage(
      MTP_STORAGE_REMOVABLE_RAM,
      "sdmc:/",
      "sdcard",
      1024U * 1024U * 100U,  /* 100 MB reserved space, to avoid filling the disk */
      false,
      1024U * 1024U * 1024U * 4U - 1  /* ~4GB arbitrary max file size */);

    MtpDatabase* mtp_database = new SwitchMtpDatabase();

    mtp_database->addStoragePath("sdmc:/",
                                 "sdcard",
                                 MTP_STORAGE_REMOVABLE_RAM, true);

    MtpServer* server = new MtpServer(
      mtp_interface,
      mtp_database,
      false,
      0,
      0,
      0);

    std::thread th(stop_thread, server);
    server->addStorage(storage);
    server->run();
    th.join();

    nxlinkStdioClose(serial_interface);
    consoleExit(NULL);
    usbExit();
    return 0;
}
