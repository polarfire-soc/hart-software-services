#include <stdint.h>
#include <string.h>
#include <ctype.h>

/* TCP/IP stack */
#include "mcc_tcpip_lite/tcpip_types.h"
#include "mcc_tcpip_lite/tcpip_config.h"
#include "mcc_tcpip_lite/tftp.h"
#include "mcc_tcpip_lite/tftp_handler_table.h"
#include "mcc_tcpip_lite/ipv4.h"
#include "mcc_tcpip_lite/udpv4.h"
#include "mcc_tcpip_lite/network.h"
#include "mcc_tcpip_lite/ip_database.h"
#include "mcc_tcpip_lite/arpv4.h"
#include "mcc_tcpip_lite/log.h"

#include "ftp_client.h"



#include <assert.h>

#define FTP_VERBOSE

/*
 * Our FTP data structure etc.
 */

static FtpInfo ftp;
static int32_t got_data_file;
/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.

   This is a port of the Dynamic C code to LwIP.
*/
/*
 *    FTP_CLIENT.lib
 *  A File Transfer Protocol (FTP) client subroutines.
 *
 *  Defines:
 *      FTP_VERBOSE - Adds some diagnostic and progress printf() output.
 *
 *      FTP_MAX_NAMELEN - Max string size for username and password.
 *      FTP_MAX_FNLEN - Max string size for file name.
 *      FTP_MAX_DIRLEN - Max string size for directory name.
 *
 */

 /*****************************************************************
  * The readln interface:                                         *
  *                                                               *
  * ftpc_readln_init() must be called first, to start the process.*
  * ftpc_readln_tick() should be called to drive readln.          *
  * ftpc_readln()      will return the line itself (char *)       *
  *****************************************************************/

/*==============================================================================
 *
 */
void ftpc_readln_init(void)
{
    ftp.readln_is_ready = 0;
    ftp.pline           = ftp.line;
}

/*==============================================================================
 *
 * Fetches either 1 or 2 bytes from the command TCP stream. Returns 0 if no data
 * or not enough data at the moment. Supports single byte push back in case 2
 * byte value straddels a packet boundary.
 */
int16_t ftpc_readln_get(uint8_t *buffer, uint16_t requested)
{
    uint16_t ret_count = 0U;
    uint16_t retval;
    uint8_t *write_ptr = buffer;

    if((2 == requested) && (1 == ftp.s_rx_len)) /* Not enough available... */
    {
        /* Push back character and wait for more data to arrive when we are called later */
        ftp.s_pushback = (uint16_t)*ftp.s_in_ptr & 0x00FFU;
        ftp.s_rx_len = 0;
    }

    if(0 == ftp.s_rx_len) /* Nothing left to process */
    {
        retval = TCP_GetRxLength(&ftp.s); /* See if there is any data available */
        if(0 != retval)
        {
            /*
             * Fetch current buffer and queue alternate buffer for follow on data.
             * We use the second byte of the buffer as the starting point in case
             * we need to add in a pushed back character.
             */

            retval = TCP_GetReceivedData(&ftp.s);
            if(ftp.s_use_buf1)
            {
                ftp.s_in_ptr = ftp.s_rx_buf1;
                ftp.s_rx_len = retval;
                TCP_InsertRxBuffer(&ftp.s, ftp.s_rx_buf2 + 1, sizeof(ftp.s_rx_buf2) - 1);
                ftp.s_use_buf1 = false;
            }
            else
            {
                ftp.s_in_ptr = ftp.s_rx_buf2;
                ftp.s_rx_len = retval;
                TCP_InsertRxBuffer(&ftp.s, ftp.s_rx_buf1 + 1, sizeof(ftp.s_rx_buf1) - 1);
                ftp.s_use_buf1 = true;
            }

            if(0xFFFFU != ftp.s_pushback)
            {
                /* Stuff pushed back character into start of buffer */
                ftp.s_rx_len++;
                ftp.s_in_ptr--;
                *ftp.s_in_ptr = (uint8_t)ftp.s_pushback;
                ftp.s_pushback = 0xFFFFU;
            }

        }
    }

    if((2 == requested) && (1 == ftp.s_rx_len)) /* Not enough available... */
    {
        /* Push back character and wait for more data to arrive when we are called later */
        ftp.s_pushback = (uint16_t)*ftp.s_in_ptr & 0x00FFU;
        ftp.s_rx_len = 0;
    }
    else
    {
        /* If we get to here we can have the following:
         *
         * available == 0 and not ok
         * available >= 1 and requested == 1 and ok
         * available >= 2 and requested == 1 or 2 also ok
         */
        if(0 != ftp.s_rx_len) /* Ok to return data */
        {
            ret_count = requested;
            *write_ptr = *ftp.s_in_ptr++;
            if(2 == requested)
            {
                write_ptr++;
                *write_ptr = *ftp.s_in_ptr++;
            }

            ftp.s_rx_len -= requested;
        }
    }

    return(ret_count);
}

/*==============================================================================
 *
 */
void ftpc_readln_tick(void)
{
    static int retval;
    static unsigned char buf[3];
    int sock_error;
    socketState_t sock_state;

    if(ftp.readln_is_ready)
        return;     /* old line hasn't been used yet */

    while(1)
    {
        /* Check health of socket... */
        sock_state = TCP_SocketPoll(&ftp.s);
        if(SOCKET_CONNECTED != sock_state)
        {
            /* Command socket expired: reset state. */
#ifdef FTP_VERBOSE
            printf("FTPC: ftpc_readln_tick() - Command socket failure\n");
            printf("FTPC: ftpc_readln_tick() - socket state %d\n", sock_state);
#endif
            FTP_SETSTATE(FTP_BAIL);
            retval = 0;
        }

        if(0 == ftp.s_rx_len) /* Nothing left to process */
        {
            retval = TCP_GetRxLength(&ftp.s); /* See if there is any data available */
            if(0 != retval)
            {
                /* Fetch current buffer and queue alternate buffer for follow on data */
                retval = TCP_GetReceivedData(&ftp.s);
                if(ftp.s_use_buf1)
                {
                    ftp.s_in_ptr = ftp.s_rx_buf1;
                    ftp.s_rx_len = retval;
                    TCP_InsertRxBuffer(&ftp.s, ftp.s_rx_buf2, sizeof(ftp.s_rx_buf2));
                    ftp.s_use_buf1 = false;
                }
                else
                {
                    ftp.s_in_ptr = ftp.s_rx_buf2;
                    ftp.s_rx_len = retval;
                    TCP_InsertRxBuffer(&ftp.s, ftp.s_rx_buf1, sizeof(ftp.s_rx_buf1));
                    ftp.s_use_buf1 = true;
                }
            }
        }

        retval = ftpc_readln_get(buf, 1);
        if(0 == retval)
            return; /* no data; wait till next tick */

        /* strip off all telnet control codes */
        if (buf[0] == 255)
        {
            retval = ftpc_readln_get(buf, 2);

#if 0
            if(retval < 0)
            {
#ifdef FTP_VERBOSE
            printf("FTPC: ftpc_readln_tick() 2 - lwip_recvfrom() failure\n");
            //sock_perror(&ftp.sock, NULL);
#endif
                // Command socket expired: reset state.
                FTP_SETSTATE(FTP_BAIL);
            }
#endif
            if(2 == retval)
            {
                switch (buf[0])
                {
                // The writes look strange, but we must respond to some telnet codes.
                case 251:
                case 252:
                   buf[2] = buf[1];
                   *(uint16_t *)buf = 0xFEFF; // Todo: PMCS check for endianess...
                   sock_state = TCP_Send(&ftp.s, buf, 3);
                    if (sock_state != NET_SUCCESS)
                    {
#ifdef FTP_VERBOSE
                        printf("FTPC: ftpc_readln_tick() 3 - TCP_Send() failure\n");
#endif
                        // Command socket expired: reset state.
                        FTP_SETSTATE(FTP_BAIL);
                    }
                    continue;

                case 253:
                case 254:
                    buf[2] = buf[1];
                   *(uint16_t *)buf = 0xFCFF; // Todo: PMCS check for endianess...
                   sock_state = TCP_Send(&ftp.s, buf, 3);
                    if (sock_state != NET_SUCCESS)
                    {
#ifdef FTP_VERBOSE
                        printf("FTPC: ftpc_readln_tick() 4 - TCP_Send() failure\n");
#endif
                        // Command socket expired: reset state.
                        FTP_SETSTATE(FTP_BAIL);
                    }
                    continue;
                case 255:
                    break;
                default:
                    continue;   /* ignore command */
                }
            }
        }

        /* data byte is good; add it to the list */
        *ftp.pline = (char)buf[0];
        ftp.pline++;
        if (buf[0] == '\n')
            break;
    }

    /* clean it up */

    /* strip off EOLN */
    ftp.pline--;
    *ftp.pline = '\0';
    ftp.pline--;
    *ftp.pline = '\0';

#ifdef FTP_VERBOSE
    printf("FTPC: got '%s'\n",ftp.line);
#endif
    ftp.readln_is_ready = 1;

    // if it is a multi-line message, skip it
    if (ftp.line[3] == '-' || ftp.line[0] == ' ')
    {
        ftpc_readln_init();
    }
    else
    {
       /* Else parse the 3 initial digits into ints to avoid string tests */
       ftp.code1 = ftp.line[0] - '0';
       ftp.code = ftp.code1*100 + (ftp.line[1] - '0')*10 + (ftp.line[2] - '0');
    }
}

/*==============================================================================
 *
 */
char *ftpc_readln(void)
{
    ftpc_readln_init(); /* reset readln */

    return ftp.line;
}

/* End of readln interface */
/******************************************************/

/*** Message send stuff. ***/
/*==============================================================================
 * Send a message over the command socket to the FTP client
 */
void ftpc_msg(char *str, int nextstate)
{
    struct sockaddr_in our_addr;
    socklen_t addr_len;

    ftp.message = str;
    ftp.msglen = (int)strlen(ftp.message);
    ftp.msgcomplete = lwip_send(ftp.s, ftp.message, (size_t)ftp.msglen, MSG_DONTWAIT);
#ifdef FTP_VERBOSE
    printf("FTPC: sending %s", ftp.message);
#endif
    if (ftp.msgcomplete < 0)
    {
#ifdef FTP_VERBOSE
        printf("FTPC: ftpc_msg() - lwip_send() failure\n");
        //sock_perror(&ftp.sock, NULL);
#endif
        FTP_SETSTATE(FTP_BAIL);
        return;
    }
    if (ftp.msgcomplete < ftp.msglen)
    {
        FTP_SETSTATE2(FTP_MSGWAIT, nextstate);
        return;
    }

//    sock_flush(ftp.s);
    ftp.msgcomplete = lwip_send(ftp.s, ftp.message, 0, 0); /* PMCS try 0 length blocking write to flush? */

// PMCS need to figure out how to do this check for socket still alive...
//    if(!tcp_tick(ftp.s))
//    FTP_SETSTATE2(FTP_DONE2, nextstate);   // socket was closed immediately after QUIT
//    else
    if(getpeername(ftp.s, (struct sockaddr *)&our_addr, &addr_len))
    {
        FTP_SETSTATE2(FTP_DONE2, nextstate);   // socket was closed immediately after QUIT
    }
    else
    {
        FTP_SETSTATE2(FTP_GETLINE, nextstate);  // now this only occurs if the socket is still open
    }
}

/*==============================================================================
 * finish sending the message, if sock_fastwrite didn't finish the first time
 */
void ftpc_finish_msg(void)
{
    auto int rc;
    rc = send(ftp.s, ftp.message + ftp.msgcomplete, (size_t)(ftp.msglen - ftp.msgcomplete), MSG_DONTWAIT);
    if (rc < 0)
    {
#ifdef FTP_VERBOSE
        printf("FTPC: ftpc_finish_msg() - lwip_send() failure\n");
        //sock_perror(&ftp.sock, NULL);
#endif
        FTP_SETSTATE(FTP_BAIL);
        return;
    }
    ftp.msgcomplete += rc;
    if(ftp.msgcomplete < ftp.msglen)
        return;
    // Trick here to avoid 3-length queue.  Set current state to GETLINE without
    // disturbing the queued next state.
    ftp.state &= 0xFF00;
    ftp.state |= FTP_GETLINE;
//    sock_flush(ftp.s);
    ftp.msgcomplete = send(ftp.s, ftp.message, 0, 0); /* PMCS try 0 length blocking write to flush? */
}

#if 0
/* START FUNCTION DESCRIPTION ********************************************
ftp_client_setup_url                   <FTP_CLIENT.LIB>

SYNTAX:         int ftp_client_setup_url( const char far *url, int mode,
                            char *buffer, int length);

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Sets up a FTP transfer. It is called first, then
               ftp_client_tick is called until it returns non-zero.

PARAMETER1:    URL to download
PARAMETER2:    Mode of transfer: FTP_MODE_UPLOAD or FTP_MODE_DOWNLOAD.
               You may also OR in the value FTP_MODE_PASSIVE to use
               passive mode transfer (important if you are behind a
               firewall).

               Use FTP_MODE_GETLIST if you just want to retrieve the file
               information given by "LIST filename." If FTP_MODE_GETLIST is
               used with a NULL filename the, the results of "LIST" are
               given. These results may just be a list of file names, or
               they may contain more information with each file - this is
               server-dependent.
PARAMETER3:    Buffer to get/put the file to/from.  Must be NULL if
               a data handler function will be used.  See ftp_data_handler
               for more details.
PARAMETER4:    On upload, length of file; on download size of buffer.
               This parameter limits the transfer size to a maximum of
               32767 bytes.  For larger transfers, it will be necessary
               to use a data handler function.

RETURN VALUE:   0: Success
                    -NETERR_DNSERROR: Couldn't resolve hostname from URL.
                    -NETERR_HOST_REFUSED: Couldn't connect to FTP server.
               -EINVAL: Error parsing URL
               -E2BIG: URL is too big to parse.  Increase URL_MAX_BUFFER_SIZE
                     to handle larger URLs.

SEE ALSO:      ftp_client_setup, ftp_client_tick, ftp_data_handler

END DESCRIPTION **********************************************************/
int ftp_client_setup_url( const char *url, int mode, char *buffer, int length)
{
    url_parsed_t parsed;
    int error;
    unsigned long ip;

    error = url_parse( &parsed, url);
    if (! error)
    {
        if ( !(ip = resolve( parsed.hostname)) )
        {
            error = -NETERR_DNSERROR;
        }
        else
        {
          if (!*parsed.username)
          {
             parsed.username = "ftp";
          }
          else if (*parsed.password)
            {
                // split userinfo into username and password
                parsed.password[-1] = '\0';
            }
            if (*parsed.filename)
            {
                // split path into path and filename
                parsed.filename[-1] = '\0';
            }
          error = ftp_client_setup (ip, parsed.port, parsed.username,
            parsed.password, mode, parsed.filename, parsed.path,
            buffer, length);
          if (error)
          {
                error = -NETERR_HOST_REFUSED;
          }
        }
    }

    return error;
}
#endif

/* START FUNCTION DESCRIPTION ********************************************
ftp_client_setup                   <FTP_CLIENT.LIB>

SYNTAX: int ftp_client_setup(
                long host, int port, const char far *username,
                const char far *password, int mode, const char far *filename,
                const char far *dir, char *buffer, int length
                );

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Sets up a FTP transfer. It is called first, then
               ftp_client_tick is called until it returns non-zero.
               Failure can occur if the host address is zero, if the
               length is negative, or if the internal control socket
               to the FTP server cannot be opened (e.g. because of
               lack of socket buffers).

               When FTP_CLIENT_KEEPALIVE_WAITTIME is defined (as an
               integer value strictly greater than zero) then TCP
               keepalives at the specified seconds interval are enabled
               on the FTP client's control and data sockets.

PARAMETER1:    Host IP address of FTP server
PARAMETER2:    Port of FTP server, 0 for default
PARAMETER3:    Username of account on FTP server
PARAMETER4:    Password of account on FTP server
PARAMETER5:    Mode of transfer: FTP_MODE_UPLOAD or FTP_MODE_DOWNLOAD.
               You may also OR in the value FTP_MODE_PASSIVE to use
               passive mode transfer (important if you are behind a
               firewall).

               Use FTP_MODE_GETLIST if you just want to retrieve the file
               information given by "LIST filename." If FTP_MODE_GETLIST is
               used with a NULL filename the, the results of "LIST" are
               given. These results may just be a list of file names, or
               they may contain more information with each file - this is
               server-dependent.
PARAMETER6:    Filename to get/put
PARAMETER7:    Directory file is in, NULL for default directory
PARAMETER8:    Buffer to get/put the file to/from.  Must be NULL if
               a data handler function will be used.  See ftp_data_handler
               for more details.
PARAMETER9:    On upload, length of file; on download size of buffer.
               This parameter limits the transfer size to a maximum of
               0x7FFFFFFF bytes.  For larger transfers, set length to 0 and
               use a data handler function.

RETURN VALUE:   0 on success, 1 on failure

SEE ALSO:      ftp_client_setup_url, ftp_client_tick, ftp_data_handler

END DESCRIPTION **********************************************************/

int ftp_client_setup(uint32_t host, int port, const char *username,
    const char *password, int mode, const char *filename,
    const char  *dir, char *buffer, int length)
{
    struct sockaddr_in host_addr;
    int response;

    memset(&ftp, 0, sizeof(ftp)); /* Start with blank slate */
    got_data_file = 0;

    if(0 == port)     /* default the port to the normal FTP port */
    {
        port = 21;
    }

    if (!host)
    {
#ifdef FTP_VERBOSE
        printf("FTPC: no host address.\n");
#endif
        return 1;
    }

    if (length < 0)
    {
#ifdef FTP_VERBOSE
        printf("FTPC: invalid length %d.\n", length);
#endif
        return 1;
    }

    /* init the readln interface */
    ftpc_readln_init();

    /* Init all data */
    strncpy(ftp.username, username, sizeof(ftp.username));
    ftp.nt1 = ftp.nt2 = ftp.nt3 = ftp.nt4 = 0;  // Null terminators
    strncpy(ftp.password, password, sizeof(ftp.password));
    ftp.mode = mode;
    strncpy(ftp.filename, filename, sizeof(ftp.filename));
    if(dir == NULL)
        ftp.dir[0] = '\0';
    else
        strncpy(ftp.dir,dir,sizeof(ftp.dir));

    ftp.buffer = buffer;
    ftp.length = length;
    ftp.offset = 0;

    FTP_SETSTATE(FTP_CONNECTWAIT);

    /* Start connection to remote host */

    ftp.s = socket(AF_INET, SOCK_STREAM, 0);
    if(ftp.mode & FTP_MODE_PASSIVE) /* Passive mode only uses data socket */
    {
        ftp.dtp_s  = socket(AF_INET, SOCK_STREAM, 0);
        ftp.lstn_s = 0;
    }
    else /* Active mode needs listening socket and data socket which is initialised later on... */
    {
        ftp.lstn_s = socket(AF_INET, SOCK_STREAM, 0);
        fcntl(ftp.lstn_s, F_SETFL, O_NONBLOCK); /* Must be non blocking or the accept() will block */
        ftp.dtp_s  = 0;
    }

    if((ftp.s < 0) || (ftp.dtp_s < 0) || (ftp.lstn_s < 0))
    {
#ifdef FTP_VERBOSE
        printf("FTPC: Cannot create sockets\n");
        //sock_perror(&ftp.sock, NULL);
#endif
        if(ftp.s > 0)
        {
            close(ftp.s);
            ftp.s = 0;
        }

        if(ftp.dtp_s > 0)
        {
            close(ftp.dtp_s);
            ftp.dtp_s = 0;
        }

        if(ftp.lstn_s > 0)
        {
            close(ftp.lstn_s);
            ftp.lstn_s = 0;
        }

        return(1);
    }

//    ftp_init_done = 1;
    memset(&host_addr, 0, sizeof(host_addr));

    host_addr.sin_family      = AF_INET;
    host_addr.sin_len         = sizeof(struct sockaddr_in);
    host_addr.sin_port        = htons((uint16_t)port);
    host_addr.sin_addr.s_addr = htonl(host);

    response = connect(ftp.s, (struct sockaddr *)&host_addr, sizeof(host_addr));
    if(response < 0) {
        #ifdef FTP_VERBOSE
        printf("FTPC: cannot talk to %08lX\n", host);
//        sock_perror(&ftp.sock, NULL);
#endif
        if(ftp.s > 0)
        {
            close(ftp.s);
            ftp.s = 0;
        }

        if(ftp.dtp_s > 0)
        {
            close(ftp.dtp_s);
            ftp.dtp_s = 0;
        }

        if(ftp.lstn_s > 0)
        {
            close(ftp.lstn_s);
            ftp.lstn_s = 0;
        }
        return 1;
    }
//   sock_set_tos(&ftp.sock, IPTOS_FAST);
#ifdef FTP_CLIENT_KEEPALIVE_WAITTIME
//   tcp_keepalive(&ftp.sock, FTP_CLIENT_KEEPALIVE_WAITTIME);
#endif
   ftp.code = 0;
    return 0;
}

/* START FUNCTION DESCRIPTION ********************************************
ftp_data_handler                   <FTP_CLIENT.LIB>

SYNTAX: void ftp_data_handler(int (*dhnd)(), void * dhnd_data, word opts)

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Sets a data handler for further FTP data transfer(s).
               This handler is only used if the "buffer" parameter to
               ftp_client_setup() is passed as NULL.

               The handler is a function which must be coded according
               the the following prototype:

                 int my_handler(char * data, int len, longword offset,
                                int flags, void * dhnd_data);

               This function is called with "data" pointing to a data
               buffer, and "len" containing the length of that buffer.
               "offset" is the byte number relative to the first byte
               of the entire FTP stream.  This is useful for data
               handler functions which do not wish to keep track of the
               current state of the data source.
               "dhnd_data" is the pointer that was passed to
               ftp_data_handler().

               "flags" contains an indicator of the current operation:
                 FTPDH_IN : data is to be stored on this host (obtained
                   from an FTP download).
                 FTPDH_OUT : data is to be filled with the next data
                   to upload to the FTP server.
                 FTPDH_END : data and len are irrelevant: this marks
                   the end of data, and gives the function an
                   opportunity to e.g. close the file.  Called after
                   either in or out processing.
                 FTPDH_ABORT : end of data; error encountered during
                   FTP operation.  Similar to END except the transfer
                   did not complete.  Can use this to e.g. delete a
                   partially written file.

               The return value from this function depends on the in/out
               flag.  For FTPDH_IN, the function should return 'len'
               if the data was processed successfully and download should
               continue; -1 if an error has occurred and the transfer
               should be aborted.  For FTPDH_OUT, the function should
               return the actual number of bytes placed in the data
               buffer, or -1 to abort.  If 0 is returned, then the
               upload is terminated normally.  For FTPDH_END, the
               return code should be 0 for success or -1 for error.  If
               an error is flagged, then this is used as the return code
               for ftp_client_tick().  For FTPDH_ABORT, the return code
               is ignored.

               When FTP_CLIENT_KEEPALIVE_WAITTIME is defined (as an
               integer value strictly greater than zero) then TCP
               keepalives at the specified seconds interval are enabled
               on the FTP client's control and data sockets.

PARAMETER1:    Pointer to data handler function, or NULL to remove
               the current data handler.
PARAMETER2:    A pointer which is passed to the data handler function.
               This may be used to point to any further data required by
               the data handler such as an open file descriptor.
PARAMETER3:    Options word (currently reserved, set to zero).

SEE ALSO:      ftp_client_setup

END DESCRIPTION **********************************************************/

/*==============================================================================
 *
 */
void ftp_data_handler(int (*dhnd)(), void * dhnd_data, uint16_t opts)
{
    (void)opts;
    ftp.dhnd = dhnd;
    ftp.dhnd_data = dhnd_data;
}

/*==============================================================================
 *
 */
void ftpc_dtp_open(void)
{
    struct sockaddr_in our_addr;
    int response;
    socklen_t length;

    if (!(ftp.mode & FTP_MODE_PASSIVE)) // Not passive mode so we pick a port
    {
        memset(&our_addr, 0, sizeof(our_addr));

        our_addr.sin_family      = AF_INET;
        our_addr.sin_len         = sizeof(struct sockaddr_in);

        /*
         * First find out what network address our control socket is on.
         * Then we bind our data connection to that address with a port of 0 to
         * get a new port allocated for this. Usually this results in the data
         * port being control port + 1 but it is not guaranteed...
         */
        length = sizeof(our_addr);
        response = getsockname(ftp.s, (struct sockaddr *)&our_addr, &length);
        if(0 == response)
        {
            ftp.dtpaddr = htonl(our_addr.sin_addr.s_addr);
        }
        else
        {
#ifdef FTP_VERBOSE
            printf("FTPC: cannot determine our IP address\n");
            //sock_perror(&ftp.dtp_sock, NULL);
#endif
            FTP_SETSTATE(FTP_BAIL);
            return;
        }

        our_addr.sin_port = 0; /* We will need a new port... */

        /* Bind port to address and start listening for the server attaching. */
        response = bind(ftp.lstn_s, (struct sockaddr *)&our_addr, length);
        response = listen(ftp.lstn_s, 1);
        if(response < 0)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: cannot listen on data transfer port\n");
            //sock_perror(&ftp.dtp_sock, NULL);
#endif
            FTP_SETSTATE(FTP_BAIL);
            return;
        }
        else
        {
            length = sizeof(our_addr);
            response = getsockname(ftp.lstn_s, (struct sockaddr *)&our_addr, &length);
            if(0 == response)
            {
                ftp.dtpport = htons(our_addr.sin_port);
            }
            else
            {
#ifdef FTP_VERBOSE
                printf("FTPC: cannot determine port number\n");
                //sock_perror(&ftp.dtp_sock, NULL);
#endif
                FTP_SETSTATE(FTP_BAIL);
                return;
            }
        }
#ifdef FTP_CLIENT_KEEPALIVE_WAITTIME
//        tcp_keepalive(&ftp.sock, FTP_CLIENT_KEEPALIVE_WAITTIME);
#endif
    }

    if ((ftp.mode & ~FTP_MODE_PASSIVE) == FTP_MODE_DOWNLOAD)
    {
        FTP_SETSTATE(FTP_GETSIZE);
    }
    else
    {
        FTP_SETSTATE(FTP_SETPORT);
    }
}


/* START FUNCTION DESCRIPTION ********************************************
ftp_client_tick                   <FTP_CLIENT.LIB>

SYNTAX: int ftp_client_tick(void);

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Tick function to run the ftp. Must be called periodically.
               The return codes are not very specific.  You can call
               ftp_last_code() to get the integer
               value of the last FTP message received from the server.
               See RFC959 for details.  For example, code 530 means that
               the client was not logged in to the server.

               When FTP_CLIENT_KEEPALIVE_WAITTIME is defined (as an
               integer value strictly greater than zero) then TCP
               keepalives at the specified seconds interval are enabled
               on the FTP client's control and data sockets.

RETURN VALUE:   FTPC_AGAIN (0): still pending, call again
               FTPC_OK (1): success
               FTPC_ERROR (2): failure (call ftp_last_code() for more
                 details).
               FTPC_NOHOST (3): failure (Couldn't connect to server)
               FTPC_NOBUF (4): failure (no buffer or data handler)
               FTPC_TIMEOUT (5): warning (Timed out on close: data may or
                 may not be OK)
               FTPC_DHERROR (6): error (Data handler error in FTPDH_END
                 operation)
               FTPC_CANCELLED (7): FTP control socket was aborted (reset)
                 by the server.

SEE ALSO:      ftp_client_setup, ftp_client_filesize, ftp_client_xfer,
               ftp_last_code

END DESCRIPTION **********************************************************/
int ftp_client_tick(void)
{
    static int i, nd, rc;
    static uint32_t hisip;
    static uint16_t hisport;
    static char *p;
    struct sockaddr_in our_addr;
    struct sockaddr_in his_addr;
    socklen_t addr_len;
    int response;
    int sock_error;
    socklen_t sock_error_len;
    enum tcp_state socket_state;

    if (getpeername(ftp.s, (struct sockaddr *)&our_addr, &addr_len) && (FTP_STATE != FTP_DONE3) && (FTP_STATE != FTP_DONE2) && (0 == got_data_file))
    {
#ifdef FTP_VERBOSE
        printf("FTPC: control connection reset by peer.\n");
        //sock_perror(ftp.s, NULL);
#endif
        ftp.code = FTPC_CANCELLED;
        goto _ftp_client_bail;
    }

    switch(FTP_STATE)
    {
    case FTP_CONNECTWAIT:
        if (!ftp.buffer && !ftp.dhnd)
        {
        _ftp_client_nobuf:
#ifdef FTP_VERBOSE
            printf("FTPC: buffer and data handler both NULL.\n");
#endif
            ftp.code = FTPC_NOBUF;
            goto _ftp_client_bail;
        }

        if (!getpeername(ftp.s, (struct sockaddr *)&our_addr, &addr_len))
        {
            /* Control connection established so get to work */
            FTP_SETSTATE2(FTP_GETLINE, FTP_USERNAME);
        }
        else
        {
            /* Control connection not established yet so check in case it failed */
            sock_error = 0;
            sock_error_len = sizeof(sock_error);
            getsockopt(ftp.s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
            if((0 != sock_error) && (EINPROGRESS != sock_error))
            {
#ifdef FTP_VERBOSE
                printf("FTPC: cannot connect to server.\n");
                //sock_perror(ftp.s, NULL);
#endif
                ftp.code = FTPC_NOHOST;
                goto _ftp_client_bail;
            }
        }
        break;

    case FTP_GETLINE:
        ftpc_readln_tick();
        if(ftp.readln_is_ready)
        {
            ftpc_readln();
            FTP_NEXTSTATE;
        }
        break;

    case FTP_MSGWAIT:
        ftpc_finish_msg();
        break;

    case FTP_USERNAME:
        // Could get a 120 (service available in the future) but treat as error.
        if (ftp.code != 220)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_USERNAME - Response code %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }

        sprintf(ftp.line, "USER %s\r\n", ftp.username);
        ftpc_msg(ftp.line, FTP_PASSWORD);
        break;

    case FTP_PASSWORD:
        if (ftp.code == 230)
        {
            // That's nice, don't need password
            FTP_SETSTATE(FTP_SETMODE);
            break;
        }

        if (ftp.code != 331)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_PASSWORD - Response code %d.\n", ftp.code);
#endif
            // Go directly to jail.  Do not pass GO.  Do not collect $200...
            goto _ftp_client_bail;
        }

        sprintf(ftp.line, "PASS %s\r\n", ftp.password);
        ftpc_msg(ftp.line, FTP_SETMODE);
        break;

    case FTP_SETMODE:
        if (ftp.code != 230)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_SETMODE - Response code %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }

        ftpc_msg("TYPE I\r\n", FTP_SETDIR);
        break;

    case FTP_SETDIR:
        if (ftp.code != 200)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_SETDIR - Response code %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }

        if (!ftp.dir[0])
        {
            ftpc_dtp_open();
        }
        else
        {
            sprintf(ftp.line, "CWD %s\r\n", ftp.dir);
            ftpc_msg(ftp.line, FTP_SETDIR2);
        }
        break;

    case FTP_SETDIR2:
        if (ftp.code != 250)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_SETDIR2 - Response code %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }

        ftpc_dtp_open();
        break;

    case FTP_GETSIZE:
        sprintf( ftp.line, "SIZE %s\r\n", ftp.filename);
        ftpc_msg( ftp.line, FTP_SETPORT);
        break;

    case FTP_SETPORT:
        ftp.filesize = 0;
        if ((ftp.mode & ~FTP_MODE_PASSIVE) == FTP_MODE_DOWNLOAD)
        {
            if (ftp.code == 213)
            {
                ftp.filesize = strtol( &ftp.line[4], NULL, 10);
            }
        }

        if (ftp.mode & FTP_MODE_PASSIVE)
        {
            ftpc_msg( "PASV\r\n", FTP_PASV);
        }
        else
        {
          sprintf( ftp.line, "PORT %u,%u,%u,%u,%u,%u\r\n",
             (int) (ftp.dtpaddr >> 24) & 0xff, (int) (ftp.dtpaddr >> 16) & 0xff,
             (int) (ftp.dtpaddr >> 8) & 0xff,  (int) ftp.dtpaddr & 0xff,
             ftp.dtpport >> 8, ftp.dtpport & 0xFF);

          ftpc_msg( ftp.line, FTP_CMD);
        }
        break;

    case FTP_PASV:
        if (ftp.code != 227)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_PASV - Response code %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }
        /*
         * Parse out his IP address and port for DTP socket.  Expect 6 groups of
         * decimal digits separated by non-digits (should be commas).
         */
        p = ftp.line + 4;   /* Skip '227 '. */
        while (*p && !isdigit(*p))
        {
            p++;
        }

        hisip = 0;
        hisport = 0;
        for (nd = 0; nd < 6; nd++)
        {
            i = 0;
            while (isdigit(*p) && i < 256)
            {
                i = i*10 + (*p - '0');
                p++;
            }
            while (*p && !isdigit(*p))
            {
                p++;
            }
            if (i > 255)
            {
    #ifdef FTP_VERBOSE
                printf("FTPC: FTP_PASV 2 - too many digits...\n");
    #endif
                goto _ftp_client_bail;
            }
            if (nd < 4)
            {
                hisip = (uint32_t)((hisip << 8) + (uint32_t)i);
            }
            else
            {
                hisport = (uint16_t)((hisport << 8) + i);
            }
        }
#ifdef FTP_VERBOSE
        printf("FTPC: opening to [%08lX]:%u\n", hisip, hisport);
#endif
        memset(&his_addr, 0, sizeof(his_addr));
        his_addr.sin_family      = AF_INET;
        his_addr.sin_len         = sizeof(struct sockaddr_in);
        his_addr.sin_port        = htons(hisport);
        his_addr.sin_addr.s_addr = htonl(hisip);

        response = connect(ftp.dtp_s, (struct sockaddr *)&his_addr, sizeof(his_addr));

        if (response)
        {
#ifdef FTP_VERBOSE
        printf("FTPC: FTP_PASV 3 - cannot open data transfer port.\n");
        //sock_perror(ftp.dtp_s, NULL);
#endif
        goto _ftp_client_bail;
        }
//        sock_set_tos(ftp.dtp_s, IPTOS_CAPACIOUS);
#ifdef FTP_CLIENT_KEEPALIVE_WAITTIME
//        tcp_keepalive(&ftp.sock, FTP_CLIENT_KEEPALIVE_WAITTIME);
#endif
        FTP_SETSTATE(FTP_CMD);
        ftp.code = 200; // Make ignore following test
        // fall through
    case FTP_CMD:
        if (ftp.code != 200)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_PASV 4 - Response code %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }

        switch (ftp.mode & ~FTP_MODE_PASSIVE)
        {
        case FTP_MODE_UPLOAD:
            sprintf(ftp.line, "STOR %s\r\n", ftp.filename);
            ftpc_msg(ftp.line, FTP_UPLOAD);
            break;

        case FTP_MODE_DOWNLOAD:
            sprintf(ftp.line, "RETR %s\r\n", ftp.filename);
            ftpc_msg(ftp.line, FTP_DOWNLOAD);
            break;

        case FTP_MODE_GETLIST:
         sprintf(ftp.line, "LIST %s\r\n", ftp.filename);
            ftpc_msg(ftp.line, FTP_DOWNLOAD);
            break;

        default:
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_PASV - unknown operation\n");
#endif
            goto _ftp_client_bail;
        }
        break;

    case FTP_UPLOAD:
        /*
         * Upload either a fixed block of data from the buffer and length
         * specified at setup or a variable amount supplied by the data handler.
         *
         * 125 - Data connection already open; transfer starting.
         * 150 - File status okay; about to open data connection.
         */
        if (ftp.code != 150 && ftp.code != 125)
        {
            goto _ftp_client_bail;
        }

        sock_error = 0;
        sock_error_len = sizeof(sock_error);
        getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
        if((0 != sock_error) && (EINPROGRESS != sock_error) && (EAGAIN != sock_error))
        {
        _ftp_dtp_was_aborted:
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_UPLOAD - data transfer port reset by peer.\n");
           // sock_perror(ftp.dtp_s, NULL);
#endif
            goto _ftp_client_bail;
        }

        if ((EINPROGRESS == sock_error))
        {
            break;
        }
        else
        {
            if (!ftp.buffer) /* Data Handler operation as there is no buffer... */
            {
                if (!ftp.dhnd) /* Or nothing at all! */
                {
                    goto _ftp_client_nobuf;
                }

                rc = ftp.dhnd(ftp.line, sizeof(ftp.line), ftp.offset, FTPDH_OUT, ftp.dhnd_data); /* Let the data handler at it */

                if (rc < 0) /* Data handler says no... */
                {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP_UPLOAD 2 - bad data handler return %d\n", rc);
#endif
                    goto _ftp_client_bail;
                }

                if (rc > 0) /* This is how much we actually put in the buffer */
                {
                    do /* Write this chunk */
                    {
                        int sent;

                        /*
                         * Do we need timeout here or rely on overall timeout on FTP operation?
                         */
                        sent = send(ftp.dtp_s, ftp.line, (size_t)rc, MSG_DONTWAIT);
                        if (sent < 0)
                        {
                            sock_error = 0;
                            sock_error_len = sizeof(sock_error);
                            getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
                            if((0 != sock_error) && (EAGAIN != sock_error))
                            {
#ifdef FTP_VERBOSE
                                printf("FTPC: FTP_UPLOAD 3 - send() failed\n");
#endif
                                goto _ftp_client_bail;
                            }
                        }
                        else
                        {
                            ftp.offset += sent;
                            rc         -= sent;
                        }
                    } while(rc);
                }
                else /* All done now so carry on */
                {
                    FTP_SETSTATE(FTP_UPLOAD2);
                    close(ftp.dtp_s);
                }

            }
            else
            {
                int sent;
                rc = (int) ftp.length - ftp.offset; /* Try to send all remaining data */

                sent = send(ftp.dtp_s, ftp.buffer + ftp.offset, (size_t)rc, MSG_DONTWAIT);
                if (sent < 0)
                {
                    sent = 0; /* in case we don't need to bail */
                    sock_error = 0;
                    sock_error_len = sizeof(sock_error);
                    getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
                    if(EWOULDBLOCK != sock_error)
                    {
#ifdef FTP_VERBOSE
                        printf("FTPC: FTP_UPLOAD 4 - send() failed - error %d\n", sock_error);
#endif
                        goto _ftp_client_bail;
                    }
                }

#ifdef FTP_VERBOSE
                if(sent)
                {
                    putchar('+');
                    fflush(stdout);
                }
#endif
                rc -= sent;
                ftp.offset += sent;
                if(0 == rc)
                {
                    FTP_SETSTATE(FTP_UPLOAD2);
#ifdef FTP_VERBOSE
                    printf("\n");
#endif

                    close(ftp.dtp_s);
                }
            }
        }
        break;

    case FTP_UPLOAD2:
        socket_state = getsockstate(ftp.dtp_s);
        if(-1 == socket_state) /* Socket no longer there or other low level issue... */
        {
#ifdef FTP_VERBOSE
//            if (sock_error) {
            printf("FTPC: FTP Data Socket disconnected.\n");
            printf("FTPC: Trying for FTP Code 226 to verify successful send.\n");
#endif
            FTP_SETSTATE2(FTP_GETLINE, FTP_UPLOAD3);
        }
        else if(ESTABLISHED != socket_state)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP UPLOAD2 - hopefully in socket wind down...\n");
#endif
            /*
             * The data socket is all done or in the process of shutting down,
             * whether aborted on error or closed
             * normally. Check the control socket's status to determine success
             * vs. failure in the next FTP upload state.
             */
            FTP_SETSTATE2(FTP_GETLINE, FTP_UPLOAD3);
        }
        else
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP UPLOAD2 - waiting for socket wind down...\n");
#endif
        }
        break;

    case FTP_UPLOAD3:
        if (ftp.code != 226)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: Expected FTP Code 226 - got %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;  // the FTP server was not happy
        }
        else
        {
            FTP_SETSTATE(FTP_DONE); // success, the server got the complete file
        }
        break;

    case FTP_DOWNLOAD:
        if (ftp.code != 150 && ftp.code != 125)
        {
#ifdef FTP_VERBOSE
            printf("FTPC: Expected FTP Code 150 or 125 - got %d.\n", ftp.code);
#endif
            goto _ftp_client_bail;
        }

        if(0 == (ftp.mode & FTP_MODE_PASSIVE))
        {
            addr_len = sizeof(our_addr);
            response = accept(ftp.lstn_s, (struct sockaddr *)&our_addr, &addr_len);
            if(response < 0)
            {
                sock_error = 0;
                sock_error_len = sizeof(sock_error);
                getsockopt(ftp.lstn_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
                if((0 != sock_error) && (EINPROGRESS != sock_error) && (EAGAIN != sock_error))
                {
                    goto _ftp_dtp_was_aborted;
                }
                else
                {
                    break; /* Let's try again */
                }
            }
            else
            {
                ftp.dtp_s = response; /* We have a receive socket established */
            }
        }
        else
        {
            sock_error = 0;
            sock_error_len = sizeof(sock_error);
            getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
            if((0 != sock_error) && (EINPROGRESS != sock_error) && (EAGAIN != sock_error))
            {
                goto _ftp_dtp_was_aborted;
            }
            /*
             * PMCS add getsockstate() and check for ESTABLISHED?
             */
            if((EINPROGRESS == sock_error) || (EAGAIN == sock_error))
            {
                break;
            }
        }

        FTP_SETSTATE(FTP_DOWNLOADING);
        // Fall thru
    case FTP_DOWNLOADING:
        sock_error = 0;
        sock_error_len = sizeof(sock_error);
        getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
        if((0 != sock_error) && (EINPROGRESS != sock_error) && (EAGAIN != sock_error))
        {
            goto _ftp_client_rx_finished;
        }
        socket_state = getsockstate(ftp.dtp_s);
        if(-1 == socket_state) /* Socket no longer there or other low level issue... */
        {
#ifdef FTP_VERBOSE
            printf("FTPC: FTP Data Socket disconnected.\n");
#endif
            goto _ftp_client_rx_finished;
        }
        /*
         * PMCS add getsockstate() and check for ESTABLISHED?
         */

        if (!ftp.buffer) /* Data Handler mode */
        {
            if (!ftp.dhnd) /* Or perhaps not... */
            {
                goto _ftp_client_nobuf;
            }
            rc = recv(ftp.dtp_s, ftp.line, sizeof(ftp.line), MSG_DONTWAIT); /* Try and fetch a buffer full */
            if (rc < 0)
            {
                sock_error = 0;
                sock_error_len = sizeof(sock_error);
                getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
                if((0 != sock_error) && (EAGAIN != sock_error) && (ENOTCONN != sock_error))
                {
                    goto _ftp_client_bail;
                }
            }
            else if (rc)
            {
                rc = ftp.dhnd(ftp.line, rc, ftp.offset, FTPDH_IN, ftp.dhnd_data);
                if (rc < 0)
                {
                    goto _ftp_client_bail;
                }
                ftp.offset += rc;
            }
        }
        else /* Single buffer mode */
        {

            rc = INT_MAX < (ftp.length - ftp.offset) ?  INT_MAX : ftp.length - ftp.offset;
            if (rc) /* Buffer not full */
            {
                rc = recv(ftp.dtp_s, ftp.buffer + ftp.offset, (size_t) rc, MSG_DONTWAIT);
                if (rc < 0)
                {
                    sock_error = 0;
                    sock_error_len = sizeof(sock_error);
                    getsockopt(ftp.dtp_s, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_len);
                    if((0 != sock_error) && (EAGAIN != sock_error) && (ENOTCONN != sock_error))
                    {
#ifdef FTP_VERBOSE
                        printf("FTPC: Error reading data - bailing out now!\n");
#endif
                        goto _ftp_client_bail;
                    }
                }
                else
                {
#ifdef FTP_VERBOSE
                    putchar('+');
                    fflush(stdout);
#endif
                    ftp.offset += rc;
                    if(ftp.offset == ftp.filesize)
                    {
                        goto _ftp_client_rx_finished;
                    }
                }
            }
            else /* Oops, buffer full, discard any remaining data */
            {
                char data_dump[256];
                // Discard data past end of buffer
                do
                {
                    rc = recv(ftp.dtp_s, data_dump,(size_t) 256, MSG_DONTWAIT);
#ifdef FTP_VERBOSE
                    putchar('-');
                    fflush(stdout);
#endif
                } while(rc > 0);

#ifdef FTP_VERBOSE
                printf("FTPC: Too much data - bailing out now!\n");
#endif
                ftp.code = FTPC_ERROR;
                goto _ftp_client_bail;
            }
        }
        break;

    _ftp_client_rx_finished:
        got_data_file = 1;
#ifdef FTP_VERBOSE
        printf("\r\nFTPC: Received %lu byte file.\n", ftp.offset);
#endif
        FTP_SETSTATE2(FTP_GETLINE, FTP_DONE);
        break;

    case FTP_DONE:
        if (!ftp.buffer && ftp.dhnd)
        {
            ftp.dhnd_rc = ftp.dhnd(ftp.line, 0, ftp.offset, FTPDH_END, ftp.dhnd_data);
        }
        else
        {
            ftp.dhnd_rc = 0;
        }
        ftpc_msg("QUIT\r\n", FTP_DONE2);
/*
 * "REIN" works better when downloading two files on a non-Rabbit server
 * because the QUIT causes the server to close the connection
 */
        break;

    case FTP_DONE2:
#ifdef FTP_VERBOSE
        printf("FTPC: Closing...\n");
#endif
        ftp.timeout = xTaskGetTickCount();

#ifdef FTP_VERBOSE
        printf("FTPC: Closing main socket.\n");
#endif
        close(ftp.s);
#ifdef FTP_VERBOSE
        printf("FTPC: Closing dtp socket.\n");
#endif
        close(ftp.dtp_s);
        if(0 == (ftp.mode & FTP_MODE_PASSIVE))
        {
            close(ftp.lstn_s);
        }
        FTP_SETSTATE(FTP_DONE3);
        // fall thru

    case FTP_DONE3:
        // Read any remaining data in socket(s)
        //if (sock_readable(ftp.s))
        //    sock_fastread(ftp.s, NULL, INT_MAX);
        //if (sock_readable(ftp.dtp_s))
        //    sock_fastread(ftp.dtp_s, NULL, INT_MAX);
        //if (chk_timeout(ftp.timeout)) {
        if((xTaskGetTickCount() - ftp.timeout) > ((FTP_TIMEOUT * 1000L) / portTICK_RATE_MS))
        {
#ifdef FTP_VERBOSE
            printf("FTPC: timeout on close\n");
#endif
          //  sock_abort(ftp.s);
          //  sock_abort(ftp.dtp_s);
            close(ftp.s);
            close(ftp.dtp_s);
            if(0 == (ftp.mode & FTP_MODE_PASSIVE))
            {
                close(ftp.lstn_s);
            }
            return FTPC_TIMEOUT;
        }
        //if( !sock_alive(ftp.s) && !sock_alive(ftp.dtp_s) ) {
#ifdef FTP_VERBOSE
            printf("FTPC: done\n");
#endif
       //     if (ftp.dhnd_rc < 0)
       //         return FTPC_DHERROR;
           return FTPC_OK;
       // }
        break;

    _ftp_client_bail:
        FTP_SETSTATE(FTP_BAIL);
        // fall thru
    case FTP_BAIL:
#ifdef FTP_VERBOSE
        printf("FTPC: bailing out\n");
#endif
        if (!ftp.buffer && ftp.dhnd)
        {
            ftp.dhnd(ftp.line, 0, ftp.offset, FTPDH_ABORT, ftp.dhnd_data);
        }
//        sock_abort(ftp.s);
//        sock_abort(ftp.dtp_s);
        close(ftp.s);
        close(ftp.dtp_s);
        if(0 == (ftp.mode & FTP_MODE_PASSIVE))
        {
            close(ftp.lstn_s);
        }

        if (ftp.code > 0 && ftp.code < 100)
        {
            return ftp.code;
        }
        return FTPC_ERROR;
    }
    return FTPC_AGAIN;
}

/* START FUNCTION DESCRIPTION ********************************************
ftp_client_filesize                   <FTP_CLIENT.LIB>

SYNTAX: int ftp_client_filesize(void);

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Returns the byte count of data transferred.  This function
               is deprecated in favor of ftp_client_xfer(), which
               returns a long value.

               If the number of bytes transferred was over 0x7FFFFFFF, then
               this function returns 0x7FFFFFFFF which may be misleading.

RETURN VALUE:   size, in bytes

SEE ALSO:      ftp_client_setup, ftp_data_handler, ftp_client_xfer

END DESCRIPTION **********************************************************/
int ftp_client_filesize(void)
{
    if (ftp.offset > 0x7FFFFFFF)
    {
        return 0x7FFFFFFF;
    }
    else
    {
        return (int)ftp.offset;
    }
}


/* START FUNCTION DESCRIPTION ********************************************
ftp_client_xfer                   <FTP_CLIENT.LIB>

SYNTAX: longword ftp_client_xfer(void);

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Returns the byte count of data transferred.  Transfers of
               over 2**32 bytes (about 4GB) are not reported correctly.

RETURN VALUE:   size, in bytes

SEE ALSO:      ftp_client_setup, ftp_data_handler, ftp_client_filesize

END DESCRIPTION **********************************************************/
uint32_t ftp_client_xfer(void)
{
    return (uint32_t)ftp.offset;
}


/* START FUNCTION DESCRIPTION ********************************************
ftp_last_code                   <FTP_CLIENT.LIB>

SYNTAX: int ftp_last_code(void)

KEYWORDS:       tcpip, ftp

DESCRIPTION:    Returns the most recent message code sent by the FTP
               server.  RFC959 describes the codes in detail.  This
               function is most useful for error diagnosis in the case
               that an FTP transfer failed.

RETURN VALUE:   error code; a number between 0 and 999.  Codes less than
               100 indicate that an internal error occurred e.g.
               the server was never contacted.

SEE ALSO:      ftp_client_setup, ftp_client_tick

END DESCRIPTION **********************************************************/
int ftp_last_code(void)
{
    return ftp.code;
}


