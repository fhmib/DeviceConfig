#include "dc_common.h"

U8 sa;
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;

//for status.json
node_s *status_root;
node_s *sflags;
node_s *sstatus;
node_s *sinformation;
node_s *sdeveloper;
node_s *ssigtable;
node_s *snode_data[MAX_NODE_CNT];

//genernal timers for process
static timers_s gts;

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
node_s *cflags;
node_s *csettings;
node_s *cmain;
node_s *caudio;
node_s *cdataport;
node_s *croute;

cdata_s config_t[] = {
    {"nodeId", NULL, NULL, "main"},
    {"nodeName", NULL, NULL, "main"},
    {"meshId", NULL, NULL, "main"},
    {"centreFreq", NULL, NULL, "main"},
    {"chanBandwidth", NULL, NULL, "main"},
    {"TX1Power", NULL, NULL, "main"},
    {"TX2Power", NULL, NULL, "main"},
    {"reduceMimo", NULL, NULL, "main"},
    {"ipAddress", NULL, NULL, "main"},
    {"ipMask", NULL, NULL, "main"},
    {"ipGateway", NULL, NULL, "main"},
    {"audioEnable", NULL, NULL, "audio"},
    {"audioHeadGain", NULL, NULL, "audio"},
    {"audioMicGain", NULL, NULL, "audio"},
    {"audioMuteLevel", NULL, NULL, "audio"},
    {"data0BaudRate", NULL, NULL, "dataPort"},
    {"data0Parity", NULL, NULL, "dataPort"},
    {"data0StopBits", NULL, NULL, "dataPort"},
    {"data0FlowControl", NULL, NULL, "dataPort"},
    {"data0Width", NULL, NULL, "dataPort"},
};

int main(int argc, char *argv[])
{
    sigset_t t_set;
    pthread_t tid;

    //for test
    sa = 1;

    init_tree();

    /*
    del_node(status_root, sstatus->name);
    del_node(status_root, "flags");
    del_node(status_root, "sigQualityTable");
    del_node(status_root, "information");
    del_node(status_root, "developer");
    */

    gen_json(STATUS_PATH, status_root);
    gen_json(CONFIG_PATH, config_root);

    sigemptyset(&t_set);
    sigaddset(&t_set, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &t_set, NULL);

    timer_add("check online", 1, chk_online, 0);
    timer_add("check config", 1, chk_config, 0);
    timer_add("check reset", 1, chk_reset, 0);
    timer_add("sub timeout", 2, sub_timeout, 0);

    pthread_create(&tid, NULL, timer_thread, &t_set);

    while(1){
        sleep(10);
    }

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
    //insert_node(sflags, create_node(JSON_STRING, "reset", "0"));

    //initialize remote status
    insert_node(sstatus, remote);
    //node_s *testnode;
    for(i = 0; i < MAX_NODE_CNT; i++){
        snode_data[i] = create_node(JSON_BRACKET, NULL, NULL);
        insert_node(remote, snode_data[i]);
        insert_node(snode_data[i], create_node(JSON_STRING, "timeout", "0"));
#if 0
        //for test delete remote status
        testnode = create_node(JSON_NORMAL, "frhoqud", NULL);
        insert_node(snode_data[i], testnode);
        insert_node(testnode, create_node(JSON_STRING, "froib3h", "15"));
        insert_node(testnode, create_node(JSON_STRING, "froib3h", "15"));
        insert_node(testnode, create_node(JSON_STRING, "froib3h", "15"));
        insert_node(snode_data[i], create_node(JSON_STRING, "froib3h", "15"));
        insert_node(snode_data[i], create_node(JSON_STRING, "froib3h", "15"));
#endif
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
    cflags = create_node(JSON_NORMAL, "flags", NULL);
    csettings = create_node(JSON_NORMAL, "settings", NULL);
    cmain = create_node(JSON_NORMAL, "main", NULL);
    caudio = create_node(JSON_NORMAL, "audio", NULL);
    cdataport = create_node(JSON_NORMAL, "dataPort", NULL);
    croute = create_node(JSON_NORMAL, "staticRoute", NULL);

    insert_node(config_root, cflags);
    insert_node(cflags, create_node(JSON_STRING, "config", "0"));
    insert_node(cflags, create_node(JSON_STRING, "reset", "0"));
    insert_node(config_root, csettings);
    insert_node(csettings, cmain);
    insert_node(csettings, caudio);
    insert_node(csettings, cdataport);
    insert_node(csettings, croute);

    rdata_s *init;
    init = read_json(INIT_PATH, NULL);
    //need more code to process 'init==NULL' situation
    if(init != NULL){
        update_config(init);
        free_rdata(init);
    }else{
    }

    node_s *pnode;
    cnt = sizeof(config_t)/sizeof(cdata_s);
    for(i = 0; i < cnt; i++){
        pnode = search_node(config_root, config_t[i].fname);
        //printf("i=%d, pnode = %p\n", i, pnode);
        insert_node(search_node(config_root, config_t[i].fname), create_node(JSON_STRING, config_t[i].name, config_t[i].pvalue));
    }

#if 0
    //test delete node func
    for(i = cnt-1; i < cnt; i++){
        del_cnode(sdeveloper, dvlp_t[i].name);
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

void *timer_thread(void *arg)
{
    struct itimerval itv;
    sigset_t *t_set = (sigset_t*)arg;
    int rval, sig;

    pthread_detach(pthread_self());

    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &itv, NULL);

    while(1){
        rval = sigwait(t_set, &sig);
        if(rval != 0){
            perror("sigwait failed");
            rval = 1;
            pthread_exit((void*)&rval);
        }

        //printf("sig = %d\n", sig);
        do_task();
    }

    pthread_exit((void*)&rval);
}

int timer_add(const char *name, int intval, void (*pcb)(U32), U32 arg)
{
    int i = 0;
    U32 emap;

    pthread_mutex_lock(&gts.mutex);

    emap = gts.tmap;
    for(i = 0; i < 32; i++){
        if(!((1<<i)&emap)){
            strcpy(gts.procs[i].name, name);
            gts.procs[i].period = intval;
            gts.procs[i].wait = intval;
            fprintf(stderr, "add timer '%s' success, period = %ds\n", gts.procs[i].name, gts.procs[i].period);
            gts.procs[i].pf = pcb;
            gts.procs[i].para = arg;

            gts.tmap |= (U32)(1<<i);
            gts.tmask |= (U32)(1<<i);

            break;
        }
    }

    pthread_mutex_unlock(&gts.mutex);

    return i;
}

void do_task()
{
    int i;
    U32 emap;

    emap = gts.tmap & gts.tmask;
    for(i = 0; i < 32; i++){
        if(!((1<<i)&(emap))){
            continue;
        }

        if(gts.procs[i].wait > 0){
            gts.procs[i].wait -= 1;
        }else{
            gts.procs[i].wait = 0;
        }

        if(gts.procs[i].wait <= 0){
            gts.procs[i].wait = gts.procs[i].period;
            (*gts.procs[i].pf)(gts.procs[i].para);
        }
    }

    return ;
}

/*
 * func:
 *      check online flag each 1 second, if it is 1, devcfg will obtain local node status and send other nodes the request.
 */
void chk_online(U32 arg)
{
    rdata_s *pd = NULL;

    //printf("I'm in %s, arg = %d\n", __func__, arg);

    if((pd = read_json(STATUS_PATH, "online")) == NULL){
        fprintf(stderr, "status.json is doubted broken\n");
        gen_json(STATUS_PATH, status_root);
        goto func_exit;
    }

    if(0 == (strcmp("1", pd->pvalue))){
        update_status();
        //need more code to send request to other node

        gen_json(STATUS_PATH, status_root);
    }

func_exit:
    if(pd != NULL){
        free_rdata(pd);
    }
    return ;
}

void chk_config(U32 arg)
{
    int mflag;
    rdata_s *pd = NULL;
    rdata_s *rdata = NULL;

    //printf("I'm in %s, arg = %d\n", __func__, arg);

    if((pd = read_json(CONFIG_PATH, "config")) == NULL){
        fprintf(stderr, "config.json is doubted broken\n");
        gen_json(CONFIG_PATH, config_root);
        goto func_exit;
    }

    if(0 == (strcmp("1", pd->pvalue))){
        rdata = read_json(CONFIG_PATH, NULL);
        mflag = cmp_config(rdata);
        gen_json(CONFIG_PATH, config_root);
        if(mflag > 0){
            gen_json(INIT_PATH, config_root);
        }
    }

func_exit:
    if(pd != NULL){
        free_rdata(pd);
    }
    if(rdata != NULL){
        free_rdata(rdata);
    }
    return ;
}

void chk_reset(U32 arg)
{
    rdata_s *pd = NULL;
    rdata_s *def = NULL;

    //printf("I'm in %s, arg = %d\n", __func__, arg);

    if((pd = read_json(CONFIG_PATH, "reset")) == NULL){
        fprintf(stderr, "config.json is doubted broken\n");
        gen_json(CONFIG_PATH, config_root);
        goto func_exit;
    }

    if(0 == (strcmp("1", pd->pvalue))){
        def = read_json(DEFAULT_PATH, NULL);
        if(def != NULL){
            update_config(def);
        }
        gen_json(CONFIG_PATH, config_root);
        gen_json(INIT_PATH, config_root);
    }

func_exit:
    if(pd != NULL){
        free_rdata(pd);
    }
    if(def != NULL){
        free_rdata(def);
    }
    return ;
}

char timeout_flag[MAX_NODE_CNT] = {0}; //1 means it can be subtracted from value that MAX_TIMEOUT defined

void sub_timeout(U32 arg)
{
    int i, value;
    char buf[8];
    char wflag = 0;
    node_s *node;

    //printf("I'm in %s, arg = %d\n", __func__, arg);

    for(i = 0; i < 32; i++){
        //printf("*******************\n");
        node = search_node(snode_data[i], "timeout");
        //printf("*******************\n");
        if(node != NULL){
            value = atoi(node->pvalue);
            //printf("--> timeout[%d] = %d, timeout_flag[%d] = %d\n", i, value, i, timeout_flag[i]);
            if(value > 0 && (i != (sa-1))){
                if(value == MAX_TIMEOUT){
                    pthread_mutex_lock(&pmutex);
                    if(timeout_flag[i] == 0){
                        timeout_flag[i] = 1;
                        pthread_mutex_unlock(&pmutex);
                        continue;
                    }else{
                        timeout_flag[i] = 0;
                    }
                    pthread_mutex_unlock(&pmutex);
                }
                value -= 1;
                sprintf(buf, "%d", value);
                mod_node(node, buf);
                wflag++;
                if(value == 0){
                    remove_status(i);
                }
            }
        }
    }

    if(wflag){
        gen_json(STATUS_PATH, status_root);
    }

    return ;
}

/*
 * func:
 *      compare config sinformation between config.json and local variable
 * ret:
 *      numbers of config successed parameters
 */
int cmp_config(rdata_s *rdata)
{
    int flag = 0;
    int cnt, i, len;
    node_s *node;

    cnt = sizeof(config_t)/sizeof(data_s);

    while(rdata != NULL){
        for(i = 0; i < cnt; i++){
            if(strcmp(rdata->name, config_t[i].name) == 0){
                if(config_t[i].pvalue == NULL){
                    //maybe need more code
                }
                if(strcmp(rdata->pvalue, config_t[i].pvalue) != 0){
                    if(config_t[i].pfunc != NULL){
                        //(*config_t[i].pfunc)(rdata->pvalue, 1);
                        
                        //if success
                        //update config_t
                        flag+=1;
                        len = strlen(rdata->pvalue);
                        if(len > (int)strlen(config_t[i].pvalue)){
                            free(config_t[i].pvalue);
                            config_t[i].pvalue = (char*)malloc(len + 1);
                        }
                        strcpy(config_t[i].pvalue, rdata->pvalue);
                        //update tree
                        node = search_node(config_root, rdata->name);
                        if(len > (int)strlen(node->pvalue)){
                            free(node->pvalue);
                            node->pvalue = (char*)malloc(len + 1);
                        }
                        strcpy(node->pvalue, rdata->pvalue);
                    }else{
                        printf("pfunc of %s is NULL, config failed\n", config_t[i].name);
                    }
                }
                break;
            }
        }
        rdata = rdata->next;
    }

    return flag;
}

static int num;
/*
 * func:
 *      update local status
 */
void update_status()
{
    node_s *pn;
    char buf[16];

    pn = search_node(sdeveloper, "BBSFCnt");
    if(pn == NULL){
        fprintf(stderr, "cannot find node\n");
        return ;
    }
    sprintf(buf, "%d", num);
    mod_node(pn, buf);
    num++;

    return ;
}

void remove_status(int i)
{
    node_l *p, *temp;

    p = &snode_data[i]->child_h;
    if(p->next != NULL){
        p = p->next;
        if(0 != strcmp(p->pnode->name, "timeout")){
            printf("error! 'timeout' is lost\n");
            return ;
        }
        temp = p;
        p = p->next;
        temp->next = NULL;
        while(p != NULL){
            _del_node(p->pnode);
            temp = p;
            p = p->next;
            free(temp);
        }
    }else{
        printf("error! 'timeout' is lost\n");
    }

    return ;
}


