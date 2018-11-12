#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <assert.h>

#define STATUS_PATH                 "status.json"
#define CONFIG_PATH                 "config.json"
#define INFO_PATH                   "info"
#define NAME_LEN                    64
#define MAX_NODE_CNT                32

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

typedef struct _rdata_s{
    char name[64];
    char *pvalue;
    struct _rdata_s *next;
}rdata_s;

int init_tree();
void update_sig();
void update_info();
void update_dvlp();
void update_main();
void update_audio();
void update_port0();

int gen_json(const char*, node_s*);
void first_tree(FILE*, int, node_s*);
node_s *create_node(int, const char*, const char*);
void insert_node(node_s*, node_s*);
int del_node(node_s*, char*);
void mod_node(node_s*, const char*);
node_s *search_node(node_s*,const char*);
rdata_s *read_json(const char*, const char*);











