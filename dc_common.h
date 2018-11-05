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

#define FILE_NAME "status.json"

#define JSON_BRACKET                0
#define JSON_NORMAL                 1
#define JSON_ARRAY                  2
#define JSON_STRING                 3
#define JSON_CUSTOM1                4

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
    int length;             //length of value

    node_l *child_h;        //head of child
    node_l *child_t;        //tail of child
}node_s;

int gen_json(const char*);
void first_tree(FILE*, int, node_s*);












