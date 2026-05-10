/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include <string.h>
#include <sys/iosupport.h>

#include "log.h"

int nxlink = 0;

static USBSerialInterface* _usb;

static ssize_t write_stdout(struct _reent *r,void *fd,const char *ptr, size_t len)
{
    return _usb->write(ptr, len);
}

static const devoptab_t dotab_stdout = {
	"usb",
	0,
	NULL,
	NULL,
	write_stdout,
	NULL,
	NULL,
	NULL
};

void nxlinkStdioInitialise(USBSerialInterface* usb)
{
    if(usb != NULL)
    {
        _usb = usb;
        devoptab_list[STD_OUT] = &dotab_stdout;
        devoptab_list[STD_ERR] = &dotab_stdout;
        setvbuf(stdout, NULL , _IONBF, 0);
        setvbuf(stderr, NULL , _IONBF, 0);

        // Wait for start command
        char start[7];
        while(strcmp(start, "#START#") != 0)
        {
            usb->read(start, 7);
        }
    }
}

void nxlinkStdioClose(USBSerialInterface* usb)
{
    if(usb != NULL)
    {
        const char *stop = "#STOP#";
        usb->write(stop, 6);
    }
}
