#include "dc_common.h"

//for status.json
node_s *status_root;
node_s *sflags;
node_s *sstatus;
node_s *sinformation;
node_s *sdeveloper;
node_s *ssigtable;
node_s *snode_data[MAX_NODE_CNT];

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

unsigned char sigQuality_t[MAX_NODE_CNT][MAX_NODE_CNT];
node_s *sigs[MAX_NODE_CNT];

//for config.json
node_s *config_root;
node_s *cflag;
node_s *csettings;
node_s *cmain;
node_s *caudio;
node_s *cdataport;
node_s *croute;

data_s config_t[] = {
    {"nodeId", NULL, NULL},
    {"nodeName", NULL, NULL},
    {"meshId", NULL, NULL},
    {"centreFreq", NULL, NULL},
    {"chanBandwidth", NULL, NULL},
    {"TX1Power", NULL, NULL},
    {"TX2Power", NULL, NULL},
    {"reduceMimo", NULL, NULL},
    {"ipAddress", NULL, NULL},
    {"ipMask", NULL, NULL},
    {"ipGateway", NULL, NULL},
    {"audioEnable", NULL, NULL},
    {"audioHeadGain", NULL, NULL},
    {"audioMicGain", NULL, NULL},
    {"audioMuteLevel", NULL, NULL},
    {"data0BaudRate", NULL, NULL},
    {"data0Parity", NULL, NULL},
    {"data0StopBits", NULL, NULL},
    {"data0FlowControl", NULL, NULL},
    {"data0Width", NULL, NULL},
};

int main()
{
    //init_tree();

    //gen_json(STATUS_PATH, status_root);
    //gen_json(CONFIG_PATH, config_root);

    return 0;
}

int init_tree()
{
    int i, j, cnt;
    char buf[1024], temp[32];
    node_s *remote = create_node(JSON_ARRAY, "remoteStatus", NULL);

    status_root = create_node(JSON_BRACKET, NULL, NULL);
    sflags = create_node(JSON_NORMAL, "flags", NULL);
    sstatus = create_node(JSON_NORMAL, "status", NULL);
    ssigtable = create_node(JSON_ARRAY, "sigQualityTable", NULL);
    sinformation = create_node(JSON_NORMAL, "information", NULL);
    sdeveloper = create_node(JSON_NORMAL, "developer", NULL);

    insert_node(status_root, sflags);
    insert_node(status_root, sstatus);
    insert_node(status_root, ssigtable);
    insert_node(status_root, sinformation);
    insert_node(status_root, sdeveloper);

    //initialize flags
    insert_node(sflags, create_node(JSON_STRING, "online", "0"));
    insert_node(sflags, create_node(JSON_STRING, "default", "0"));

    //initialize remote status
    insert_node(sstatus, remote);
    for(i = 0; i < MAX_NODE_CNT; i++){
        snode_data[i] = create_node(JSON_BRACKET, NULL, NULL);
        insert_node(remote, snode_data[i]);
        insert_node(snode_data[i], create_node(JSON_STRING, "timeout", "0"));
    }

    //initialize signal quality table
    for(i = 0; i < MAX_NODE_CNT; i++){
        sigs[i] = create_node(JSON_CUSTOM1, NULL, NULL);
        insert_node(ssigtable, sigs[i]);
    }
    update_sig();
    for(i = 0; i < MAX_NODE_CNT; i++){
        buf[0] = 0;
        for(j = 0; j < MAX_NODE_CNT; j++){
            sprintf(temp, "%u", sigQuality_t[i][j]);
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
        insert_node(sinformation, create_node(JSON_STRING, info_t[i].name, info_t[i].pvalue));
    }

    update_dvlp();
    cnt = sizeof(dvlp_t)/sizeof(data_s);
    for(i = 0; i < cnt; i++){
        insert_node(sdeveloper, create_node(JSON_STRING, dvlp_t[i].name, dvlp_t[i].pvalue));
    }

    config_root = create_node(JSON_BRACKET, NULL, NULL);
    cflag = create_node(JSON_STRING, "config", "0");
    csettings = create_node(JSON_NORMAL, "settings", NULL);
    cmain = create_node(JSON_NORMAL, "main", NULL);
    caudio = create_node(JSON_NORMAL, "audio", NULL);
    cdataport = create_node(JSON_NORMAL, "dataPort", NULL);
    croute = create_node(JSON_NORMAL, "staticRoute", NULL);

    insert_node(config_root, cflag);
    insert_node(config_root, csettings);
    insert_node(csettings, cmain);
    insert_node(csettings, caudio);
    insert_node(csettings, cdataport);
    insert_node(csettings, croute);

    rdata_s *init;
    init = read_json(INIT_PATH, NULL);
    if(init != NULL){
        update_config(init);
        free_rdata(init);
    }

    cnt = sizeof(config_t)/sizeof(data_s);
    for(i = 0; i < cnt; i++){
        insert_node(cdataport, create_node(JSON_STRING, config_t[i].name, config_t[i].pvalue));
    }

#if 0
    //test delete node func
    for(i = cnt-1; i < cnt; i++){
        del_node(sdeveloper, dvlp_t[i].name);
    }

    //test search and modify node
    node_s *node = search_node(status_root, "boardType");
    if(node == NULL){
        printf("search failed!\n");
    }else{
        mod_node(node, "13523523");
    }

    //test read data from a json file
    rdata_s *h;
    h = read_json(STATUS_PATH, NULL);
    if(h == NULL){
        printf("no data\n");
    }else{
        while(h != NULL){
            printf("--->%s: %s\n", h->name, h->pvalue);
            h = h->next;
        }
        free_rdata(h);
    }
    printf("\n");
    h = read_json(STATUS_PATH, "fpgaVersion");
    if(h == NULL){
        printf("no data\n");
    }else{
        while(h != NULL){
            printf("--->%s: %s\n", h->name, h->pvalue);
            h = h->next;
        }
        free_rdata(h);
    }
#endif

    return 0;
}

void update_sig()
{
    memset(sigQuality_t, 255, sizeof(sigQuality_t));

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

void update_config(rdata_s *init)
{
    int i, cnt, len;

    cnt = sizeof(config_t)/sizeof(data_s);

    while(init != NULL){
        for(i = 0; i < cnt; i++){
            if(strcmp(init->name, config_t[i].name) == 0){
                len = strlen(init->pvalue);
                if(config_t[i].pvalue == NULL){
                    config_t[i].pvalue = (char*)malloc(len + 1);
                }else{
                    if(len > (int)strlen(config_t[i].pvalue)){
                        free(config_t[i].pvalue);
                        config_t[i].pvalue = (char*)malloc(len + 1);
                    }
                }
                strcpy(config_t[i].pvalue, init->pvalue);
                break;
            }
        }
        init = init->next;
    }

    return ;
}

