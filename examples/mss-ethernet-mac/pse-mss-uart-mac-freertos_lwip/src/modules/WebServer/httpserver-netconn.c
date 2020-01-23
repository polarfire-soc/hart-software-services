/*******************************************************************************
 * (c) Copyright 2013 Microsemi SoC Producst Group.  All rights reserved.
 *
 *
 * SVN $Revision: 4971 $
 * SVN $Date: 2013-01-11 22:14:31 +0000 (Fri, 11 Jan 2013) $
 */

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "httpserver-netconn.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"

#include "mpfs_hal/mss_coreplex.h"

#include <assert.h>

#include "mpfs_hal/mss_plic.h"
#include "config/hardware/hw_platform.h"

#include "drivers/mss_mac/mss_ethernet_registers.h"
#include "drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

#include "if_utils.h"

#if LWIP_NETCONN

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif

#define RTC_PRESCALER    (50000000u - 1u)        /* 50MhZ clock is RTC clock source. */

/*------------------------------------------------------------------------------
 * External functions.
 */

/*------------------------------------------------------------------------------
 *
 */
void handle_trigger_request(char * buf, u16_t len);

/*------------------------------------------------------------------------------
 *
 */
extern const char mscc_jpg_logo[8871];

/*------------------------------------------------------------------------------
 *
 */
static const char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
static const char http_json_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: application/jsonrequest\r\n\r\n";

static const char http_post_resp_hdr[] = "HTTP/1.1 204 No Content\r\n\r\n";
static const char http_html_ok_hdr[] = "HTTP/1.1 200 OK\r\n\r\n";

/*------------------------------------------------------------------------------
 *
 */
#if defined(USB_DEVICE_RNDIS)
#if defined(SF2_DEV_KIT)
#define DEMO_PLATFORM "SF2 Dev Kit<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined(SF2_DEV_KIT_1588)
#define DEMO_PLATFORM "SF2 Dev Kit 1588<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined(SF2_EVAL_KIT)
#define DEMO_PLATFORM "SF2 Eval Kit<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined(SF2_SEC_EVAL_KIT)
#define DEMO_PLATFORM "SF2 Security Eval Kit<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined(SF2_ADV_DEV_KIT)
#define DEMO_PLATFORM "SF2 Advanced Dev Kit<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined(_ZL303XX_FMC_BOARD)
#define DEMO_PLATFORM "SF2 Advanced Dev Kit + 1588 FMC Board<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined(_ZL303XX_NTM_BOARD)
#define DEMO_PLATFORM "Calix 1588 FMC Board<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined (SF2_STARTER_KIT_010)
#define DEMO_PLATFORM "SF2 Starter Kit (M2S010)<br />FreeRTOS + lwIP + RNDIS"
#endif
#if defined (SF2_STARTER_KIT_010_166)
#define DEMO_PLATFORM "SF2 Starter Kit (M2S010 @ 166MHz)<br />FreeRTOS + lwIP + RNDIS"
#endif
#else
#if defined(SF2_DEV_KIT)
#define DEMO_PLATFORM "SF2 Dev Kit<br />FreeRTOS + lwIP"
#endif
#if defined(SF2_DEV_KIT_1588)
#define DEMO_PLATFORM "SF2 Dev Kit 1588<br />FreeRTOS + lwIP"
#endif
#if defined(SF2_EVAL_KIT)
#define DEMO_PLATFORM "SF2 Eval Kit<br />FreeRTOS + lwIP"
#endif
#if defined(SF2_SEC_EVAL_KIT)
#define DEMO_PLATFORM "SF2 Security Eval Kit<br />FreeRTOS + lwIP"
#endif
#if defined(SF2_ADV_DEV_KIT)
#define DEMO_PLATFORM "SF2 Advanced Dev Kit<br />FreeRTOS + lwIP"
#endif
#if defined(_ZL303XX_FMC_BOARD)
#if defined(_ZL303XX_MIV)
#define DEMO_PLATFORM "SF2 Advanced Dev Kit + 1588 FMC Board<br />Risc-V + FreeRTOS + lwIP"
#else
#define DEMO_PLATFORM "SF2 Advanced Dev Kit + 1588 FMC Board<br />FreeRTOS + lwIP"
#endif
#endif
#if defined(_ZL303XX_NTM_BOARD)
#define DEMO_PLATFORM "Calix 1588 Board<br />FreeRTOS + lwIP"
#endif
#if defined (SF2_STARTER_KIT_010)
#define DEMO_PLATFORM "SF2 Starter Kit (M2S010)<br />FreeRTOS + lwIP"
#endif
#if defined (SF2_STARTER_KIT_010_166)
#define DEMO_PLATFORM "SF2 Starter Kit (M2S010 @ 166MHz)<br />FreeRTOS + lwIP"
#endif
#endif
#if defined(TARGET_ALOE)
#define DEMO_PLATFORM "SiFive Aloe Board<br />FreeRTOS + lwIP"
#endif

#if defined(TARGET_G5_SOC)
#define DEMO_PLATFORM "G5 SOC Emulation Peripheral Board<br />FreeRTOS + lwIP"
#endif


static const char http_index_html[] = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\
<html dir=\"ltr\"><head>\
    <meta content=\"text/html; charset=UTF-8\" http-equiv=\"content-type\">\
    <title>Microsemi SmartFusion2 Demo</title>\
\
    <meta content=\"cyriljean\" name=\"author\"><meta content=\"BlueGriffon\
      wysiwyg editor\" name=\"generator\">\
    <style type=\"text/css\">\
        .bodyText {\
            font-family: Arial, Helvetica, sans-serif;\
            font-size: 12px;\
            color: #333333;\
        }\
        .headline2 {\
            font-family: Arial, Helvetica, sans-serif;\
            font-size: 12px;\
            font-weight: bold;\
            color: #333333;\
        }\
        .headline1 {\
            font-family: \"Trebuchet MS\";\
            font-size: 18px;\
            font-weight: bold;\
            color: #666666;\
        }\
        .smallText {\
            font-family: Verdana, Tahoma, Arial, Helvetica, sans-serif;\
            font-size: 9px;\
            color: #666666;\
        }\
        .top_headline {\
            font-family: Arial, Helvetica, sans-serif;\
            font-size: 24px;\
            color: #666666;\
            padding-top: 5px;\
        }\
        </style>\
  </head>\
<body>\
<table border=\"0\" width=\"800\" align=\"center\">\r\
  <tbody>\r\
    <tr>\r\
        <td width=\"50%\"><img name=\"index_r1_c1\" src=\"./index_r1_c1.jpg\" width=\"300\" height=\"89\" border=\"0\" alt=\"\"></td>\r\
        <td width=\"50%\"><p class=\"top_headline\">Bootloader Sample Application<br>" DEMO_PLATFORM "</p></td>\r\
    </tr>\r\
  </tbody>\r\
</table>\r\
<hr width=\"800\" />\r\
<table border=\"0\" width=\"800\" align=\"center\">\r\
      <tbody>\r\
        <tr>\r\
          <td width=\"50%\" colspan=\"1\" rowspan=\"2\"><div class=\"headline1\" style=\"text-align: center;\"><span style=\"font-weight: bold;\">Network Interface</span><br></div>\r\
            <table border=\"0\" width=\"60%\" align=\"center\" cellspacing=\"10\">\r\
              <tbody class=\"bodyText\">\r\
                <tr>\r\
                  <td class=\"headline2\" width=\"50%\" style=\"text-align: right;\">MAC Address:<br></td>\r\
                  <td id=\"MAC_Addr\" width=\"50%\" style=\"text-align: center;\">-<br></td>\r\
                </tr>\r\
                <tr>\r\
                  <td class=\"headline2\" style=\"text-align: right;\">TCP/IP Address:<br></td>\r\
                  <td id=\"TCPIP_Addr\" style=\"text-align: center;\">-<br></td>\r\
                </tr>\r\
                <tr>\r\
                  <td class=\"headline2\" style=\"text-align: right;\">Speed:<br></td>\r\
                  <td id=\"LinkSpeed\" style=\"text-align: center;\">-<br></td>\r\
                </tr>\r\
                <tr>\r\
                  <td><br></td>\r\
                  <td id=\"DebugStatus\"><br></td>\r\
                </tr>\r\
              </tbody>\r\
            </table>\r\
          </td>\r\
          <td>\r\
            <div class=\"headline1\" style=\"text-align: center;\"><span style=\"font-weight: bold;\">Real Time Counter</span><br>\r\
            </div>\r\
            <table border=\"0\" width=\"30%\" align=\"center\" cellspacing=\"10\">\r\
              <tbody class=\"bodyText\">\r\
                <tr>\r\
                  <td width=\"40%\"><div class=\"headline2\" style=\"text-align: center;\">Time:<br></div></td>\r\
                  <td width=\"20%\"><div id=\"CurrentRTCTime\" style=\"text-align: center;\">-<br></div></td>\r\
                </tr>\r\
                <tr>\r\
                  <td width=\"40%\"><div class=\"headline2\" style=\"text-align: center;\">Date:<br></div></td>\r\
                  <td width=\"20%\"><div id=\"Current_RTC_Date\" style=\"text-align: center;\">-<br></div></td>\r\
                </tr>\r\
              </tbody>\r\
            </table>\r\
          </td>\r\
        </tr>\r\
        <tr>\r\
          <td>\r\
            <hr />\r\
            <div class=\"headline1\" width=\"50%\" style=\"text-align: center; font-weight: bold;\">Change RTC value<br></div>\r\
            <table border=\"0\" width=\"65%\" align=\"center\" cellspacing=\"10\">\r\
              <tbody class=\"bodyText\">\r\
                <tr>\r\
                  <td>\r\
                    <table border=\"0\" width=\"100%\" align=\"center\">\r\
                      <tbody class=\"bodyText\">\r\
                        <tr>\r\
                            <td  class=\"headline2\">Time (hh:mm:ss):</td>\r\
                            <td><input id=\"SetTime\" type=\"text\" name=\"time\" maxlength=\"8\" size=\"8\" value=\"00:00:00\"  style=\"text-align: center;\" /></td>\r\
                            <td><button onclick=\"post_new_time_request();\">Set</button></td>\r\
                        </tr>\r\
                      </tbody>\r\
                    </table>\r\
                    </td>\r\
                </tr>\r\
                <tr>\r\
                  <td>\r\
                    <table border=\"0\" width=\"100%\" align=\"center\">\r\
                      <tbody class=\"bodyText\">\r\
                        <tr>\r\
                            <td  class=\"headline2\">Date (dd/mm/yy):</td>\r\
                            <td><input id=\"SetDate\" type=\"text\" name=\"time\" maxlength=\"8\" size=\"8\" value=\"00:00:00\"  style=\"text-align: center;\" /></td>\r\
                            <td><button onclick=\"post_new_date_request();\">Set</button></td>\r\
                        </tr>\r\
                      </tbody>\r\
                    </table>\r\
                    </td>\r\
                </tr>\r\
                <tr>\r\
                  <td>\r\
                    </td>\r\
                </tr>\r\
              </tbody>\r\
            </table>\r\
          </td>\r\
        </tr>\r\
      </tbody>\r\
    \r\
</table>\r\
<hr width=\"800\" />\r\
<div class=\"smallText\" width=\"50%\" style=\"text-align: center; font-weight: bold;\">Microsemi SoC Products Group - v0.1<br></div>\r\
<p><br>\
    </p>\r\
<script type=\"text/javascript\">\r\
function update_page() {\r\
    var request = new XMLHttpRequest();\r\
    request.open(\"GET\",\"status\");\r\
    request.onreadystatechange = function() {\r\
        if(request.readyState === 4 && request.status === 200) {\r\
            var parsed_status = JSON.parse(request.responseText);\r\
            var mac_addr = document.getElementById(\"MAC_Addr\");\r\
            mac_addr.innerHTML = parsed_status.MAC_Addr;\r\
            var tcpip_addr = document.getElementById(\"TCPIP_Addr\");\r\
            tcpip_addr.innerHTML = parsed_status.TCPIP_Addr;\r\
            var link_speed = document.getElementById(\"LinkSpeed\");\r\
            link_speed.innerHTML = parsed_status.LinkSpeed;\r\
            var current_time = document.getElementById(\"CurrentRTCTime\");\r\
            current_time.innerHTML = parsed_status.CurrentRTCTime;\r\
            var current_date = document.getElementById(\"Current_RTC_Date\");\r\
            current_date.innerHTML = parsed_status.Current_RTC_Date;\r\
        };\r\
    };\r\
    request.send(null);\r\
};\r\
\r\
function encodeFormData(data) {\r\
    if (!data) return \"\";\r\
    var pairs = [];\r\
    for(var name in data) {\r\
        if (!data.hasOwnProperty(name)) continue;\r\
        if (typeof data[name] === \"function\") continue;\r\
        var value = data[name].toString();\r\
        name = encodeURIComponent(name.replace(\" \", \"+\"));\r\
        value = encodeURIComponent(value.replace(\" \", \"+\"));\r\
        pairs.push(name + \"=\" + value);\r\
    }\r\
    return pairs.join('&');\r\
}\r\
\r\
function post_new_time_request() {\r\
    var req_data = { secs:0, ns:0};\r\
    req_data.secs = document.getElementById(\"SetTime\").value;\r\
    var request = new XMLHttpRequest();\r\
    request.open(\"GET\",\"trigger0?\" + encodeFormData(req_data));\r\
    request.send(null);\r\
};\r\
\r\
function post_new_date_request() {\r\
    var req_data = { secs:0, ns:0};\r\
    req_data.secs = document.getElementById(\"SetDate\").value;\r\
    var request = new XMLHttpRequest();\r\
    request.open(\"GET\",\"trigger1?\" + encodeFormData(req_data));\r\
    request.send(null);\r\
};\r\
\r\
window.onload = function() {\r\
    document.getElementById(\"SetTime\").value = \"09:15:35\";\r\
    document.getElementById(\"SetDate\").value = \"01/01/13\";\r\
    setInterval(update_page, 500);\r\
};\r\
</script>\r\
</body></html>";

/*------------------------------------------------------------------------------
 *
 */
static char status_json[400];

/*------------------------------------------------------------------------------
 *
 */

extern mss_mac_speed_t g_net_speed;


/** Serve one HTTP connection accepted in the http thread */
#if defined(_ZL303XX_MIV)
static void
http_server_netconn_serve
(
    struct netconn *conn,
    uint32_t count
)
#else
static void
http_server_netconn_serve
(
    struct netconn *conn,
    mss_rtc_calendar_t * calendar_count
)
#endif
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;

#ifndef USB_DEVICE_RNDIS
    const char * mac_speed_lut[] =
    {
        "10Mbps",
        "100Mbps",
        "1000Mbps",
        "invalid"
    };
#endif

    /* Read the data from the port, blocking if nothing yet there. 
    We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &inbuf);

    if(err == ERR_OK)
    {
        netbuf_data(inbuf, (void**)&buf, &buflen);
    
        /* Is this an HTTP GET command? (only check the first 5 chars, since
        there are other formats for GET, and we're keeping it very simple )*/
        if(buflen >= 5 &&
           buf[0] == 'G' &&
           buf[1] == 'E' &&
           buf[2] == 'T' &&
           buf[3] == ' ' &&
           buf[4] == '/' )
        {
        
            if(buf[5]=='s')
            {
                uint32_t json_resp_size;
                uint32_t ip_addr;
                uint8_t mac_addr[6];
#ifndef USB_DEVICE_RNDIS
                uint32_t mac_speed_idx;
#endif
#if LWIP_IPV4
                ip_addr = get_ipv4_address(IP4_IP_ADDR);
#endif
                get_mac_address(mac_addr);
#ifndef USB_DEVICE_RNDIS

 /*  ToDo: MIV/CoreTSE option...             mac_speed_idx = SYSREG->MAC_CR & 0x00000003u; */
#if defined(TARGET_G5_SOC)
                mac_speed_idx = g_net_speed; /* PMCS: hack for test on GEM with emulation platform */
#else
                mac_speed_idx = 2;
#endif
                json_resp_size = snprintf(status_json, sizeof(status_json),
                                          "{\r\n\"MAC_Addr\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"TCPIP_Addr\": \"%d.%d.%d.%d\",\r\n\"LinkSpeed\": \"%s\"\r,\
                                          \r\n",
                                          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
                                          (int)(ip_addr & 0x000000FFu),
                                          (int)((ip_addr >> 8u) & 0x000000FFu),
                                          (int)((ip_addr >> 16u) & 0x000000FFu),
                                          (int)((ip_addr >> 24u) & 0x000000FFu),
                                          mac_speed_lut[mac_speed_idx]);


#else
				json_resp_size = snprintf(status_json, sizeof(status_json),
										  "{\r\n\"MAC_Addr\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"TCPIP_Addr\": \"%d.%d.%d.%d\",\r\n\"LinkSpeed\": \"%s\"\r,\
										  \r\n",
										  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
										  (int)(ip_addr & 0x000000FFu),
										  (int)((ip_addr >> 8u) & 0x000000FFu),
										  (int)((ip_addr >> 16u) & 0x000000FFu),
										  (int)((ip_addr >> 24u) & 0x000000FFu),
										  MSS_USBD_is_high_speed() ? "480Mbps" : "12Mbs");
#endif
#if defined(_ZL303XX_MIV)
                json_resp_size += snprintf(&status_json[json_resp_size], sizeof(status_json),
                                           "\"CurrentRTCTime\": \"%d\" }\r\n",
                                           count);

#else
                json_resp_size += snprintf(&status_json[json_resp_size], sizeof(status_json),
                                           "\"CurrentRTCTime\": \"%02d:%02d:%02d\", ",
                                           calendar_count->hour,
                                           calendar_count->minute,
                                           calendar_count->second);

                json_resp_size += snprintf(&status_json[json_resp_size], sizeof(status_json),
                                           "\"Current_RTC_Date\": \"%02d/%02d/20%02d\" }\r\n",
                                           calendar_count->day,
                                           calendar_count->month,
                                           calendar_count->year);
#endif
                assert(json_resp_size < sizeof(status_json));
                if(json_resp_size > sizeof(status_json))
                {
                    json_resp_size = sizeof(status_json);
                }

                /* Send the HTML header 
                * subtract 1 from the size, since we dont send the \0 in the string
                * NETCONN_NOCOPY: our data is const static, so no need to copy it
                */
                netconn_write(conn, http_json_hdr, sizeof(http_json_hdr)-1, NETCONN_NOCOPY);

                /* Send our HTML page */
                netconn_write(conn, status_json, json_resp_size-1, NETCONN_NOCOPY);
            }
            else if(buf[5]=='i')
            {
                netconn_write(conn, mscc_jpg_logo, sizeof(mscc_jpg_logo)-1, NETCONN_NOCOPY);
            }
            else if(buf[5]=='t')
            {
                handle_trigger_request(&buf[5], buflen - 5);
                netconn_write(conn, http_html_ok_hdr, sizeof(http_html_ok_hdr)-1, NETCONN_NOCOPY);
            }
            else 
            {
                /* Send the HTML header 
                     * subtract 1 from the size, since we dont send the \0 in the string
                     * NETCONN_NOCOPY: our data is const static, so no need to copy it
                */
                netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
              
                /* Send our HTML page */
                netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
            }
        }
        else if (buflen>=6 &&
                 buf[0]=='P' &&
                 buf[1]=='O' &&
                 buf[2]=='S' &&
                 buf[3]=='T' &&
                 buf[4]==' ' &&
                 buf[5]=='/' )
        {
        
        err = netconn_recv(conn, &inbuf);
        
        ++err;
        
        /* Send the HTML header 
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
        */
        netconn_write(conn, http_post_resp_hdr, sizeof(http_post_resp_hdr)-1, NETCONN_NOCOPY);
    }
    }
    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);
  
    /* Delete the buffer (netconn_recv gives us ownership,
      so we have to make sure to deallocate the buffer) */
    netbuf_delete(inbuf);
}

/** The main function, never returns! */
void
http_server_netconn_thread(void *arg);
void
http_server_netconn_thread(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
#if defined(_ZL303XX_MIV)
    	uint32_t counter = 0;
#endif
    (void)arg;

#if !defined(_ZL303XX_MIV)
    MSS_RTC_init(MSS_RTC_CALENDAR_MODE, RTC_PRESCALER);
    MSS_RTC_start();
#endif
    /* Create a new TCP connection handle */
#if LWIP_IPV6
    /*
     * The following will allow connection with IPV6 and IPV4 so works for dual
     * stack as well.
     * */
    conn = netconn_new(NETCONN_TCP_IPV6);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);
  
    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind(conn, IP6_ADDR_ANY, 80);
#else /* Must be IPV4 only */
    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);
  
    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind(conn, IP_ADDR_ANY, 80);
#endif
    /* Put the connection into LISTEN state */
    netconn_listen(conn);
  
    do {
#if defined(_ZL303XX_MIV)
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            http_server_netconn_serve(newconn, counter);
            netconn_delete(newconn);
        }
        counter++;
#else
        mss_rtc_calendar_t calendar_count;
        uint32_t rtc_count_updated;
        
        rtc_count_updated = MSS_RTC_get_update_flag();
        if(rtc_count_updated)
        {
            MSS_RTC_get_calendar_count(&calendar_count);
            MSS_RTC_clear_update_flag();
        }
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            http_server_netconn_serve(newconn, &calendar_count);
            netconn_delete(newconn);
        }
#endif
    } while(err == ERR_OK);
    LWIP_DEBUGF(HTTPD_DEBUG,
                ("http_server_netconn_thread: netconn_accept received error %d, shutting down",
                err));
    netconn_close(conn);
    netconn_delete(conn);
}

/** Initialize the HTTP server (start its thread) */
void
http_server_netconn_init(void)
{
  sys_thread_new("http_server_netconn", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}

/** */
void handle_trigger_request(char * buf, u16_t len)
{
    unsigned int trigger_id;
    unsigned int seconds = 1;
    int idx;
    int time[3] = {0, 0, 0};
    int time_inc = 0;
    
    trigger_id = buf[7] - '0';
    if(trigger_id < 3)
    {
        /* Parse request for the trigger's seconds value. */
        idx = 14;
        while((buf[idx] != '&') && (idx < len) && (time_inc < 3))
        {
            if('%' == buf[idx])
            {
                idx += 3;   /* skip %3A. */
                ++time_inc;
            }
            else
            {
                if((buf[idx] >= '0') && (buf[idx] <= '9'))
                {
                    time[time_inc] = (time[time_inc] * 10) + buf[idx] - '0';
                    ++idx;
                }
                else
                {   /* Invalid character found in request. */
                    seconds = 0;
                    idx = len;
                }
            }
        }
        
        if(seconds != 0)
        {
#if 1
#else
            mss_rtc_calendar_t new_calendar_time;
            
            MSS_RTC_get_calendar_count(&new_calendar_time);
            if(0 == trigger_id)
            {
                new_calendar_time.hour = (uint8_t)time[0];
                new_calendar_time.minute = (uint8_t)time[1];
                new_calendar_time.second = (uint8_t)time[2];
            }
            else if(1 == trigger_id)
            {
                if((time[0] > 0) && (time[0] <= 31))
                {
                    new_calendar_time.day = (uint8_t)time[0];
                }
                if((time[1] > 0) && (time[1] <= 12))
                {
                    new_calendar_time.month = (uint8_t)time[1];
                }
                if((time[2] > 0) && (time[2] <= 255))
                {
                    new_calendar_time.year = (uint8_t)time[2];
                }
            }
            MSS_RTC_set_calendar_count(&new_calendar_time);
#endif
        }
    }
}

#endif /* LWIP_NETCONN*/
