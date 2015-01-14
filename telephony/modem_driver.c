/* Copyright (C) 2007-2008 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/
/* implement the modem character device for Android within the QEMU event loop.
 * it communicates through a serial port with "rild" (Radio Interface Layer Daemon)
 * on the emulated device.
 */
#include "modem_driver.h"
#include "sysemu/char.h"
#include "sysdeps.h"

#define  xxDEBUG

#ifdef DEBUG
#  include <stdio.h>
#  define  D(...)   ( fprintf( stderr, __VA_ARGS__ ) )
#else
#  define  D(...)   ((void)0)
#endif

CharDriverState*  android_modem_cs[MAX_GSM_DEVICES];

typedef struct {
    CharDriverState*  cs;
    AModem            modem;
    char              in_buff[ 1024 ];
    int               in_pos;
    int               in_sms;
} ModemDriver;

static ModemDriver  modem_driver[MAX_GSM_DEVICES];

/* send unsollicited messages to the device */
static void
modem_driver_unsol( void*  _md, const char*  message)
{
    ModemDriver*      md = _md;
    int               len = strlen(message);

    qemu_chr_write(md->cs, (const uint8_t*)message, len);
}

static int
modem_driver_can_read( void*  _md )
{
    ModemDriver*  md  = _md;
    int           ret = sizeof(md->in_buff) - md->in_pos;

    return ret;
}

/* despite its name, this function is called when the device writes to the modem */
static void
modem_driver_read( void*  _md, const uint8_t*  src, int  len )
{
    ModemDriver*      md  = _md;
    const uint8_t*    end = src + len;
    int               nn;

    D( "%s: reading %d from %p bytes:", __FUNCTION__, len, src );
    for (nn = 0; nn < len; nn++) {
        int  c = src[nn];
        if (c >= 32 && c < 127)
            D( "%c", c );
        else if (c == '\n')
            D( "<LF>" );
        else if (c == '\r')
            D( "<CR>" );
        else
            D( "\\x%02x", c );
    }
    D( "\n" );

    for ( ; src < end; src++ ) {
        char  c = src[0];

        if (md->in_sms) {
            if (c != 26)
                goto AppendChar;

            md->in_buff[ md->in_pos ] = c;
            md->in_pos++;
            md->in_sms = 0;
            c = '\n';
        }

        if (c == '\n' || c == '\r') {
            const char*  answer;

            if (md->in_pos == 0)  /* skip empty lines */
                continue;

            md->in_buff[ md->in_pos ] = 0;
            md->in_pos                = 0;

            D( "%s: << %s\n", __FUNCTION__, md->in_buff );
            md->in_sms = amodem_send(md->modem, md->in_buff);

            continue;
        }
    AppendChar:
        md->in_buff[ md->in_pos++ ] = c;
        if (md->in_pos == sizeof(md->in_buff)) {
            /* input is too long !! */
            md->in_pos = 0;
        }
    }
    D( "%s: done\n", __FUNCTION__ );
}


static void
modem_driver_init( int  base_port, int  instance_id, ModemDriver*  dm, CharDriverState*  cs )
{
    dm->cs     = cs;
    dm->in_pos = 0;
    dm->in_sms = 0;
    dm->modem  = amodem_create( base_port, instance_id, modem_driver_unsol, dm );

    qemu_chr_add_handlers( cs, modem_driver_can_read, modem_driver_read, NULL, dm );
}


void android_modem_init( int  base_port )
{
    bool sys_main_inited = false;
    int i;

    for (i = 0; i < amodem_num_devices; i++) {
        if (android_modem_cs[i] == NULL) {
            continue;
        }

        // Moved from amodem_create() because now we may invoke amodem_create()
        // several times in modem_driver_init().
        if (!sys_main_inited) {
            sys_main_inited = true;
            sys_main_init();
        }

        modem_driver_init( base_port, i, &modem_driver[i], android_modem_cs[i] );
    }
}


AModem
amodem_get_instance( int  instance_id )
{
    if (android_modem_cs[instance_id])
        return modem_driver[instance_id].modem;

    return NULL;
}
