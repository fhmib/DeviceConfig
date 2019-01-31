#ifndef _DC_COMMON_H_
#define _DC_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>

#define U8                          unsigned char
#define U16                         unsigned short
#define U32                         unsigned int
#define U64                         unsigned long long

#define ON_BOARD                    0
#define PRINT_COMMAND               1
#define SOCKET_TEST                 1       //means local test, it will send socket with msg.node=(sa+1)

#define IPSTAT_FREQ                 3

#if ON_BOARD
#define STATUS_PATH                 "/etc/boa/status.json"
#define CONFIG_PATH                 "/etc/boa/config.json"
#else
#define STATUS_PATH                 "status.json"
#define CONFIG_PATH                 "config.json"
#endif
#define INIT_PATH                   "init.json"
#define DEFAULT_PATH                "default.json"
#define XD_INIT_PATH                "init.sh"
#define XD_CONFIG_PATH              "config.ini"
#define INFO_PATH                   "devinfo"

#define UART0_PATH                  "/dev/ttyPS1"
#define NET_PATH                    "/proc/net/dev"
#define KEY_PATH                    "/etc/profile"
#define VOLTAGE_PATH                "/sys/bus/iio/devices/iio:device0/in_voltage8_raw"
#define TEMP_PATH                   "/sys/bus/iio/devices/iio:device0/in_temp0_raw"
#define WIFI_PATH_AP                "/home/wifi/ap.sh"
#define WIFI_PATH_SAP               "/home/wifi/release/start_ap.sh"
#define WIFI_PATH_SHUTDOWN              "/home/wifi/release/remove_all.sh"
#define WIFI_PATH_OPEN              "/home/wifi/release/wpa_supplicant_open.conf"
#define WIFI_PATH_TKIP              "/home/wifi/release/wpa_supplicant_tkip.conf"
#define WIFI_PATH_CCMP              "/home/wifi/release/wpa_supplicant_ccmp.conf"
#define WIFI_PATH_WEP64             "/home/wifi/release/wpa_supplicant_wep64.conf"
#define WIFI_PATH_WEP128            "/home/wifi/release/wpa_supplicant_wep128.conf"
#define WIFI_NAME                   "wifi1"
#if ON_BOARD
#define NET_DEV_NAME                "br0"
#else
#define NET_DEV_NAME                "ens33"
#endif
#define AUDIO_NAME                  "audio_test"

#define GROUP_IP                    "224.0.1.129"

//micro NODE_IP is absoleted
//#define NODE_IP                     "192.168.0.5"

#define SN_MNCONF                   8
#define SN_DEVCFG                   11
#define MN_REQ_ROUTE_TOP            100
#define MMSG_MN_GUIIN               2004
#define MMSG_MN_GUIOUT              2005
#define MMSG_MN_RESET               2006
#define MN_REQ_MAC_STATE            3100
#define RESET_SET_AAR               14504
#define MN_REP_MAC                  21002
#define MN_REP_ROUTE                21003

#define MAX_SOCK_LEN                8192
#define MAX_MSG_LEN                 8192
#define MUL_PORT                    9000
#define INFO_PORT                   9001
#define NAME_LEN                    64
#define MAX_NODE_CNT                32
#define MAX_TIMEOUT                 15
#define PORT_CNT                    1
#define ROUTE_CNT                   4

#define MMAX(a,b)                   ((a > b) ? a : b)

#define CNAME_NULL                  "null"
#define CNAME_NOTDEF                "notDefine"
#define CNAME_NODE                  "node"

#define CNAME_STATUS                "Status"
#define CNAME_DVLP                  "Developer"
#define CNAME_INFO                  "Information"
#define CNAME_SIGTABLE              "SignalQualityTable"
#define CNAME_NODEHEADER            "NodeStatus"
#define CNAME_IPSTATUS              "IpStatus"
#define CNAME_IPTXBYTE              "IPTxBytes"
#define CNAME_IPTXPKT               "IPTxPackets"
#define CNAME_IPTXERR               "IPTxErrorPackets"
#define CNAME_IPRXBYTE              "IPRxBytes"
#define CNAME_IPRXPKT               "IPRxPackets"
#define CNAME_IPRXERR               "IPRxErrorPackets"
#define CNAME_FLAGS                 "Flags"
#define CNAME_OL                    "Online"
#define CNAME_CONFIG                "Config"
#define CNAME_RESET                 "Reset"
#define CNAME_RESTART               "Restart"
#define CNAME_MAIN                  "Main"
#define CNAME_AUDIO                 "Audio"
#define CNAME_USER                  "User"
#define CNAME_USERNAME              "name"
#define CNAME_USERPASSWORD          "password"
#define CNAME_DATAPORT              "SerialPort"
#define CNAME_DATAPORT0             "SerialPort0"
#define CNAME_GPS                   "GPS"
#define CNAME_ROUTE                 "StaticRoute"
#define CNAME_ROUTE0                "StaticRoute0"
#define CNAME_ROUTE1                "StaticRoute1"
#define CNAME_ROUTE2                "StaticRoute2"
#define CNAME_ROUTE3                "StaticRoute3"
#define CNAME_NODEID                "NodeId"
#define CNAME_NODENAME              "NodeName"
#define CNAME_VOLTAGE               "SupplyVoltage"
#define CNAME_TEMP                  "FPGATemperature"
#define CNAME_MESHID                "MeshId"
#define CNAME_SOPINT                "SopInterval"
#define CNAME_FREQ                  "CentreFrequency"
#define CNAME_CHANBW                "ChannelBandwidth"
#define CNAME_TFCI                  "MCS"
#define CNAME_TXPOWER               "TxPower"
#define CNAME_MIMO                  "MimoMode"
#define CNAME_IPADDRESS             "IPAddress"
#define CNAME_IPMASK                "SubnetMask"
#define CNAME_IPGATEWAY             "Gateway"
#define CNAME_AUDIOENABLE           "AudioMode"
#define CNAME_AUDIOMIC              "MicrophoneVolume"
#define CNAME_AUDIOPLAY             "HeadphoneVolume"
#define CNAME_AUDIOALC              "MuteThreshold"
#define CNAME_UARTRATE              "BaudRate"
#define CNAME_UARTPARITY            "Parity"
#define CNAME_UARTDATAWIDTH         "DataWidth"
#define CNAME_UARTSTOP              "StopBits"

#define CNAME_WIFI                  "Wifi"
#define CNAME_WIFIMODE              "Mode"
#define CNAME_WIFIIP                "IPAddress"
#define CNAME_WIFIENCRY             "Encryption"
#define CNAME_WIFISSID              "SSID"
#define CNAME_WIFIPW                "Password"
#define CNAME_WIFIBW                "Bandwidth"
#define CNAME_WIFICHAN              "Channel"
#define CNAME_WIFITXPOWER           "TxPower"

#define CNAME_GPSENABLE             "GPSMode"
#define CNAME_GPSUTCTIME            "UTCTime"
#define CNAME_GPSUTCDATE            "UTCDate"
#define CNAME_GPSSATECNT            "NumberofSatellites"
#define CNAME_GPSLAT                "Latitude"
#define CNAME_GPSLATHEM             "LatitudeHemisphere"
#define CNAME_GPSLONG               "Longitude"
#define CNAME_GPSLONGHEM            "LongitudeHemisphere"
#define CNAME_GPSSPEED              "GroundSpeed"
#define CNAME_GPSCOURSE             "GroundCourse"
#define CNAME_GPSHDOP               "HDOP"
#define CNAME_GPSVDOP               "VDOP"
#define CNAME_GPSHEIGHT             "Altitude"
#define CNAME_GPSTYPE               "AlignType"

//#define CNAME_ROUTEADDR             "Network/HostIPAddress"
#define CNAME_ROUTEADDR             "IPAddress"
#define CNAME_ROUTEMASK             "SubnetMask"
#define CNAME_ROUTEGATE             "Gateway"
/*
#define CNAME_ROUTE0ADDR            "staticRoute0Network"
#define CNAME_ROUTE1ADDR            "staticRoute1Network"
#define CNAME_ROUTE2ADDR            "staticRoute2Network"
#define CNAME_ROUTE3ADDR            "staticRoute3Network"
#define CNAME_ROUTE0MASK            "staticRoute0SubMask"
#define CNAME_ROUTE1MASK            "staticRoute1SubMask"
#define CNAME_ROUTE2MASK            "staticRoute2SubMask"
#define CNAME_ROUTE3MASK            "staticRoute3SubMask"
#define CNAME_ROUTE0GATE            "staticRoute0GateWay"
#define CNAME_ROUTE1GATE            "staticRoute1GateWay"
#define CNAME_ROUTE2GATE            "staticRoute2GateWay"
#define CNAME_ROUTE3GATE            "staticRoute3GateWay"
*/

#define CNAME_SOFTVER               "SoftwareVersion"
#define CNAME_PROVER                "ProtocolVersion"
#define CNAME_FPGAVER               "FPGAVersion"
#define CNAME_BOARDTYPE             "BoardType"
#define CNAME_SERIAL                "SerialNumber"
#define CNAME_MACADDR               "MacAddress"

#define CNAME_NETSTATUS             "NetworkStatus"
#define CNAME_TBS                   "TBSAddress"
#define CNAME_UPS                   "UPSAddress"
#define CNAME_CLOCK                 "ClockLevel"
#define CNAME_SLOTCNT               "NumberofSlots"
#define CNAME_L2BNUM                "BBTxPackets"
#define CNAME_B2LNUM                "BBRxPackets"
#define CNAME_BBSFNUM               "ServiceFrame"
#define CNAME_LMRETX                "ReTxCount"
#define CNAME_TXFAIL                "TxFailCount"
#define CNAME_FORWARD               "ForwardingCount"
#define CNAME_MAXTXBYTE             "MaxTxByte"
#define CNAME_ACTUALTXBYTE          "ActualTxByte"
#define CNAME_VMODE                 "MCSMode"
#define CNAME_FSTABLE               "FixedSlots"
#define CNAME_DSTABLE               "DynamicSlots"

#define CNAME_MTUTABLE              "TfciTable"
#define CNAME_SNR                   "SNR"
#define CNAME_NOISE0                "Noise0"
#define CNAME_NOISE1                "Noise1"
#define CNAME_DISTANCE              "Distance"
#define CNAME_RSSI0                 "RSSI0"
#define CNAME_RSSI1                 "RSSI1"

//#define JSON_BRACKET                0
//#define JSON_NORMAL                 1
//#define JSON_ARRAY                  2
//#define JSON_STRING                 3
//#define JSON_CUSTOM1                4

typedef char pgps[32];

typedef enum {
    JSON_BRACKET,
    JSON_NORMAL,
    JSON_ARRAY,
    JSON_STRING,
    JSON_CUSTOM1,
} JSON_TYPE;

typedef enum {
    SMSG_REQ,
    SMSG_INFO,
} SMSG_TYPE;

typedef struct _smsg_t {
    U8 type;
    U8 node;
    char buf[MAX_SOCK_LEN];
} smsg_t;
#define SMSG_LEN sizeof(smsg_t)

typedef struct _mmsg_t {
    long mtype;
    U8 node;
    char data[MAX_MSG_LEN];
} mmsg_t;
#define MMSG_LEN sizeof(mmsg_t)

typedef struct _mnhd_t {
    long type;
} mnhd_t;
#define MNHD_LEN sizeof(mnhd_t)

typedef struct _node_l node_l;
typedef struct _node_s node_s;

typedef struct _node_l {
    node_s* pnode;
    struct _node_l* next;
} node_l;

typedef struct _node_s {
    char type; //type of node

    char name[64]; //name of parameter
    char* pvalue; //pointer to the value of parameter
    char isstr; //whether it is a string

    node_l child_h; //head of child
    node_l child_t; //tail of child
} node_s;

#if 0
//struct for store read-only data
typedef struct _odata_s{
    char name[64];
    char *pvalue;
}odata_s;

//struct for store local data
typedef struct _data_s{
    char name[64];
    char *pvalue;
    //int length;                     //length of value
    int *(*pfunc)(char*, int);    //pointer to function reads/writes value of the parameter
    char fname[64];                 //father's name of tree
}data_s;
#endif

//struct for store status data and transfer
typedef struct _sdata_s {
    char type;
    char name[64];
    char* pvalue;
    //int length;                     //length of value
    int (*pfunc)(int, char, char*); //pointer to function reads/writes value of the parameter
    char fname[64]; //father's name of tree
    char isstr;
} sdata_s;

#if 0
//struct for store local config data
typedef struct _cdata_s{
    char name[64];
    char *pvalue;
    //int length;                     //length of value
    char *(*pfunc)(char*, char);    //pointer to function reads/writes value of the parameter
    //char fname[64];                 //father's name of tree
}cdata_s;
#endif

//struct for store data from json file
typedef struct _rdata_s {
    char name[64];
    char* pvalue;
    struct _rdata_s* next;
} rdata_s;

typedef struct _tproc_t {
    char name[64];
    int period;
    int wait;
    void (*pf)(U32);
    U32 para;
} tproc_t;

typedef struct _timers_s {
    pthread_mutex_t mutex;
    U32 tmap;
    U32 tmask;
    tproc_t procs[32];
} timers_s;

typedef struct _mac_state {
    U16 rfnt;
    U8 sta;
    U8 tbs;
    U8 ups;
    U8 clks;
    U8 bsmap[32];
    U8 delay;
    U8 osn;
    U16 l2bnum;
    U16 b2lnum;
    U16 sfb2l;
    U16 lretxnum; /* lowmac retx num add 2019 01 14 */
    U16 ltxfailnum; /* lowmac tx fail num 2019 01 14 */
    U16 laafnum; /* lowmac aaf num 2019 01 14 */
    U16 ltxmaxlen; /* lowmac tx max len(bytes number) 2019 01 14 */
    U16 ltxinresultlen; /* lowmac tx len(bytes number)in result 2019 01 14 */
    U8 dsmap[55];
    U8 vmode;
} mac_state;

void print_info(char*);
int init_tree();
int init_device();
void update_rdonly();
void update_sig();
void update_info();
void update_dvlp();
void update_config(rdata_s*);
void* timer_thread(void*);
int timer_add(const char*, int, void (*)(U32), U32);
void do_task();
void chk_online(U32);
void chk_config(U32);
void chk_reset(U32);
void sub_timeout(U32);
void ip_status(U32);
int cmp_config(rdata_s*);
int add_config();
//void reset_config(rdata_s*);
void update_local_status();
void add_local_status();
void insert_status_2tree(int, node_s*);
void remove_status(int);
//void _remove_status(node_s*);
void* rcv_thread(void*);
int dc_init();
void send_req();
int send_info(int, void*);
void update_time(int, int);
int update_node_status(int, char*);
int stat2tree(node_s*, sdata_s*);
void* gps_thread(void*);
void print_sdata();

int gen_json(const char*, node_s*);
void first_tree(FILE*, int, node_s*);
int gen_tree(node_s*, sdata_s*, int);
node_s* create_node(int, const char*, const char*, char);
void insert_node(node_s*, node_s*);
int del_node(node_s*, const char*);
void _del_node(node_s*);
int remove_childs(node_s*);
void mod_node(node_s*, const char*);
node_s* search_node(node_s*, const char*);
rdata_s* read_json(const char*, const char*);
void free_rdata(rdata_s*);
void modify_value(char**, const char*);
int read_ipaddr(const char*, int*);
int ipisnull(const char*);
int ipishost(const char*);
int getnumfromstr(char*);

#endif
