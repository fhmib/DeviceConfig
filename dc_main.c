#include "dc_common.h"
#include "dc_io.h"

U8 sa;
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
int mc_fd;
struct sockaddr_in mc_addr;

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

//read-only data
odata_s rdonly_data[] = {
    {"serialNumber", NULL},
    {"boardType", NULL}
};
int rdonly_cnt = sizeof(rdonly_data)/sizeof(odata_s);

//local status table
sdata_s status_data[] = {
    {JSON_NORMAL, "nodeHeader", NULL, NULL, "null"},
    {JSON_STRING, "nodeId", NULL, &io_nodeId, "nodeHeader"},
    {JSON_STRING, "nodeName", NULL, &io_nodeName, "nodeHeader"},
    {JSON_STRING, CNAME_SERIAL, NULL, &io_undo, "nodeHeader"},
    {JSON_STRING, "ipAddress", NULL, &io_ipAddress, "nodeHeader"},
    {JSON_NORMAL, "ipStatus", NULL, NULL, "null"},
    {JSON_STRING, "ipTxByteCnt", NULL, &io_ipTxByteCnt, "ipStatus"},
    {JSON_STRING, "ipTxPktCnt", NULL, &io_ipTxPktCnt, "ipStatus"},
    {JSON_STRING, "ipTxErrorCnt", NULL, &io_ipTxErrorCnt, "ipStatus"},
    {JSON_STRING, "ipRxByteCnt", NULL, &io_ipRxByteCnt, "ipStatus"},
    {JSON_STRING, "ipRxPktCnt", NULL, &io_ipRxPktCnt, "ipStatus"},
    {JSON_STRING, "ipRxErrorCnt", NULL, &io_ipRxErrorCnt, "ipStatus"},
};
int status_cnt = sizeof(status_data)/sizeof(sdata_s);

//information table
data_s info_t[] = {
    {"softwareVersion", NULL, NULL, ""},
    {"protocolVersion", NULL, NULL, ""},
    {"fpgaVersion", NULL, NULL, ""},
    {"serialNumber", NULL, NULL, ""},
    {CNAME_BOARDTYPE, NULL, NULL, ""},
    {"phyAddress", NULL, NULL, ""}
};

//developer table
data_s dvlp_t[] = {
    {"tbsAddress", NULL, NULL, ""},
    {"upsAddress", NULL, NULL, ""},
    {"clockLevel", NULL, NULL, ""},
    {"netStatus", NULL, NULL, ""},
    {"slotCnt", NULL, NULL, ""},
    {"BBTxCnt", NULL, NULL, ""},
    {"BBRxCnt", NULL, NULL, ""},
    {"BBSFCnt", NULL, NULL, ""},
    {"bsTable", NULL, NULL, ""},
    {"dsTable", NULL, NULL, ""},
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

//config table
sdata_s config_t[] = {
    {JSON_STRING, "nodeId", NULL, &io_nodeId, "main"},
    {JSON_STRING, "nodeName", NULL, &io_nodeName, "main"},
    {JSON_STRING, "meshId", NULL, NULL, "main"},
    {JSON_STRING, "centreFreq", NULL, NULL, "main"},
    {JSON_STRING, "chanBandwidth", NULL, &io_chanBW, "main"},
    {JSON_STRING, "TX1Power", NULL, NULL, "main"},
    {JSON_STRING, "TX2Power", NULL, NULL, "main"},
    {JSON_STRING, "reduceMimo", NULL, NULL, "main"},
    {JSON_STRING, "ipAddress", NULL, &io_ipAddress, "main"},
    {JSON_STRING, "ipMask", NULL, &io_ipMask, "main"},
    {JSON_STRING, "ipGateway", NULL, &io_ipGateway, "main"},
    {JSON_STRING, "audioEnable", NULL, NULL, "audio"},
    {JSON_STRING, "audioHeadGain", NULL, NULL, "audio"},
    {JSON_STRING, "audioMicGain", NULL, NULL, "audio"},
    {JSON_STRING, "audioMuteLevel", NULL, NULL, "audio"},
    {JSON_STRING, "data0BaudRate", NULL, NULL, "dataPort"},
    {JSON_STRING, "data0Parity", NULL, NULL, "dataPort"},
    {JSON_STRING, "data0StopBits", NULL, NULL, "dataPort"},
    {JSON_STRING, "data0FlowControl", NULL, NULL, "dataPort"},
    {JSON_STRING, "data0Width", NULL, NULL, "dataPort"},
};
int config_cnt = sizeof(config_t)/sizeof(sdata_s);

//for ip status
U64 now_txbytes, now_txpackets, now_txerrors; 
U64 pre_txbytes, pre_txpackets, pre_txerrors; 
U64 now_rxbytes, now_rxpackets, now_rxerrors; 
U64 pre_rxbytes, pre_rxpackets, pre_rxerrors; 

U64 txbytes, txpackets, txerrors; 
U64 rxbytes, rxpackets, rxerrors; 

struct timeval pre_tv = {0,0};
struct timeval now_tv = {0,0};

int main(int argc, char *argv[])
{
    sigset_t t_set;
    pthread_t tm_tid, rcv_tid;

    if(argc < 2){
        fprintf(stderr, "%s:error! need a node address\n", argv[0]);
        goto main_exit;
    }

    sscanf(argv[1], "%u", (U32*)&sa);
    printf("%s:node address is %d\n", argv[0], sa);

    print_info(argv[0]);

    if(dc_init()){
        goto main_exit;
    }

    init_tree();

    config_device();

    gen_json(STATUS_PATH, status_root);
    gen_json(CONFIG_PATH, config_root);

    sigemptyset(&t_set);
    sigaddset(&t_set, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &t_set, NULL);

    timer_add("check online", 1, chk_online, 0);
    timer_add("check config", 1, chk_config, 0);
    timer_add("check reset", 1, chk_reset, 0);
    timer_add("sub timeout", 2, sub_timeout, 0);
    timer_add("ip status", 1, ip_status, 0);

    pthread_create(&tm_tid, NULL, timer_thread, &t_set);
    pthread_create(&rcv_tid, NULL, rcv_thread, NULL);

    while(1){
        sleep(10);
    }

main_exit:
    return 0;
}

void print_info(char *arg)
{
#if ON_BOARD
    fprintf(stderr, "%s: on board test\n", arg);
#else
    fprintf(stderr, "%s: ubuntu test\n", arg);
#endif

    return ;
}

int init_tree()
{
    int i, j, cnt;
    char init_flag = 0;
    char buf[1024], temp[32];

    //initialize read-only data
    update_rdonly();

    //initialize config
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
        fprintf(stderr, "init.json is broken, try to restore it\n");
        init = read_json(DEFAULT_PATH, NULL);
        if(init == NULL){
            fprintf(stderr, "default.json is broken! restore failed\n");
            goto func_exit;
        }
        update_config(init);
        free_rdata(init);
        init_flag = 1;
    }

    node_s *pnode;
    for(i = 0; i < config_cnt; i++){
        pnode = search_node(config_root, config_t[i].fname);
        //printf("i=%d, pnode = %p\n", i, pnode);
        insert_node(search_node(config_root, config_t[i].fname), create_node(JSON_STRING, config_t[i].name, config_t[i].pvalue));
    }

    //restore init.json
    if(init_flag == 1){
        gen_json(INIT_PATH, config_root);
        fprintf(stderr, "restore init.json success\n");
    }

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

    //initialize remote status
    insert_node(sstatus, remote);
    for(i = 0; i < MAX_NODE_CNT; i++){
        snode_data[i] = create_node(JSON_BRACKET, NULL, NULL);
        insert_node(remote, snode_data[i]);
        insert_node(snode_data[i], create_node(JSON_STRING, "timeout", "0"));
    }
    mod_node(snode_data[sa-1]->child_h.next->pnode, "15");
    update_status();

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
    update_info_t();
    cnt = sizeof(info_t)/sizeof(data_s);
    for(i = 0; i < cnt; i++){
        insert_node(sinformation, create_node(JSON_STRING, info_t[i].name, info_t[i].pvalue));
    }

    update_dvlp_t();
    cnt = sizeof(dvlp_t)/sizeof(data_s);
    for(i = 0; i < cnt; i++){
        insert_node(sdeveloper, create_node(JSON_STRING, dvlp_t[i].name, dvlp_t[i].pvalue));
    }

func_exit:
    return 0;
}

int config_device()
{
    int rval = 0;
    int i, times;

    for(i = 0; i < config_cnt; i++){
        if(config_t[i].pfunc == NULL){
            continue;
        }else{
            //printf("%s: configuring %s, value = %s\n", __func__, config_t[i].name, config_t[i].pvalue);
            times = 0;
            while(1){
                rval = (*config_t[i].pfunc)(i, 1, config_t[i].pvalue);
                if(rval){
                    fprintf(stderr, "%s configured failed, try again, times = %d\n", config_t[i].name, times++);
                    if(times > 2){
                        fprintf(stderr, "%s configured failed, times = %d\n", __func__, times);
                        break;
                    }
                    usleep(500000);
                }else{
                    break;
                }
            }
        }
    }

    return rval;
}

void update_rdonly()
{
    int i, j;
    rdata_s *rdata = NULL;
    rdata_s *pd = NULL;

    rdata = read_json(INFO_PATH, NULL);
    if(rdata == NULL){
        fprintf(stderr, "%s: read json failed\n", __func__);
        goto func_exit;
    }
    pd = rdata;

    while(pd != NULL){
        for(i = 0; i < rdonly_cnt; i++){
            if(strcmp(pd->name, rdonly_data[i].name) == 0){
                modify_value(&rdonly_data[i].pvalue, pd->pvalue);

                if(strcmp(pd->name, CNAME_SERIAL) == 0){
                    for(j = 0; j < status_cnt; j++){
                        if(strcmp(status_data[j].name, CNAME_SERIAL) == 0){
                            modify_value(&status_data[j].pvalue, pd->pvalue);
                            break;
                        }
                    }
                }else if(strcmp(pd->name, CNAME_BOARDTYPE) == 0){
                    for(j = 0; j < status_cnt; j++){
                        if(strcmp(info_t[j].name, CNAME_SERIAL) == 0){
                            modify_value(&info_t[j].pvalue, pd->pvalue);
                            //printf("%s, %d\n", __func__, __LINE__);
                            break;
                        }
                    }
                }

                break;
            }
        }
        pd = pd->next;
    }


func_exit:
    if(rdata != NULL){
        free_rdata(rdata);
        rdata = NULL;
    }
    return ;
}

void update_sig()
{
    memset(sigQuality_t, 255, sizeof(sigQuality_t));

    return ;
}

void update_info_t()
{
    /*
    char buf[64];
    int i, cnt;

    cnt = sizeof(info_t)/sizeof(data_s);
    strcpy(buf, "1.1");
    for(i = 0; i < cnt; i++){
        if(info_t[i].pvalue == NULL){
            info_t[i].pvalue = (char*)malloc(strlen(buf)+1);
            strcpy(info_t[i].pvalue, buf);
        }else{
        }
    }
    */

    return ;
}

void update_dvlp_t()
{
    char buf[64];
    int i, cnt;

    cnt = sizeof(dvlp_t)/sizeof(data_s);
    strcpy(buf, "11");
    for(i = 0; i < cnt; i++){
        if(dvlp_t[i].pvalue == NULL){
            dvlp_t[i].pvalue = (char*)malloc(strlen(buf)+1);
            strcpy(dvlp_t[i].pvalue, buf);
        }else{
        }
    }

    return ;
}

/*
 * func:
 *      update config_t[] according to 'init'
 */
void update_config(rdata_s *init)
{
    int i, len;

    while(init != NULL){
        for(i = 0; i < config_cnt; i++){
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

    pthread_mutex_lock(&pmutex);

    if((pd = read_json(STATUS_PATH, "online")) == NULL){
        fprintf(stderr, "status.json is doubted broken\n");
        gen_json(STATUS_PATH, status_root);
        goto func_exit;
    }

    if(0 == (strcmp("1", pd->pvalue))){
        //fprintf(stderr, "%s,%d\n", __func__, __LINE__);
        update_status();
        update_dvlp_t();

        //send request to other node
        send_req();

        gen_json(STATUS_PATH, status_root);
    }

func_exit:
    pthread_mutex_unlock(&pmutex);
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

    pthread_mutex_lock(&pmutex);

    if((pd = read_json(CONFIG_PATH, "config")) == NULL){
        fprintf(stderr, "%s:config.json is doubted broken\n", __func__);
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
    pthread_mutex_unlock(&pmutex);

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

    pthread_mutex_lock(&pmutex);

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
    pthread_mutex_unlock(&pmutex);
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

    pthread_mutex_lock(&pmutex);

    for(i = 0; i < 32; i++){
        //printf("*******************\n");
        if(i == sa-1) continue;
        node = search_node(snode_data[i], "timeout");
        //printf("*******************\n");
        if(node != NULL){
            value = atoi(node->pvalue);
            //printf("--> timeout[%d] = %d, timeout_flag[%d] = %d\n", i, value, i, timeout_flag[i]);
            if(value > 0 && (i != (sa-1))){
                if(value == MAX_TIMEOUT){
                    if(timeout_flag[i] == 0){
                        timeout_flag[i] = 1;
                        continue;
                    }else{
                        timeout_flag[i] = 0;
                    }
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

    pthread_mutex_unlock(&pmutex);
    return ;
}

void ip_status(U32 arg)
{
    char line[1024];
    FILE *fp = NULL;
    int len, i;
    char *pos, *p;

    pthread_mutex_lock(&pmutex);

    fp = fopen(NET_PATH, "r");
    if(fp == NULL){
        fprintf(stderr, NET_PATH "is not exist\n");
        goto func_exit;
    }

    memset(line, 0, 1024);
    len = fread(line, 1, 1024, fp);
    if(len <= 0){
        fprintf(stderr, "%s:read failed\n", __func__);
        goto func_exit;
    }

    if((pos = strstr(line, NET_DEV_NAME)) == NULL){
        fprintf(stderr, "%s:can not find " NET_DEV_NAME "\n", __func__);
        goto func_exit;
    }

    pre_rxbytes = now_rxbytes;
    pre_rxpackets = now_rxpackets;
    pre_rxerrors = now_rxerrors;
    pre_txbytes = now_txbytes;
    pre_txpackets = now_txpackets;
    pre_txerrors = now_txerrors;

    pre_tv = now_tv;
    gettimeofday(&now_tv, NULL);

    for(i = 0, p = strtok(pos, " \n\t\r"); i < 12; p = strtok(NULL, " \n\t\r"), i++){
        switch(i){
            case 1:
                sscanf(p, "%lld", &now_rxbytes);
                //printf("%d:%lld\n", i, now_rxbytes);
                break;
            case 2:
                sscanf(p, "%lld", &now_rxpackets);
                //printf("%d:%lld\n", i, now_rxpackets);
                break;
            case 3:
                sscanf(p, "%lld", &now_rxerrors);
                //printf("%d:%lld\n", i, now_rxerrors);
                break;
            case 9:
                sscanf(p, "%lld", &now_txbytes);
                //printf("%d:%lld\n", i, now_txbytes);
                break;
            case 10:
                sscanf(p, "%lld", &now_txpackets);
                //printf("%d:%lld\n", i, now_txpackets);
                break;
            case 11:
                sscanf(p, "%lld", &now_txerrors);
                //printf("%d:%lld\n", i, now_txerrors);
                break;
            default:
                break;
        }
    }

    if((pre_tv.tv_sec + pre_tv.tv_usec) != 0){
        rxbytes = now_rxbytes - pre_rxbytes;
        rxpackets = now_rxpackets - pre_rxpackets;
        rxerrors = now_rxerrors - pre_rxerrors;
        txbytes = now_txbytes - pre_txbytes;
        txpackets = now_txpackets - pre_txpackets;
        txerrors = now_txerrors - pre_txerrors;
    }

    //printf("now_rx: %lld, %lld, %lld\n", now_rxbytes, now_rxpackets, now_rxerrors);
    //printf("pre_rx: %lld, %lld, %lld\n", pre_rxbytes, pre_rxpackets, pre_rxerrors);
    //printf("tx: %lld, %lld, %lld\n", txbytes, txpackets, txerrors);
    //printf("rx: %lld, %lld, %lld\n\n", rxbytes, rxpackets, rxerrors);
    //printf("now_tv: %ld %ld\n", now_tv.tv_sec, now_tv.tv_usec);
    //printf("pre_tv: %ld %ld\n\n", pre_tv.tv_sec, pre_tv.tv_usec);

func_exit:
    if(fp != NULL)
        fclose(fp);

    pthread_mutex_unlock(&pmutex);

    return ;
}

/*
 * func:
 *      compare config information between config.json and local variable, call pfunc if defference
 * ret:
 *      numbers of config successed parameters
 */
int cmp_config(rdata_s *rdata)
{
    int flag = 0;
    int i, len;
    node_s *node;
    int rval = 0;

    while(rdata != NULL){
        for(i = 0; i < config_cnt; i++){
            if(strcmp(rdata->name, config_t[i].name) == 0){
                if(config_t[i].pvalue == NULL){
                    //maybe need more code
                }
                if(strcmp(rdata->pvalue, config_t[i].pvalue) != 0){
                    if(config_t[i].pfunc != NULL){
                        rval = (*config_t[i].pfunc)(i, 1, rdata->pvalue);

                        //if failed
                        if(rval != 0){
                            fprintf(stderr, "config %s failed\n", rdata->name);
                            break;
                        }

                        //if success
                        flag+=1;
                        //update config_t
#if 0
                        len = strlen(rdata->pvalue);
                        if(len > (int)strlen(config_t[i].pvalue)){
                            free(config_t[i].pvalue);
                            config_t[i].pvalue = (char*)malloc(len + 1);
                        }
                        strcpy(config_t[i].pvalue, rdata->pvalue);
#endif
                        modify_value(&config_t[i].pvalue, rdata->pvalue);

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
    int i;

    //do pfunc to update status_data
    for(i = 0; i < status_cnt; i++){
        //fprintf(stderr, "%s,%d\n", __func__, __LINE__);
        if(status_data[i].pfunc == NULL) continue;
        else{
            (*status_data[i].pfunc)(i, 0, NULL);
        }
    }

    pn = snode_data[sa-1];
    remove_status(sa-1);
    for(i = 0; i < status_cnt; i++){
        stat2tree(pn, &status_data[i]);
    }

    return ;
}

/*
 * func:
 *      remove all status infomation without 'timeout'
 */
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
    snode_data[i]->child_t = snode_data[i]->child_h;

    return ;
}

void *rcv_thread(void *arg)
{
    int rval;
    int reqfd, infofd;
    struct ip_mreq mreq;
    struct sockaddr_in mserv, userv;
    struct sockaddr_in cli;
    socklen_t sock_len;
    mmsg_t *pm = NULL;
    fd_set rset, std_rset;

    pthread_detach(pthread_self());

    //create request socket
    reqfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(reqfd < 0){
        perror("create request socket failed");
        rval = 1;
        goto thread_exit;
    }
    //create information socket
    infofd = socket(AF_INET, SOCK_DGRAM, 0);
    if(infofd < 0){
        perror("create information socket failed");
        rval = 1;
        goto thread_exit;
    }

    //join multicast group
    if(inet_pton(AF_INET, GROUP_IP, &mreq.imr_multiaddr.s_addr) <= 0){
        printf("Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }
    if(inet_pton(AF_INET, NODE_IP, &mreq.imr_interface.s_addr) <= 0){
        printf("Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }
    //mreq.imr_interface.s_addr = INADDR_ANY;
    setsockopt(reqfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));

    //fill socket address variable
    mserv.sin_family = AF_INET;
    mserv.sin_port = htons(PORT);
    if(inet_pton(AF_INET, GROUP_IP, &mserv.sin_addr) <= 0){
        printf("Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }

    userv.sin_family = AF_INET;
    userv.sin_port = htons(PORT);
    if(inet_pton(AF_INET, NODE_IP, &userv.sin_addr) <= 0){
        printf("Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }

    //bind socket
    if(bind(reqfd, (struct sockaddr*)&mserv, sizeof(struct sockaddr)) < 0){
        perror("bind faliled");
        rval = 3;
        goto thread_exit;
    }
    if(bind(infofd, (struct sockaddr*)&userv, sizeof(struct sockaddr)) < 0){
        perror("bind faliled");
        rval = 3;
        goto thread_exit;
    }

    sock_len = sizeof(struct sockaddr);
    pm = (mmsg_t*)malloc(MMSG_LEN);

    FD_ZERO(&std_rset);
    FD_SET(reqfd, &std_rset);
    FD_SET(infofd, &std_rset);

    while(1){
        memcpy(&rset, &std_rset, sizeof(fd_set));

        if(select(MMAX(infofd, reqfd)+1, &rset, NULL, NULL, NULL) < 0){
            perror("select");
            continue;
        }

        if(FD_ISSET(reqfd, &rset)){
            recvfrom(reqfd, pm, MMSG_LEN, 0, (struct sockaddr*)&cli, &sock_len);
            if(pm->type == MMSG_REQ){
                //if(pm->node == sa) continue;

                printf("recv a requst msg from node %d\n", pm->node);
                pthread_mutex_lock(&pmutex);
                update_status();
                gen_json(STATUS_PATH, status_root);
                rval = send_info(reqfd, &cli);
                if(rval != 0){
                    fprintf(stderr, "send info failed\n");
                }
                pthread_mutex_unlock(&pmutex);
            }
        }
        if(FD_ISSET(infofd, &rset)){
            recvfrom(infofd, pm, MMSG_LEN, 0, (struct sockaddr*)&cli, &sock_len);
            if(pm->type == MMSG_INFO){
                pthread_mutex_lock(&pmutex);
                printf("recv an info msg from node %d\n", pm->node);
                //printf("msg:[%s]\n", pm->buf);

                rval = update_node(pm->node, pm->buf);
                if(rval){
                    update_time(pm->node, 0);
                }else{
                    update_time(pm->node, 15);
                }
                gen_json(STATUS_PATH, status_root);
                pthread_mutex_unlock(&pmutex);
            }
        }
    }

thread_exit:
    if(pm != NULL) free(pm);
    pthread_exit((void*)&rval);
}

/*
 * func:
 *      initialize DeviceConfig
 * ret:
 *      0:          success
 *      1:          failure
 */
int dc_init()
{
    int rval = 0;

    mc_fd = socket(AF_INET, SOCK_DGRAM, 0);
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, GROUP_IP, &mc_addr.sin_addr) <= 0){
        printf("Wrong IP address\n");
        rval = 1;
        goto func_exit;
    }

func_exit:
    return rval;
}

void send_req()
{
    mmsg_t msg;

    msg.type = MMSG_REQ;
    msg.node = sa;
    sendto(mc_fd, &msg, MMSG_LEN-sizeof(msg.buf), 0, (struct sockaddr*)&mc_addr, sizeof(struct sockaddr));

    return ;
}

int send_info(int reqfd, void *cli)
{
    mmsg_t msg;
    int len = 0, llen = 0, i;
    int rval = 0;
    char buf[256];

    ((struct sockaddr_in*)cli)->sin_port = htons(PORT);

    msg.node = sa+1;
    len += sizeof(msg.node);
    msg.type = MMSG_INFO;
    len += sizeof(msg.type);

    msg.buf[0] = 0;
    for(i = 0; i < status_cnt; i++){
        switch(status_data[i].type){
            case JSON_NORMAL:
            case JSON_ARRAY:
                sprintf(buf, "%s|", status_data[i].name);
                llen += strlen(buf);
                if(llen >= MAX_MSG_LEN){
                    rval = 1;
                    fprintf(stderr, "msg is too long\n");
                    goto func_exit;
                }
                strcat(msg.buf, buf);
                break;
            case JSON_STRING:
                if(status_data[i].pvalue == NULL) break;
                sprintf(buf, "%s|", status_data[i].name);
                llen += strlen(buf);
                if(llen >= MAX_MSG_LEN){
                    rval = 1;
                    fprintf(stderr, "msg is too long\n");
                    goto func_exit;
                }
                strcat(msg.buf, buf);
                sprintf(buf, "%s|", status_data[i].pvalue);
                llen += strlen(buf);
                if(llen >= MAX_MSG_LEN){
                    rval = 1;
                    fprintf(stderr, "msg is too long\n");
                    goto func_exit;
                }
                strcat(msg.buf, buf);
                break;
            default:
                break;
        }
    }
    len += strlen(msg.buf)+1;

    //printf("node = %-2d, type = %-2d, msg = [%s]\n", msg.node, msg.type, msg.buf);
    //sendto(reqfd, &msg, MMSG_LEN-sizeof(msg.buf), 0, (struct sockaddr*)cli, sizeof(struct sockaddr));
    sendto(reqfd, &msg, len, 0, (struct sockaddr*)cli, sizeof(struct sockaddr));

func_exit:
    return rval;
}

void update_time(int addr, int value)
{
    node_s *node;
    char buf[16];

    node = search_node(snode_data[addr-1], "timeout");
    sprintf(buf, "%d", value);
    mod_node(node, buf);
}

int update_node(int addr, char *pmsg)
{
    char *str1;
    int i, rval = 0;
    sdata_s mould;

    //remove old status
    remove_status(addr-1);

    str1 = strtok(pmsg, "|");
    if(str1 != NULL){
        for(i = 0; i < status_cnt; i++){
            if(strcmp(str1, status_data[i].name) == 0){
                mould = status_data[i];
                if(mould.type == JSON_NORMAL || mould.type == JSON_ARRAY){
                    mould.pvalue = NULL;
                }else if(mould.type == JSON_STRING){
                    mould.pvalue = strtok(NULL, "|");
                    if(mould.pvalue == NULL){
                        rval = 1;
                        goto func_exit;
                    } 
                }
                stat2tree(snode_data[addr-1], &mould);
                break;
            }
        }
    }
    while((str1 = strtok(NULL, "|")) != NULL){
        for(i = 0; i < status_cnt; i++){
            if(strcmp(str1, status_data[i].name) == 0){
                mould = status_data[i];
                if(mould.type == JSON_NORMAL || mould.type == JSON_ARRAY){
                    mould.pvalue = NULL;
                }else if(mould.type == JSON_STRING){
                    mould.pvalue = strtok(NULL, "|");
                    if(mould.pvalue == NULL){
                        rval = 1;
                        goto func_exit;
                    } 
                }
                stat2tree(snode_data[addr-1], &mould);
                break;
            }
        }
    }

func_exit:
    if(rval){
        remove_status(addr-1);
    }
    return rval;
}

/*
 * func:
 *      update status information to the tree
 */
int stat2tree(node_s *pn, sdata_s *sdata)
{
    int rval = 0;
    node_s *node, *pfn;

    if(strcmp(sdata->fname, "null") == 0){
        node = create_node(sdata->type, sdata->name, sdata->pvalue);
        if(node == NULL){
            fprintf(stderr, "%s,%d:%s is set wrong\n", __func__, __LINE__, sdata->name);
            rval = 1;
            goto func_exit;
        }else{
            insert_node(pn, node);
        }
    }else{
        pfn = search_node(pn, sdata->fname);
        if(pfn == NULL){
            fprintf(stderr, "%s's fname is set wrong, fname is '%s'\n", sdata->name, sdata->fname);
            rval = 2;
            goto func_exit;
        }
        node = create_node(sdata->type, sdata->name, sdata->pvalue);
        if(node == NULL){
            fprintf(stderr, "%s,%d:%s is set wrong\n", __func__, __LINE__, sdata->name);
            rval = 3;
            goto func_exit;
        }else{
            insert_node(pfn, node);
        }
    }

func_exit:
    return rval;
}













