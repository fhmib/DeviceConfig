#include "dc_common.h"
#include "dc_io.h"

U8 sa;
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;

int mn_qid = -1;
int dc_qid = -1;

//for sending socket
int mc_fd;
struct sockaddr_in mc_addr;

extern void *g_FPGA_pntr;

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

#if 0
//read-only data
//absoleted
odata_s rdonly_data[] = {
    {CNAME_SERIAL, NULL},
    {CNAME_BOARDTYPE, NULL},
};
int rdonly_cnt = sizeof(rdonly_data)/sizeof(rdonly_data[0]);
#endif

//for managing status that obtained from other nodes.
int timeout[MAX_NODE_CNT] = {0};

//local status table
sdata_s status_data[] = {
    {JSON_NORMAL, CNAME_NOTDEF, NULL, NULL, CNAME_NULL, 1},
    {JSON_NORMAL, CNAME_NODEHEADER, NULL, NULL, CNAME_NOTDEF, 1},
    {JSON_STRING, CNAME_NODEID, NULL, &io_nodeId, CNAME_NODEHEADER, 1},
    {JSON_STRING, CNAME_NODENAME, NULL, &io_nodeName, CNAME_NODEHEADER, 1},
    {JSON_STRING, CNAME_SERIAL, NULL, &io_readfrominfo, CNAME_NODEHEADER, 1},
    {JSON_STRING, CNAME_VOLTAGE, NULL, &io_voltage, CNAME_NODEHEADER, 1},
    {JSON_STRING, CNAME_TEMP, NULL, &io_temperature, CNAME_NODEHEADER, 1},
    {JSON_STRING, CNAME_IPADDRESS, NULL, &io_ipAddress, CNAME_NODEHEADER, 1},
    {JSON_NORMAL, CNAME_IPSTATUS, NULL, NULL, CNAME_NOTDEF, 1},
    {JSON_STRING, CNAME_IPTXBYTE, NULL, &io_ipTxByteCnt, CNAME_IPSTATUS, 1},
    {JSON_STRING, CNAME_IPTXPKT, NULL, &io_ipTxPktCnt, CNAME_IPSTATUS, 1},
    {JSON_STRING, CNAME_IPTXERR, NULL, &io_ipTxErrorCnt, CNAME_IPSTATUS, 1},
    {JSON_STRING, CNAME_IPRXBYTE, NULL, &io_ipRxByteCnt, CNAME_IPSTATUS, 1},
    {JSON_STRING, CNAME_IPRXPKT, NULL, &io_ipRxPktCnt, CNAME_IPSTATUS, 1},
    {JSON_STRING, CNAME_IPRXERR, NULL, &io_ipRxErrorCnt, CNAME_IPSTATUS, 1},
};
int status_cnt = sizeof(status_data)/sizeof(status_data[0]);

//information table
sdata_s info_t[] = {
    {JSON_STRING, CNAME_SOFTVER, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_PROVER, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_FPGAVER, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_SERIAL, NULL, &io_readInfo, CNAME_NULL, 1},
    {JSON_STRING, CNAME_BOARDTYPE, NULL, &io_readInfo, CNAME_NULL, 1},
    {JSON_STRING, CNAME_MACADDR, NULL, &io_macAddr, CNAME_NULL, 1},
};
int info_cnt = sizeof(info_t)/sizeof(info_t[0]);

//developer table
sdata_s dvlp_t[] = {
    {JSON_STRING, CNAME_TBS, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_UPS, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_CLOCK, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_NETSTATUS, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_SLOTCNT, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_L2BNUM, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_B2LNUM, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_BBSFNUM, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_VMODE, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_BSTABLE, NULL, NULL, CNAME_NULL, 0},
    {JSON_STRING, CNAME_DSTABLE, NULL, NULL, CNAME_NULL, 0},
};
int dvlp_cnt = sizeof(dvlp_t)/sizeof(dvlp_t[0]);

unsigned char sigQuality_t[MAX_NODE_CNT][MAX_NODE_CNT];
node_s *sigs[MAX_NODE_CNT];

//for config.json
node_s *config_root;
// node_s *cflags;
// node_s *csettings;
// node_s *cmain;
// node_s *caudio;
// node_s *cdataport;
// node_s *croute;

//config table
sdata_s config_t[] = {
    {JSON_NORMAL, CNAME_FLAGS, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_CONFIG, NULL, NULL, CNAME_FLAGS, 1},
    {JSON_STRING, CNAME_RESET, NULL, NULL, CNAME_FLAGS, 1},
    {JSON_NORMAL, CNAME_MAIN, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_NODEID, NULL, &io_nodeId, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_NODENAME, NULL, &io_nodeName, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_MESHID, NULL, NULL, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_FREQ, NULL, NULL, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_CHANBW, NULL, &io_chanBW, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_TFCI, NULL, &io_tfci, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_TXPOWER, NULL, &io_txPower, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_MIMO, NULL, NULL, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_IPADDRESS, NULL, &io_ipAddress, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_IPMASK, NULL, &io_ipMask, CNAME_MAIN, 1},
    {JSON_STRING, CNAME_IPGATEWAY, NULL, &io_ipGateway, CNAME_MAIN, 1},
    {JSON_NORMAL, CNAME_AUDIO, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_AUDIOENABLE, NULL, &io_audioEnable, CNAME_AUDIO, 1},
    {JSON_STRING, CNAME_AUDIOPLAY, NULL, &io_audioVol, CNAME_AUDIO, 1},
    {JSON_STRING, CNAME_AUDIOMIC, NULL, &io_audioVol, CNAME_AUDIO, 1},
    {JSON_STRING, CNAME_AUDIOALC, NULL, &io_audioVol, CNAME_AUDIO, 1},
    {JSON_NORMAL, CNAME_DATAPORT, NULL, NULL, CNAME_NULL, 1},
    {JSON_STRING, CNAME_UART0RATE, NULL, &io_dataRate, CNAME_DATAPORT, 1},
    {JSON_STRING, CNAME_UART0PARITY, NULL, &io_dataParity, CNAME_DATAPORT, 1},
    //{JSON_STRING, "data0StopBits", NULL, NULL, CNAME_DATAPORT, 1},
    //{JSON_STRING, "data0FlowControl", NULL, NULL, CNAME_DATAPORT, 1},
    //{JSON_STRING, "data0Width", NULL, NULL, CNAME_DATAPORT, 1},
    {JSON_NORMAL, CNAME_ROUTE, NULL, NULL, CNAME_NULL, 1},
    {JSON_NORMAL, CNAME_ROUTE0, NULL, NULL, CNAME_ROUTE, 1},
    {JSON_STRING, CNAME_ROUTE0ADDR, NULL, &io_route, CNAME_ROUTE0, 1},
    {JSON_STRING, CNAME_ROUTE0MASK, NULL, &io_route, CNAME_ROUTE0, 1},
    {JSON_STRING, CNAME_ROUTE0GATE, NULL, &io_route, CNAME_ROUTE0, 1},
    {JSON_NORMAL, CNAME_ROUTE1, NULL, NULL, CNAME_ROUTE, 1},
    {JSON_STRING, CNAME_ROUTE1ADDR, NULL, &io_route, CNAME_ROUTE1, 1},
    {JSON_STRING, CNAME_ROUTE1MASK, NULL, &io_route, CNAME_ROUTE1, 1},
    {JSON_STRING, CNAME_ROUTE1GATE, NULL, &io_route, CNAME_ROUTE1, 1},
    {JSON_NORMAL, CNAME_ROUTE2, NULL, NULL, CNAME_ROUTE, 1},
    {JSON_STRING, CNAME_ROUTE2ADDR, NULL, &io_route, CNAME_ROUTE2, 1},
    {JSON_STRING, CNAME_ROUTE2MASK, NULL, &io_route, CNAME_ROUTE2, 1},
    {JSON_STRING, CNAME_ROUTE2GATE, NULL, &io_route, CNAME_ROUTE2, 1},
    {JSON_NORMAL, CNAME_ROUTE3, NULL, NULL, CNAME_ROUTE, 1},
    {JSON_STRING, CNAME_ROUTE3ADDR, NULL, &io_route, CNAME_ROUTE3, 1},
    {JSON_STRING, CNAME_ROUTE3MASK, NULL, &io_route, CNAME_ROUTE3, 1},
    {JSON_STRING, CNAME_ROUTE3GATE, NULL, &io_route, CNAME_ROUTE3, 1},
};
int config_cnt = sizeof(config_t)/sizeof(config_t[0]);

//for ip status
U64 now_txbytes, now_txpackets, now_txerrors; 
U64 pre_txbytes, pre_txpackets, pre_txerrors; 
U64 now_rxbytes, now_rxpackets, now_rxerrors; 
U64 pre_rxbytes, pre_rxpackets, pre_rxerrors; 
U64 txbytes, txpackets, txerrors; 
U64 rxbytes, rxpackets, rxerrors; 
struct timeval pre_tv = {0,0};
struct timeval now_tv = {0,0};

//for config serial port
char port_flag[PORT_CNT] = {0};
//for config route
char route_flag[ROUTE_CNT] = {0};

int main(int argc, char *argv[])
{
    sigset_t t_set;
    pthread_t tm_tid, rcv_tid;

#if 0
    if(argc < 2){
        fprintf(stderr, "%s:error! need a node address\n", argv[0]);
        goto main_exit;
    }

    sscanf(argv[1], "%u", (U32*)&sa);
    fprintf(stderr, "%s:node address is %d\n", argv[0], sa);
#endif

    print_info(argv[0]);

    if(dc_init()){
        goto main_exit;
    }

    init_tree();

#if 0
    int i;
    for(i = 0; i < config_cnt; i++){
        if(config_t[i].pvalue != NULL)
            printf("%s:%s\n", config_t[i].name, config_t[i].pvalue);
    }
#endif

    init_device();
    usleep(100000);
    update_local_status();

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
    int i, j;
    char init_flag = 0;
    char buf[1024], temp[32];

#if 0
    //initialize read-only data
    update_rdonly();
#endif

    //initialize config
    config_root = create_node(JSON_BRACKET, NULL, NULL, 1);
    // cflags = create_node(JSON_NORMAL, "flags", NULL);
    // csettings = create_node(JSON_NORMAL, "settings", NULL);
    // cmain = create_node(JSON_NORMAL, CNAME_MAIN, NULL);
    // caudio = create_node(JSON_NORMAL, CNAME_AUDIO, NULL);
    // cdataport = create_node(JSON_NORMAL, CNAME_DATAPORT, NULL);
    // croute = create_node(JSON_NORMAL, CNAME_ROUTE, NULL);

    // insert_node(config_root, cflags);
    // insert_node(cflags, create_node(JSON_STRING, "config", "0"));
    // insert_node(cflags, create_node(JSON_STRING, "reset", "0"));
    // insert_node(config_root, csettings);
    // insert_node(csettings, cmain);
    // insert_node(csettings, caudio);
    // insert_node(csettings, cdataport);
    // insert_node(csettings, croute);

    rdata_s *init;
    init = read_json(INIT_PATH, NULL);
    if(init != NULL){
        update_config(init);
        free_rdata(init);
    }else{
        fprintf(stderr, "init.json is broken, try to restore it\n");
        init = read_json(DEFAULT_PATH, NULL);
        if(init == NULL){
            fprintf(stderr, "default.json is broken! restore failed\n");
            exit(1);
        }
        update_config(init);
        free_rdata(init);
        init_flag = 1;
    }

    //read node id from config_t[]
    for(i = 0; i < config_cnt; i++){
        if(0 == strcmp(config_t[i].name, CNAME_NODEID)){
            sscanf(config_t[i].pvalue, "%u", (int*)&sa);
            if(sa > MAX_NODE_CNT){
                fprintf(stderr, "%s: nodeId is invalid\n", __func__);
                exit(1);
            }
            fprintf(stderr, "%s: sa = %d\n", __func__, sa);
        }
    }

    // fprintf(stderr, "%s%d\n", __func__, __LINE__);
    gen_tree(config_root, &config_t[0], config_cnt);
    // node_s *pnode;
    // for(i = 0; i < config_cnt; i++){
    //     pnode = search_node(config_root, config_t[i].fname);
    //     //printf("i=%d, pnode = %p\n", i, pnode);
    //     insert_node(search_node(config_root, config_t[i].fname), create_node(JSON_STRING, config_t[i].name, config_t[i].pvalue));
    // }

    //restore init.json
    if(init_flag == 1){
        gen_json(INIT_PATH, config_root);
        fprintf(stderr, "restore init.json success\n");
    }

    // node_s *remote = create_node(JSON_ARRAY, "remoteStatus", NULL);

    status_root = create_node(JSON_BRACKET, NULL, NULL, 1);
    sflags = create_node(JSON_NORMAL, CNAME_FLAGS, NULL, 1);
    sstatus = create_node(JSON_NORMAL, CNAME_STATUS, NULL, 1);
    ssigtable = create_node(JSON_ARRAY, CNAME_SIGTABLE, NULL, 1);
    sinformation = create_node(JSON_NORMAL, CNAME_INFO, NULL, 1);
    sdeveloper = create_node(JSON_NORMAL, CNAME_DVLP, NULL, 1);

    insert_node(status_root, sflags);
    insert_node(status_root, sstatus);
    insert_node(status_root, ssigtable);
    insert_node(status_root, sinformation);
    insert_node(status_root, sdeveloper);

    //initialize flags
    insert_node(sflags, create_node(JSON_STRING, CNAME_OL, "0", 1));

    //initialize remote status
    timeout[sa-1] = MAX_TIMEOUT;
    for(i = 0; i < status_cnt; i++){
        if(strcmp(status_data[i].name, CNAME_NOTDEF) == 0){
            sprintf(status_data[i].name, CNAME_NODE "%u", (U32)sa&0x000000FF);
        }
        if(strcmp(status_data[i].fname, CNAME_NOTDEF) == 0){
            sprintf(status_data[i].fname, CNAME_NODE "%u", (U32)sa&0x000000FF);
        }
    }
    // insert_node(sstatus, remote);
    // for(i = 0; i < MAX_NODE_CNT; i++){
    //     snode_data[i] = create_node(JSON_BRACKET, NULL, NULL);
    //     insert_node(remote, snode_data[i]);
    //     insert_node(snode_data[i], create_node(JSON_STRING, "timeout", "0"));
    // }
    // mod_node(snode_data[sa-1]->child_h.next->pnode, "15");

    //initialize signal quality table
    for(i = 0; i < MAX_NODE_CNT; i++){
        sigs[i] = create_node(JSON_CUSTOM1, NULL, NULL, 0);
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
    // for(i = 0; i < info_cnt; i++){
    //     insert_node(sinformation, create_node(JSON_STRING, info_t[i].name, info_t[i].pvalue));
    // }
    gen_tree(sinformation, &info_t[0], info_cnt);

#if ON_BOARD
    update_dvlp();
#endif
    // for(i = 0; i < dvlp_cnt; i++){
    //     insert_node(sdeveloper, create_node(JSON_STRING, dvlp_t[i].name, dvlp_t[i].pvalue));
    // }
    gen_tree(sdeveloper, &dvlp_t[0], dvlp_cnt);

func_exit:
    return 0;
}

int init_device()
{
    int rval = 0;
    int i, times;

#if ON_BOARD
    int fd;
    rval = drvFPGA_Init(&fd);
    if(rval){
        fprintf(stderr, "%s:initialize drvFPGA failed\n", __func__);
        rval = 1;
        goto func_exit;
    }
#endif

    for(i = 0; i < config_cnt; i++){
        if(config_t[i].pfunc == NULL){
            continue;
        }else{
            //printf("%s: configuring %s, value = %s\n", __func__, config_t[i].name, config_t[i].pvalue);
            times = 0;
            while(1){
                if(config_t[i].pvalue == NULL){
                    fprintf(stderr, "%s configured failed, pvalue is NULL\n", config_t[i].name);
                    break;
                }
                rval = (*config_t[i].pfunc)(i, 1, config_t[i].pvalue);
                if(rval){
                    fprintf(stderr, "%s configured failed, try again, rval = %d, times = %d\n", config_t[i].name, rval, times++);
                    if(times > 2){
                        fprintf(stderr, "%s configured failed, rval = %d, times = %d\n", __func__, rval, times);
                        break;
                    }
                    usleep(500000);
                }else{
                    usleep(100000);
                    break;
                }
            }
        }
    }

    add_config();

func_exit:
#if ON_BOARD
    if(g_FPGA_pntr != NULL){
        drvFPGA_Close(&fd);
    }
#endif

    return 0;
}

#if 0
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
#endif

void update_sig()
{
    memset(sigQuality_t, 255, sizeof(sigQuality_t));

    return ;
}

void update_info()
{
    char buf[64];
    int i;

    for(i = 0; i < info_cnt; i++){
        if(info_t[i].pfunc == NULL) continue;

        (*info_t[i].pfunc)(i, 0, NULL);
    }

    return ;
}

/*
 * func:
 *      update dvlp_t through receiving msg form mn
 */
void update_dvlp()
{
    mmsg_t msg;
    mmsg_t rmsg;
    mnhd_t *mnhd;
    int len = 0;
    int i, j;
    char buf[1024], temp[16];
    mac_state *hm_state;

    msg.mtype = MMSG_MN_GUIIN;
    msg.node = 5;
    len += sizeof(msg.node);

    mnhd = (mnhd_t*)msg.data;
    mnhd->type = MN_REQ_MAC_STATE;
    len += MNHD_LEN;

    msgsnd(mn_qid, &msg, len, 0);

    i = 3;
    while(i--){
        if(-1 == msgrcv(dc_qid, &rmsg, MAX_MSG_LEN, MMSG_MN_GUIOUT, IPC_NOWAIT)){
            if((i < 2) && (i >= 0)){
                perror("update_dvlp:msgrcv hm_state failed, try again\n");
            }
            usleep(500000);
        }else{
            break;
        }
    }

    //recv failed
    if(i < 0){
        fprintf(stderr, "%s:error! msgrcv hm_state failed\n", __func__);
        goto func_exit;
    }

    mnhd = (mnhd_t*)rmsg.data;
    if(mnhd->type != MN_REP_MAC){
        fprintf(stderr, "%s:receive wrong type, type=%ld\n", __func__, mnhd->type);
        goto func_exit;
    }

    hm_state = (mac_state*)(rmsg.data + MNHD_LEN);

#if 0
    printf("%s:rfnt:%u\n", __func__, hm_state->rfnt);
    printf("%s:net_state:%u\n", __func__, hm_state->sta);
    printf("%s:tbs:%u\n", __func__, hm_state->tbs);
    printf("%s:ups:%u\n", __func__, hm_state->ups);
    printf("%s:clk_level:%u\n", __func__, hm_state->clks);
    printf("%s:delay:%u\n", __func__, hm_state->delay);
    printf("%s:occupy_slot_number:%u\n", __func__, hm_state->osn);
    printf("%s:Low2BB:%u\n", __func__, hm_state->l2bnum);
    printf("%s:BB2Low:%u\n", __func__, hm_state->b2lnum);
    printf("%s:BB2Low_SF:%u\n", __func__, hm_state->sfb2l);
    printf("%s:vmode:%u\n", __func__, hm_state->vmode);
#endif

    for(i = 0; i < config_cnt; i++){
        if(strcmp(config_t[i].name, CNAME_MESHID) == 0){
            sprintf(buf, "%u", hm_state->rfnt);
            modify_value(&config_t[i].pvalue, buf);
            break;
        }
    }

    for(i = 0; i < dvlp_cnt; i++){
        if(strcmp(dvlp_t[i].name, CNAME_NETSTATUS) == 0){
            switch(hm_state->sta){
                case 0:
                    strcpy(buf, "INIT");
                    break;
                case 1:
                    strcpy(buf, "SCAN");
                    break;
                case 2:
                    strcpy(buf, "WAN");
                    break;
                case 3:
                    strcpy(buf, "NET");
                    break;
                default:
                    buf[0] = 0;
                    break;
            }
            if(buf[0] == 0){
                modify_value(&dvlp_t[i].pvalue, NULL);
            }else{
                modify_value(&dvlp_t[i].pvalue, buf);
            }
        }else if(strcmp(dvlp_t[i].name, CNAME_TBS) == 0){
            sprintf(buf, "%u", hm_state->tbs);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_UPS) == 0){
            sprintf(buf, "%u", hm_state->ups);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_CLOCK) == 0){
            sprintf(buf, "%u", hm_state->clks);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_SLOTCNT) == 0){
            sprintf(buf, "%u", hm_state->osn);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_L2BNUM) == 0){
            sprintf(buf, "%u", hm_state->l2bnum);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_B2LNUM) == 0){
            sprintf(buf, "%u", hm_state->b2lnum);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_BBSFNUM) == 0){
            sprintf(buf, "%u", hm_state->sfb2l);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_VMODE) == 0){
            sprintf(buf, "%u", hm_state->vmode);
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_BSTABLE) == 0){
            for(j = 0; j < 32; j++){
                if(j == 0){
                    sprintf(temp, "[%u, ", hm_state->bsmap[j]);
                    sprintf(buf, "%s", temp);
                }else if(j < 31){
                    sprintf(temp, "%u, ", hm_state->bsmap[j]);
                    strcat(buf, temp);
                }else{
                    sprintf(temp, "%u]", hm_state->bsmap[j]);
                    strcat(buf, temp);
                }
            }
            modify_value(&dvlp_t[i].pvalue, buf);
        }else if(strcmp(dvlp_t[i].name, CNAME_DSTABLE) == 0){
            for(j = 0; j < 55; j++){
                if(j == 0){
                    sprintf(temp, "[%u, ", hm_state->dsmap[j]);
                    sprintf(buf, "%s", temp);
                }else if(j < 54){
                    sprintf(temp, "%u, ", hm_state->dsmap[j]);
                    strcat(buf, temp);
                }else{
                    sprintf(temp, "%u]", hm_state->dsmap[j]);
                    strcat(buf, temp);
                }
            }
            modify_value(&dvlp_t[i].pvalue, buf);
        }
    }

    remove_childs(sdeveloper);

    for(i = 0; i < dvlp_cnt; i++){
#if 0
        if(dvlp_t[i].pvalue != NULL){
            printf("%s:%s\n", dvlp_t[i].name, dvlp_t[i].pvalue);
        }
#endif
        stat2tree(sdeveloper, &dvlp_t[i]);
    }

func_exit:
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
                // fprintf(stderr, "%s%d: %s,%s\n", __func__, __LINE__, config_t[i].name, init->pvalue);
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

    pthread_mutex_lock(&gts.mutex);

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

    pthread_mutex_unlock(&gts.mutex);

    return ;
}

/*
 * func:
 *      check online flag each 1 second, if it is 1, devcfg will obtain local node status and send other nodes the request.
 */
void chk_online(U32 arg)
{
    rdata_s *pd = NULL;

    // printf("I'm in %s, arg = %d\n", __func__, arg);

    pthread_mutex_lock(&pmutex);

    if((pd = read_json(STATUS_PATH, "online")) == NULL){
        fprintf(stderr, "status.json is doubted broken\n");
        gen_json(STATUS_PATH, status_root);
        goto func_exit;
    }

    if(0 == (strcmp("1", pd->pvalue))){
        // fprintf(stderr, "%s,%d\n", __func__, __LINE__);
        update_local_status();
#if ON_BOARD
        update_dvlp();
#endif

        //send request to other node
        send_req();

        gen_json(STATUS_PATH, status_root);
    }

func_exit:
    pthread_mutex_unlock(&pmutex);
    if(pd != NULL){
        free_rdata(pd);
    }
    // fprintf(stderr, "%s,%d\n", __func__, __LINE__);
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

    // for(i = 0; i < 32; i++){
    //     if(i == sa-1) continue;
    //     node = search_node(snode_data[i], "timeout");
    //     if(node != NULL){
    //         value = atoi(node->pvalue);
    //         //printf("--> timeout[%d] = %d, timeout_flag[%d] = %d\n", i, value, i, timeout_flag[i]);
    //         if(value > 0 && (i != (sa-1))){
    //             if(value == MAX_TIMEOUT){
    //                 if(timeout_flag[i] == 0){
    //                     timeout_flag[i] = 1;
    //                     continue;
    //                 }else{
    //                     timeout_flag[i] = 0;
    //                 }
    //             }
    //             value -= 1;
    //             sprintf(buf, "%d", value);
    //             mod_node(node, buf);
    //             wflag++;
    //             if(value == 0){
    //                 remove_status(i);
    //             }
    //         }
    //     }
    // }

    for(i = 0; i < MAX_NODE_CNT; i++){
        if(i == (int)(sa-1)&0x000000FF){
            continue;
        }else{
            if(timeout[i] > 0){
                (timeout[i])--;
                if(timeout[i] == 0){
                    remove_status(i+1);
                    wflag++;
                }
            }
        }
    }

#if 0
    fprintf(stderr, "%s,%d:wflag=%d\n", __func__, __LINE__, wflag);
    for(i = 0; i < MAX_NODE_CNT; i++){
        fprintf(stderr, "%d:%d%s", i, timeout[i], (i==(MAX_NODE_CNT-1)?"\n":","));
    }
#endif

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

#if ON_BOARD
    int fd;
    rval = drvFPGA_Init(&fd);
    if(rval){
        fprintf(stderr, "%s:initialize drvFPGA failed\n", __func__);
        rval = 1;
        goto func_exit;
    }
#endif

    while(rdata != NULL){
        for(i = 0; i < config_cnt; i++){
            if(strcmp(rdata->name, config_t[i].name) == 0){
                if(config_t[i].pvalue == NULL){
                    //maybe need more code
                }
                if(strcmp(rdata->pvalue, config_t[i].pvalue) != 0){
                    if(config_t[i].pfunc != NULL){
                        rval = (*config_t[i].pfunc)(i, 2, rdata->pvalue);
                        usleep(100000);

                        //if failed
                        if(rval != 0){
                            fprintf(stderr, "config %s failed, rval = %d\n", rdata->name, rval);
                            break;
                        }

                        //if success
                        flag+=1;
                        //update config_t
                        modify_value(&config_t[i].pvalue, rdata->pvalue);

                        //update tree
                        node = search_node(config_root, rdata->name);
                        if(len > (int)strlen(node->pvalue)){
                            free(node->pvalue);
                            node->pvalue = (char*)malloc(len + 1);
                        }
                        strcpy(node->pvalue, rdata->pvalue);
                    }else{
                        // fprintf(stderr, "pfunc of %s is NULL, config failed\n", config_t[i].name);
                    }
                }
                break;
            }
        }
        rdata = rdata->next;
    }

    add_config();

func_exit:
#if ON_BOARD
    if(g_FPGA_pntr != NULL){
        drvFPGA_Close(&fd);
    }
#endif
    return flag;
}

//additional config
int add_config()
{
    int rval = 0;
    int i;

    //config port
    for(i = 0; i < PORT_CNT; i++){
        //fprintf(stderr, "%d\n", port_flag[i]);
        if(port_flag[i]){
            rval = config_uart(i);
            if(rval){
                fprintf(stderr, "%s:config uart failed\n", __func__);
            }
            port_flag[i] = 0;
        }
    }
    
    //config route
    for(i = 0; i < ROUTE_CNT; i++){
        config_route(i);
        route_flag[i] = 0;
    }

func_exit:
    return rval;
}

static int num;
/*
 * func:
 *      update local status
 */
void update_local_status()
{
    node_s *pn;
    int i;

    //do pfunc to update status_data
    for(i = 0; i < status_cnt; i++){
        //fprintf(stderr, "%s,%d: papare to config %s\n", __func__, __LINE__, status_data[i].name);
        if(status_data[i].pfunc == NULL) continue;
        else{
            (*status_data[i].pfunc)(i, 0, NULL);
        }
    }

    // pn = snode_data[sa-1];
    // remove_status(sa-1);
    remove_status(sa&0x000000FF);
    add_local_status();
    // for(i = 0; i < status_cnt; i++){
    //     stat2tree(pn, &status_data[i]);
    // }

    return ;
}

void add_local_status(){
    int i, nodeid;
    node_s *node = NULL;
    node_l *pl, *newl;

    nodeid = (int)sa&0x000000FF;

    //create sub status tree
    node = create_node(status_data[0].type, status_data[0].name, status_data[0].pvalue, status_data[0].isstr);
    for(i = 1; i < status_cnt; i++){
        // fprintf(stderr, "%s,%d:%s\n", __func__, __LINE__, status_data[i].name);
        stat2tree(node, &status_data[i]);
    }

    //insert the new tree to sstatus by order
    // pl = &sstatus->child_h;
    // while(pl->next != NULL){
    //     if(nodeid > getnumfromstr(pl->next->pnode->name)){
    //         break;
    //     }else{
    //         pl = pl->next;
    //     }
    // }
    // newl = (node_l*)malloc(sizeof(node_l));
    // newl->pnode = node;
    // newl->next = pl->next;
    // pl->next = newl;
    // if(newl->next == NULL){
    //     sstatus->child_t.next = newl;
    // }
    insert_status_2tree(nodeid, node);

    return ;
}

/*
 * func:
 *      insert a sub-status-tree to sstatus by order
 */
void insert_status_2tree(int nodeid, node_s *node)
{
    node_l *pl, *newl;

    pl = &sstatus->child_h;
    while(pl->next != NULL){
        // fprintf(stderr, "%s,%d:node:%d, from:%d\n", __func__, __LINE__, nodeid, getnumfromstr(pl->next->pnode->name));
        if(nodeid < getnumfromstr(pl->next->pnode->name)){
            break;
        }else{
            pl = pl->next;
        }
    }
    newl = (node_l*)malloc(sizeof(node_l));
    newl->pnode = node;
    newl->next = pl->next;
    pl->next = newl;
    if(newl->next == NULL){
        sstatus->child_t.next = newl;
    }
    return ;
}

/*
 * func:
 *      remove all status infomation without 'timeout'
 * params:
 *      nodeid:         node id, not index.
 */
void remove_status(int nodeid)
{
    char buf[64];
    node_l *p, *temp;

    sprintf(buf, CNAME_NODE "%d", nodeid);
    del_node(sstatus, buf);
    // p = &snode_data[i]->child_h;
    // if(p->next != NULL){
    //     p = p->next;
    //     if(0 != strcmp(p->pnode->name, "timeout")){
    //         fprintf(stderr, "error! 'timeout' is lost\n");
    //         return ;
    //     }
    //     temp = p;
    //     p = p->next;
    //     temp->next = NULL;
    //     while(p != NULL){
    //         _del_node(p->pnode);
    //         temp = p;
    //         p = p->next;
    //         free(temp);
    //     }
    // }else{
    //     fprintf(stderr, "error! 'timeout' is lost\n");
    // }
    // snode_data[i]->child_t = snode_data[i]->child_h;

    return ;
}

void *rcv_thread(void *arg)
{
    int rval = 0, i;
    int reqfd, infofd;
    struct ip_mreq mreq;
    struct sockaddr_in mserv, userv;
    struct sockaddr_in cli;
    socklen_t sock_len;
    smsg_t *pm = NULL;
    fd_set rset, std_rset;
    char node_ip[16];

    pthread_detach(pthread_self());

    for(i = 0; i < status_cnt; i++){
        if(strcmp(status_data[i].name, CNAME_IPADDRESS) == 0){
            (*status_data[i].pfunc)(i, 0, NULL);
            if(status_data[i].pvalue == NULL){
                rval = 10;
                fprintf(stderr, "%s:ip pvalue is NULL\n", __func__);
                goto thread_exit;
            }else{
                strcpy(node_ip, status_data[i].pvalue);
                fprintf(stderr, "node_ip:%s\n", node_ip);
            }
        }
    }

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
        fprintf(stderr, "Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }
    if(inet_pton(AF_INET, node_ip, &mreq.imr_interface.s_addr) <= 0){
        fprintf(stderr, "Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }
    //mreq.imr_interface.s_addr = INADDR_ANY;
    setsockopt(reqfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));

    //fill socket address variable
    mserv.sin_family = AF_INET;
    mserv.sin_port = htons(MUL_PORT);
    if(inet_pton(AF_INET, GROUP_IP, &mserv.sin_addr) <= 0){
        fprintf(stderr, "Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }

    userv.sin_family = AF_INET;
    userv.sin_port = htons(INFO_PORT);
    //fprintf(stderr, "node_ip:%s\n", node_ip);
    if(inet_pton(AF_INET, node_ip, &userv.sin_addr) <= 0){
        fprintf(stderr, "Wrong IP address\n");
        rval = 2;
        goto thread_exit;
    }

    //bind socket
    if(bind(reqfd, (struct sockaddr*)&mserv, sizeof(struct sockaddr)) < 0){
        perror("bind reqfd failed");
        rval = 3;
        goto thread_exit;
    }
    //sleep(1);
    if(bind(infofd, (struct sockaddr*)&userv, sizeof(struct sockaddr)) < 0){
        perror("bind infofd failed");
        rval = 3;
        goto thread_exit;
    }

    sock_len = sizeof(struct sockaddr);
    pm = (smsg_t*)malloc(SMSG_LEN);

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
            recvfrom(reqfd, pm, SMSG_LEN, 0, (struct sockaddr*)&cli, &sock_len);
            if(pm->type == SMSG_REQ){
                if(pm->node == sa){
                    fprintf(stderr, "recv a requst msg from myself, node %d, drop it\n", pm->node);
                    continue;
                }

                fprintf(stderr, "recv a requst msg from node %d\n", pm->node);
                pthread_mutex_lock(&pmutex);
                update_local_status();
                gen_json(STATUS_PATH, status_root);
                rval = send_info(reqfd, &cli);
                if(rval != 0){
                    fprintf(stderr, "send info failed\n");
                }
                pthread_mutex_unlock(&pmutex);
            }
        }
        if(FD_ISSET(infofd, &rset)){
            recvfrom(infofd, pm, SMSG_LEN, 0, (struct sockaddr*)&cli, &sock_len);
            if(pm->type == SMSG_INFO){
                if(pm->node == sa){
                    fprintf(stderr, "recv an info msg from myself, node %d, drop it\n", pm->node);
                    continue;
                }
                fprintf(stderr, "recv an info msg from node %d\n", pm->node);
                //printf("msg:[%s]\n", pm->buf);
                pthread_mutex_lock(&pmutex);

                rval = update_node_status(pm->node, pm->buf);
                // if(rval){
                //     update_time(pm->node, 0);
                // }else{
                //     update_time(pm->node, 15);
                // }
                if(rval){
                    fprintf(stderr, "%s,%d:update_node_status failed, rval = %d\n", __func__, __LINE__, rval);
                }else{
                    gen_json(STATUS_PATH, status_root);
                }
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
    key_t dc_key, mn_key;

    //create multicast client socket
    mc_fd = socket(AF_INET, SOCK_DGRAM, 0);
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_port = htons(MUL_PORT);
    if(inet_pton(AF_INET, GROUP_IP, &mc_addr.sin_addr) <= 0){
        fprintf(stderr, "Wrong IP address\n");
        rval = 1;
        goto func_exit;
    }


    //create message queue
    dc_key = ftok(KEY_PATH, SN_DEVCFG);
    mn_key = ftok(KEY_PATH, SN_MNCONF);

    mn_qid = msgget(mn_key, IPC_CREAT | 0755);
    if(mn_qid == -1){
        perror("msgget mn_qid");
        exit(1);
    }
    dc_qid = msgget(dc_key, IPC_CREAT | 0755);
    if(dc_qid == -1){
        perror("msgget dc_qid");
        exit(1);
    }

func_exit:
    return rval;
}

void send_req()
{
    smsg_t msg;

    msg.type = SMSG_REQ;
#if SOCKET_TEST
    msg.node = sa+1;
#else
    msg.node = sa;
#endif
    sendto(mc_fd, &msg, SMSG_LEN-sizeof(msg.buf), 0, (struct sockaddr*)&mc_addr, sizeof(struct sockaddr));

    return ;
}

int send_info(int reqfd, void *cli)
{
    smsg_t msg;
    int len = 0, llen = 0, i;
    int rval = 0;
    char buf[256];

    ((struct sockaddr_in*)cli)->sin_port = htons(INFO_PORT);

#if SOCKET_TEST
    msg.node = sa+1;
    msg.node = (msg.node>MAX_NODE_CNT)?1:msg.node;
    char test[64];
#else
    msg.node = sa;
#endif
    len += sizeof(msg.node);
    msg.type = SMSG_INFO;
    len += sizeof(msg.type);

    msg.buf[0] = 0;
    for(i = 0; i < status_cnt; i++){
        switch(status_data[i].type){
            case JSON_NORMAL:
            case JSON_ARRAY:
#if SOCKET_TEST
                sprintf(test, CNAME_NODE "%d", (int)sa&0x000000FF);
                if(strcmp(test, status_data[i].name) == 0){
                    sprintf(test, CNAME_NODE "%d", (msg.node)&0x000000FF);
                    sprintf(buf, "%s|", test);
                }else{
                    sprintf(buf, "%s|", status_data[i].name);
                }
#else
                sprintf(buf, "%s|", status_data[i].name);
#endif
                llen += strlen(buf);
                if(llen >= MAX_SOCK_LEN){
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
                if(llen >= MAX_SOCK_LEN){
                    rval = 1;
                    fprintf(stderr, "msg is too long\n");
                    goto func_exit;
                }
                strcat(msg.buf, buf);
                sprintf(buf, "%s|", status_data[i].pvalue);
                llen += strlen(buf);
                if(llen >= MAX_SOCK_LEN){
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
    //sendto(reqfd, &msg, SMSG_LEN-sizeof(msg.buf), 0, (struct sockaddr*)cli, sizeof(struct sockaddr));
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

/*
 * func:
 *      update node information to sstatus according to pmsg, if success, it will set correspoding timeout[] to 15.
 */ 
int update_node_status(int nodeid, char *pmsg)
{
    char *str1;
    int i, rval = 0;
    // sdata_s mould;
    node_s *node = NULL;
    char name[64];
    sdata_s sd;

    str1 = strtok(pmsg, "|");
    if(str1 != NULL){
        for(i = 0; i < status_cnt; i++){
            if(strncmp(str1, status_data[i].name, strlen(CNAME_NODE)) == 0){
                // fprintf(stderr, "%s,%d:name:str1:%s, sta_data:%s\n", __func__, __LINE__, str1, status_data[i].name);
                sd = status_data[i];
                strcpy(name, str1);
                strcpy(sd.name, name);
                if(sd.type == JSON_NORMAL || sd.type == JSON_ARRAY){
                    sd.pvalue = NULL;
                }else if(sd.type == JSON_STRING){
                    sd.pvalue = strtok(NULL, "|");
                    if(sd.pvalue == NULL){
                        rval = 1;
                        goto func_exit;
                    } 
                }
                if(node == NULL){
                    //create a sub-status-tree about receiving node information
                    node = create_node(sd.type, sd.name, sd.pvalue, sd.isstr);
                }else{
                    rval = 2;
                    goto func_exit;
                }
                break;
            }
        }
    }
    if(node == NULL){
        rval = 3;
        goto func_exit;
    }
    while((str1 = strtok(NULL, "|")) != NULL){
        for(i = 0; i < status_cnt; i++){
            if(strcmp(str1, status_data[i].name) == 0){
                sd = status_data[i];
                if(strncmp(sd.fname, CNAME_NODE, strlen(CNAME_NODE)) == 0){
                    // fprintf(stderr, "%s,%d:fname:sd:%s, sta_data:%s\n", __func__, __LINE__, sd.fname, name);
                    strcpy(sd.fname, name);
                }
                if(sd.type == JSON_NORMAL || sd.type == JSON_ARRAY){
                    sd.pvalue = NULL;
                }else if(sd.type == JSON_STRING){
                    sd.pvalue = strtok(NULL, "|");
                    if(sd.pvalue == NULL){
                        rval = 4;
                        goto func_exit;
                    } 
                }
                stat2tree(node, &sd);
                break;
            }
        }
    }

    //remove old status
    remove_status(nodeid);

    insert_status_2tree(nodeid, node);

func_exit:
    if(rval){
        if(node != NULL){
            _del_node(node);
        }
    }else{
        timeout[nodeid-1] = 15;
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
        node = create_node(sdata->type, sdata->name, sdata->pvalue, sdata->isstr);
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
        node = create_node(sdata->type, sdata->name, sdata->pvalue, sdata->isstr);
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














