/* Sample system-dependent communications i/o routines for embedded Kermit. */

/*
  Author: Frank da Cruz.
  Copyright (C) 1995, 2011.
  Trustees of Columbia University in the City of New York.
  All rights reserved.
  See kermit.c for license.
*/

/*
  The sample i/o routines for UNIX that provide packet i/o
  functions on the console (login) device.
  Copy this file, rename it appropriately, and replace the contents
  of each routine appropriately for your platform.

  Device i/o:

    int devopen()    Communications device - open
    int pktmode()    Communications device - enter/exit packet mode
    int readpkt()    Communications device - read a packet
    int tx_data()    Communications device - send data
    int devclose()   Communications device - close
    int inchk()      Communications device - check if bytes are ready to read

  File i/o:

    int openfile()   File - open for input or output
    ULONG fileinfo() Get input file modtime and size
    int readfile()   Input file - read data
    int writefile()  Output file - write data
    int closefile()  Input or output file - close

  Full definitions below, prototypes in kermit.h.

  These routines must handle speed setting, parity, flow control, file i/o,
  and similar items without the kermit() routine knowing anything about it.
  If parity is in effect, these routines must add it to outbound characters
  and strip it from inbound characters.
*/
#include "typedefs.h"
#include "cdefs.h"
#include "ekermit_debug.h"
#include "kermit_platform.h"
#include "kermit.h"
#include "config.h"
#include "uart.h"
#include "string.h"
#include "flash_drv.h"
#include "hal.h"
#include "../WolfBoot/include/target.h"

#include "leuart.h"
#ifdef F_SCAN
#undef F_SCAN
#endif
// TODO: configure in runtime
#if KERMIT_IO == KERMIT_IO_RS232
#define _IO_RECV(buf, len) uart_recv(0, buf, len, (u32)(-1))
#define _IO_SEND(buf, len) uart_send(0, buf, len)
#elif KERMIT_IO == KERMIT_IO_LEUART
#define _IO_RECV(buf, len) leuart_recv(0, buf, len, (u32)(-1))
#define _IO_SEND(buf, len) leuart_send(0, buf, len)
#endif

UCHAR o_buf[OBUFLEN + 8]; /* File output buffer */
UCHAR i_buf[IBUFLEN + 8]; /* File output buffer */

#ifdef RAW_UPDATE_PARTITION
// uint32_t flash_position_pointer = KERMIT_FLASH_START;
uint32_t flash_position_pointer = WOLFBOOT_PARTITION_BOOT_ADDRESS;
#else
/*
  In this example, the output file is unbuffered to ensure that every
  output byte is commited.  The input file, however, is buffered for speed.
  This is just one of many possible implmentation choices, invisible to the
  Kermit protocol module.
*/
static FS_FILE *ifile = NULL; /* and pointers */
static FS_FILE *ofile = NULL; /* and pointers */
#endif
uint32_t t0, dt;
/*  D E V O P E N  --  Open communications device  */
/*

  Call with: string pointer to device name.  This routine should get the
  current device settings and save them so devclose() can restore them.
  It should open the device.  If the device is a serial port, devopen()
  set the speed, stop bits, flow control, etc.
  Returns: 0 on failure, 1 on success.
*/
int devopen(char *device)
{
    return (1);
}

/*  P K T M O D E  --  Put communications device into or out of packet mode  */
/*
  Call with: 0 to put in normal (cooked) mode, 1 to put in packet (raw) mode.
  For a "dumb i/o device" like an i/o port that does not have a login attached
  to it, this routine can usually be a no-op.
  Returns: 0 on failure, 1 on success.
*/
int pktmode(short on)
{
    return (1);
}

/*  D E V S E T T I N G S  */

int devsettings(char *s)
{
    return (1);
}

/*  D E V R E S T O R E  */

int devrestore(void)
{
    return (1);
}

/*  D E V C L O S E  --  Closes the current open communications device  */
/*
  Call with: nothing
  Closes the device and puts it back the way it was found by devopen().
  Returns: 0 on failure, 1 on success.
*/
int devclose(void)
{
    return (1);
}

/* I N C H K  --  Check if input waiting */

/*
  Check if input is waiting to be read, needed for sliding windows.  This
  sample version simply looks in the stdin buffer (which is not portable
  even among different Unixes).  If your platform does not provide a way to
  look at the device input buffer without blocking and without actually
  reading from it, make this routine return -1.  On success, returns the
  numbers of characters waiting to be read, i.e. that can be safely read
  without blocking.
*/
int inchk(struct k_data *k)
{
    return (-1);
}

/*  R E A D P K T  --  Read a Kermit packet from the communications device  */
/*
  Call with:
    k   - Kermit struct pointer
    p   - pointer to read buffer
    len - length of read buffer

  When reading a packet, this function looks for start of Kermit packet
  (k->r_soh), then reads everything between it and the end of the packet
  (k->r_eom) into the indicated buffer.  Returns the number of bytes read, or:
     0   - timeout or other possibly correctable error;
    -1   - fatal error, such as loss of connection, or no buffer to read into.
*/

int readpkt(struct k_data *k, UCHAR *p, int len, int fc)
{
    int n;
    short flag;
    UCHAR c;
/*
  Timeout not implemented in this sample.
  It should not be needed.  All non-embedded Kermits that are capable of
  making connections are also capable of timing out, and only one Kermit
  needs to time out.  NOTE: This simple example waits for SOH and then
  reads everything up to the negotiated packet terminator.  A more robust
  version might be driven by the value of the packet-length field.
*/
#ifdef EKERMIT_DEBUG
    char *p2;
#endif /* EKERMIT_DEBUG */

#ifdef F_CTRLC
    short ccn;
    ccn = 0;
#endif /* F_CTRLC */

    flag = n = 0; /* Init local variables */

#ifdef EKERMIT_DEBUG
    p2 = p;
#endif /* EKERMIT_DEBUG */

    while (1)
    {
        uint32_t rcv = 0;
        rcv = _IO_RECV(&c, 1);
        if (rcv == 1)
        {
#ifdef F_CTRLC
            /* In remote mode only: three consecutive ^C's to quit */
            if (k->remote && c == (UCHAR)3)
            {
                if (++ccn > 2)
                {
                    debug(DB_MSG, "readpkt ^C^C^C", 0, 0);
                    break;
                }
            }
            else
            {
                ccn = 0;
            }
#endif /* F_CTRLC */

            if (!flag && c != k->r_soh) /* No start of packet yet */
                continue;               /* so discard these bytes. */
            if (c == k->r_soh)
            {             /* Start of packet */
                flag = 1; /* Remember */
                continue; /* But discard. */
            }
            else if (c == k->r_eom  /* Packet terminator */
                     || c == '\012' /* 1.3: For HyperTerminal */
            )
            {
#ifdef EKERMIT_DEBUG
                *p = NUL; /* Terminate for printing */
                debug(DB_PKT, "RPKT", p2, n);
#endif /* EKERMIT_DEBUG */
                return (n);
            }
            else
            {                          /* Contents of packet */
                if (n++ > k->r_maxlen) /* Check length */
                    return (0);
                else
                    *p++ = c & 0xff;
            }
        }
    }
    return (-1);
}

/*  T X _ D A T A  --  Writes n bytes of data to communication device.  */
/*
  Call with:
    k = pointer to Kermit struct.
    p = pointer to data to transmit.
    n = length.
  Returns:
    X_OK on success.
    X_ERROR on failure to write - i/o error.
*/
int tx_data(struct k_data *k, UCHAR *p, int n)
{
    if (!_IO_SEND(p, n))
    {
        return (X_ERROR);
    }
    return (X_OK); /* Success */
}

/*  O P E N F I L E  --  Open output file  */
/*
  Call with:
    Pointer to filename.
    Size in bytes.
    Creation date in format yyyymmdd hh:mm:ss, e.g. 19950208 14:00:00
    Mode: 1 = read, 2 = create, 3 = append.
  Returns:
    X_OK on success.
    X_ERROR on failure, including rejection based on name, size, or date.
*/
int openfile(struct k_data *k, UCHAR *s, int mode)
{
if (mode == 2)
{
    hal_flash_erase(WOLFBOOT_PARTITION_BOOT_ADDRESS, WOLFBOOT_PARTITION_SIZE);
    return (int) (X_OK);
}
return (int )(X_ERROR);
}

/*  W R I T E F I L E  --  Write data to file  */
/*
  Call with:
    Kermit struct
    String pointer
    Length
  Returns:
    X_OK on success
    X_ERROR on failure, such as i/o error, space used up, etc
*/
int writefile(struct k_data *k, UCHAR *s, int n)
{
    int rc;
    rc = X_ERROR;
    t0 = RTC->CNT;
    if(hal_flash_write(flash_position_pointer, (char const *) s, n) == 0)
    {
      flash_position_pointer += n;
      rc = X_OK;
    }
    dt = RTC->CNT - t0;
    return (rc);
}

/*  C L O S E F I L E  --  Close output file  */
/*
  Mode = 1 for input file, mode = 2 or 3 for output file.

  For output files, the character c is the character (if any) from the Z
  packet data field.  If it is D, it means the file transfer was canceled
  in midstream by the sender, and the file is therefore incomplete.  This
  routine should check for that and decide what to do.  It should be
  harmless to call this routine for a file that that is not open.
*/
int closefile(struct k_data *k, UCHAR c, int mode)
{
    int rc = X_OK; /* Return code */
    return (rc);
}