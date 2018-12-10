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
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
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

#define STATUS_PATH                 "status.json"
#define CONFIG_PATH                 "config.json"
#define INIT_PATH                   "init.json"
#define DEFAULT_PATH                "default.json"
#define XD_INIT_PATH                "init.sh"
#define XD_CONFIG_PATH              "config.ini"
#define INFO_PATH                   "devinfo"

#define NET_PATH                    "/proc/net/dev"
#define NET_DEV_NAME                "ens33"

#define GROUP_IP                    "224.0.1.129"
#define NODE_IP                     "192.168.0.106"

#define MAX_MSG_LEN                 1024
#define PORT                        9001
#define NAME_LEN                    64
#define MAX_NODE_CNT                32
#define MAX_TIMEOUT                 15

#define MMAX(a,b)                   ((a > b) ? a : b)

#define CNAME_SERIAL                "serialNumber"
#define CNAME_BOARDTYPE             "boardType"

//#define JSON_BRACKET                0
//#define JSON_NORMAL                 1
//#define JSON_ARRAY                  2
//#define JSON_STRING                 3
//#define JSON_CUSTOM1                4

typedef enum{
    JSON_BRACKET,
    JSON_NORMAL,
    JSON_ARRAY,
    JSON_STRING,
    JSON_CUSTOM1,
}JSON_TYPE;

typedef enum{
    MMSG_REQ,
    MMSG_INFO,
}MSG_TYPE;

typedef struct mmsg_t{
    U8 type;
    U8 node;
    char buf[MAX_MSG_LEN];
}mmsg_t;
#define MMSG_LEN        sizeof(mmsg_t)

typedef struct _node_l node_l;
typedef struct _node_s node_s;

typedef struct _node_l{
    node_s *pnode;
    struct _node_l *next;
}node_l;

typedef struct _node_s{
    char type;              //type of node

    char name[64];          //name of parameter
    char *pvalue;           //pointer to the value of parameter

    node_l child_h;        //head of child
    node_l child_t;        //tail of child
}node_s;

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

//struct for store status data and transfer
typedef struct _sdata_s{
    char type;
    char name[64];
    char *pvalue;
    //int length;                     //length of value
    int (*pfunc)(int, char, char*);    //pointer to function reads/writes value of the parameter
    char fname[64];                 //father's name of tree
}sdata_s;

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
typedef struct _rdata_s{
    char name[64];
    char *pvalue;
    struct _rdata_s *next;
}rdata_s;

typedef struct _tproc_t{
    char name[64];
    int period;
    int wait;
    void (*pf)(U32);
    U32 para;
}tproc_t;

typedef struct _timers_s{
    pthread_mutex_t mutex;
    U32 tmap;
    U32 tmask;
    tproc_t procs[32];
}timers_s;

void print_info(char*);
int init_tree();
int config_device();
void update_rdonly();
void update_sig();
void update_info_t();
void update_dvlp_t();
void update_config(rdata_s*);
void *timer_thread(void*);
int timer_add(const char*, int, void (*)(U32), U32);
void do_task();
void chk_online(U32);
void chk_config(U32);
void chk_reset(U32);
void sub_timeout(U32);
void ip_status(U32);
int cmp_config(rdata_s*);
void reset_config(rdata_s*);
void update_status();
void remove_status(int);
void _remove_status(node_s*);
void *rcv_thread(void*);
int dc_init();
void send_req();
int send_info(int, void*);
void update_time(int, int);
int update_node(int, char*);
int stat2tree(node_s*, sdata_s*);

int gen_json(const char*, node_s*);
void first_tree(FILE*, int, node_s*);
node_s *create_node(int, const char*, const char*);
void insert_node(node_s*, node_s*);
int del_node(node_s*, const char*);
void _del_node(node_s*);
void mod_node(node_s*, const char*);
node_s *search_node(node_s*,const char*);
rdata_s *read_json(const char*, const char*);
void free_rdata(rdata_s*);










#endif
