#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>

#define U8                          unsigned char
#define U16                         unsigned short
#define U32                         unsigned int

#define STATUS_PATH                 "status.json"
#define CONFIG_PATH                 "config.json"
#define INIT_PATH                   "init.json"
#define DEFAULT_PATH                "default.json"
#define INFO_PATH                   "info"
#define NAME_LEN                    64
#define MAX_NODE_CNT                32
#define MAX_TIMEOUT                 15

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

//struct for store local data
typedef struct _data_s{
    char name[64];
    char *pvalue;
    //int length;                     //length of value
    char *(*pfunc)(char*, char);    //pointer to function reads/writes value of the parameter
}data_s;

//struct for store local config data
typedef struct _cdata_s{
    char name[64];
    char *pvalue;
    //int length;                     //length of value
    char *(*pfunc)(char*, char);    //pointer to function reads/writes value of the parameter
    char fname[64];                 //father's name of tree
}cdata_s;

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

int init_tree();
void update_sig();
void update_info();
void update_dvlp();
void update_config(rdata_s*);
void *timer_thread(void*);
int timer_add(const char*, int, void (*)(U32), U32);
void do_task();
void chk_online(U32);
void chk_config(U32);
void chk_reset(U32);
void sub_timeout(U32);
int cmp_config(rdata_s*);
void reset_config(rdata_s*);
void update_status();
void remove_status(int);
void _remove_status(node_s*);

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











