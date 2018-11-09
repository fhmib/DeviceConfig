#include "dc_common.h"

data_s info_t[] = {
    {"softwareVersion", NULL, NULL},
    {"protocolVersion", NULL, NULL},
    {"fpgaVersion", NULL, NULL},
    {"serialNumber", NULL, NULL},
    {"boardType", NULL, NULL},
    {"phyAddress", NULL, NULL}
};

data_s dvlp_t[] = {
    {"tbsAddress", NULL, NULL},
    {"upsAddress", NULL, NULL},
    {"clockLevel", NULL, NULL},
    {"netStatus", NULL, NULL},
    {"slotCnt", NULL, NULL},
    {"BBTxCnt", NULL, NULL},
    {"BBRxCnt", NULL, NULL},
    {"BBSFCnt", NULL, NULL},
    {"bsTable", NULL, NULL},
    {"dsTable", NULL, NULL},
};

unsigned char sig_t[MAX_NODE_CNT][MAX_NODE_CNT];
node_s *sigs[MAX_NODE_CNT];

node_s *root;
node_s *flags;
node_s *status;
node_s *information;
node_s *developer;
node_s *sigtable;
node_s *rdata[MAX_NODE_CNT];

int main(int argc, char *argv[])
{
    init_tree();

    gen_json(STATUS_PATH, root);

    return 0;
}

int init_tree()
{
    int i, j, cnt;
    char buf[1024], temp[32];
    node_s *remote = create_node(JSON_ARRAY, "remoteStatus", NULL);

    root = create_node(JSON_BRACKET, NULL, NULL);
    flags = create_node(JSON_NORMAL, "flags", NULL);
    status = create_node(JSON_NORMAL, "status", NULL);
    sigtable = create_node(JSON_ARRAY, "sigQualityTable", NULL);
    information = create_node(JSON_NORMAL, "information", NULL);
    developer = create_node(JSON_NORMAL, "developer", NULL);

    insert_node(root, flags);
    insert_node(root, status);
    insert_node(root, sigtable);
    insert_node(root, information);
    insert_node(root, developer);

    //initialize flags
    insert_node(flags, create_node(JSON_STRING, "online", "0"));
    insert_node(flags, create_node(JSON_STRING, "default", "0"));

    //initialize remote status
    insert_node(status, remote);
    for(i = 0; i < MAX_NODE_CNT; i++){
        rdata[i] = create_node(JSON_BRACKET, NULL, NULL);
        insert_node(remote, rdata[i]);
        insert_node(rdata[i], create_node(JSON_STRING, "timeout", "0"));
    }

    //initialize signal quality table
    for(i = 0; i < MAX_NODE_CNT; i++){
        sigs[i] = create_node(JSON_CUSTOM1, NULL, NULL);
        insert_node(sigtable, sigs[i]);
    }
    update_sig();
    for(i = 0; i < MAX_NODE_CNT; i++){
        buf[0] = 0;
        for(j = 0; j < MAX_NODE_CNT; j++){
            sprintf(temp, "%u", sig_t[i][j]);
            strcat(buf, temp);
            if(j < MAX_NODE_CNT - 1){
                strcat(buf, ", ");
            }
        }
        if(sigs[i]->pvalue == NULL){
            sigs[i]->pvalue = (char*)malloc(strlen(buf)+1);
        }else{
            if(strlen(sigs[i]->pvalue) < strlen(buf)){
                free(sigs[i]->pvalue);
                sigs[i]->pvalue = (char*)malloc(strlen(buf)+1);
            }
        }
        strcpy(sigs[i]->pvalue, buf);
    }

    //initialize information table
    update_info();

    cnt = sizeof(info_t)/sizeof(data_s);
    for(i = 0; i < cnt; i++){
        insert_node(information, create_node(JSON_STRING, info_t[i].name, info_t[i].pvalue));
    }

    update_dvlp();

    cnt = sizeof(dvlp_t)/sizeof(data_s);
    for(i = 0; i < cnt; i++){
        insert_node(developer, create_node(JSON_STRING, dvlp_t[i].name, dvlp_t[i].pvalue));
    }

    return 0;
}

void update_sig()
{
    memset(sig_t, 255, sizeof(sig_t));

    return ;
}

void update_info()
{
    char buf[64];
    int i, cnt;

    cnt = sizeof(info_t)/sizeof(data_s);
    strcpy(buf, "1.1");
    for(i = 0; i < cnt; i++){
        info_t[i].pvalue = (char*)malloc(strlen(buf)+1);
        strcpy(info_t[i].pvalue, buf);
    }

    return ;
}

void update_dvlp()
{
    char buf[64];
    int i, cnt;

    cnt = sizeof(dvlp_t)/sizeof(data_s);
    strcpy(buf, "11");
    for(i = 0; i < cnt; i++){
        dvlp_t[i].pvalue = (char*)malloc(strlen(buf)+1);
        strcpy(dvlp_t[i].pvalue, buf);
    }

    return ;
}

