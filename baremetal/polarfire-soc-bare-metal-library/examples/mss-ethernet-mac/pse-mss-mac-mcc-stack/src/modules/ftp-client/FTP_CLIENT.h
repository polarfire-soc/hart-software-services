/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#ifdef FTP_DEBUG
    #define _ftpc_nodebug __debug
#else
    #define _ftpc_nodebug
#endif

#ifndef FTP_MAX_NAMELEN
    #define FTP_MAX_NAMELEN 80
#endif

#ifndef FTP_MAX_FNLEN
    #define FTP_MAX_FNLEN   80
#endif

#ifndef FTP_MAX_DIRLEN
    #define FTP_MAX_DIRLEN  64
#endif

#ifndef FTP_MAXLINE
    #define FTP_MAXLINE 4096
#endif

#ifndef FTP_TIMEOUT
    #define FTP_TIMEOUT 16
#endif

// Optionally define FTP_CLIENT_KEEPALIVE_WAITTIME to enable TCP keepalives, at
//  the specified seconds interval, on FTP client control and data sockets.
#ifdef FTP_CLIENT_KEEPALIVE_WAITTIME
 #if FTP_CLIENT_KEEPALIVE_WAITTIME+1 <= 1
    #error "If defined, FTP_CLIENT_KEEPALIVE_WAITTIME (in seconds) is expected" \
                                      " to be a constant integer greater than 0."
    #fatal "If an FTP_CLIENT_KEEPALIVE_WAITTIME variable definition is desired" \
                                 " then this conditional block must be disabled."
 #endif
#endif

#define FTP_MODE_UPLOAD     0x0000
#define FTP_MODE_DOWNLOAD   0x0001
#define FTP_MODE_PASSIVE    0x0100
#define FTP_MODE_GETLIST    0x0200

#define FTP_CMD_RX_BUF_LEN 128
#define FTP_CMD_TX_BUF_LEN 128
#define FTP_DAT_RX_BUF_LEN 2048
#define FTP_DAT_TX_BUF_LEN 2048

typedef struct {
    /* Network stuff */
/*
    tcp_Socket      sock;
    tcp_Socket *    s;
    tcp_Socket      dtp_sock;
    tcp_Socket *    dtp_s;
*/
    //tcp_Socket      sock;
    tcpTCB_t        s;
    //tcp_Socket      dtp_sock;
    tcpTCB_t        lstn_s; /* Socket for listening in active mode */
    tcpTCB_t        dtp_s;  /* Socket for data transfer in active and passive mode */
    uint16_t        dtpport;
    uint32_t        dtpaddr;

    uint8_t         s_rx_buf1[FTP_CMD_RX_BUF_LEN + 1]; /* Allow + 1 for push back character */
    uint8_t         s_rx_buf2[FTP_CMD_RX_BUF_LEN + 1];
    bool            s_use_buf1;
    uint16_t        s_rx_len;
    uint8_t         *s_in_ptr;
    uint16_t        s_pushback; /* if == 0xFFFF, no push backed character */
    uint8_t         s_tx_buf[FTP_CMD_TX_BUF_LEN];
    uint8_t         d_rx_buf[FTP_DAT_RX_BUF_LEN];
    uint8_t         d_tx_buf[FTP_DAT_TX_BUF_LEN];
    uint8_t         l_rx_buf[FTP_DAT_RX_BUF_LEN];
    uint8_t         l_tx_buf[FTP_DAT_TX_BUF_LEN];

    uint8_t         in_line[FTP_CMD_RX_BUF_LEN + 1]; /* Add nul as a guard when loading */

    /* General info */
    char                username[FTP_MAX_NAMELEN];
    char                nt1;
    char                password[FTP_MAX_NAMELEN];
    char                nt2;
    int                 mode;
    char                filename[FTP_MAX_FNLEN];
    char                nt3;
    char                dir[FTP_MAX_DIRLEN];
    char                nt4;
    char *          buffer;
    int32_t         length;         // size of buffer
    int32_t         offset;         // offset into file stream
    int32_t         filesize;       // length of file being downloaded

    /* State machine vars */
    uint16_t            state;  // Queue of up to 2 states
#define FTP_SETSTATE(s) (ftp.state = (s))
#define FTP_SETSTATE2(s1, s2) (ftp.state = (uint16_t)((s1) | (s2)<<8))
#define FTP_NEXTSTATE (ftp.state >>= 8)
#define FTP_STATE ((int)(ftp.state & 0xFF))
    //int               nextstate;
    //int               next2state;
    uint32_t         timeout;

    /* readln stuff */
    char                line[FTP_MAXLINE];
    char *          pline;
    int                 readln_is_ready;
    int             code1;      // 1st digit of server messages (0-9) - stored in binary, not char
    int             code;           // All digits (0-999)

    /* msg send stuff */
    int                 msglen;
    int             msgcomplete;
    char *          message;

    // Data handler stuff
    int                 (*dhnd)();
    void *          dhnd_data;
    int             dhnd_rc;        // Final return code

} FtpInfo;

/* States */
enum {
    FTP_CONNECTWAIT,
    FTP_GETLINE,
    FTP_MSGWAIT,
    FTP_BAIL,
    FTP_USERNAME,
    FTP_PASSWORD,
    FTP_SETMODE,
    FTP_SETDIR,
    FTP_SETDIR2,
    FTP_GETSIZE,
    FTP_SETPORT,
    FTP_CMD,
    FTP_UPLOAD,
    FTP_UPLOAD2,
    FTP_UPLOAD3,
    FTP_DOWNLOAD,
    FTP_DOWNLOADING,
    FTP_DONE,
    FTP_DONE2,
    FTP_DONE3,
    FTP_PASV
};


/*****************************************************************
 * The readln interface:                                         *
 *                                                               *
 * ftpc_readln_init() must be called first, to start the process.*
 * ftpc_readln_tick() should be called to drive readln.          *
 * ftpc_readln()    will return the line itself (char *)           *
 *****************************************************************/

void ftpc_readln_init(void);
void ftpc_readln_tick(void);
char *ftpc_readln(void);
/* End of readln interface */
/******************************************************/

/*** Message send stuff. ***/
/*
 * Send a message over the command socket to the FTP client
 */
void ftpc_msg(char *str, int nextstate);
void ftpc_finish_msg(void);
void ftpc_finish_msg(void);
int ftp_client_setup_url( const char *url, int mode, char *buffer, int length);
int ftp_client_setup(
    uint32_t host,                      /* remote host */
    int port,                       /* main port on remote host, 0 == default */
    const char *username, /* login info */
    const char *password,
    int mode,                       /* mode of FTP (see #defines above) */
    const char *filename, /* file to get */
    const char *dir,          /* directory (NULL == default dir) */
    char *buffer,                   /* pointer to file, or area to store dl'd file */
    int length                      /* length of file, of size of storage area */
    );

#define FTPDH_IN        1
#define FTPDH_OUT   2
#define FTPDH_END   3
#define FTPDH_ABORT 4

void ftp_data_handler(int (*dhnd)(), void * dhnd_data, uint16_t opts);
void ftpc_dtp_open(void);
int ftp_client_tick(void);

#define FTPC_AGAIN          0
#define FTPC_OK             1
#define FTPC_ERROR          2
#define FTPC_NOHOST         3
#define FTPC_NOBUF          4
#define FTPC_TIMEOUT        5
#define FTPC_DHERROR        6
#define FTPC_CANCELLED      7

int ftp_client_filesize(void);
uint32_t ftp_client_xfer(void);
int ftp_last_code(void);

#endif
