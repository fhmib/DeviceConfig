#include "dc_io.h"

extern pthread_mutex_t pmutex;

extern sdata_s status_data[];
extern int status_cnt;

extern U64 txbytes, txpackets, txerrors; 
extern U64 rxbytes, rxpackets, rxerrors; 
extern struct timeval now_tv, pre_tv;

extern sdata_s config_t[];
extern int config_cnt;

int io_undo(int index, char mode, char *pvalue)
{
    return 0;
}

int io_ipTxByteCnt(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[32];
#if 0
    int time;

    time = (now_tv.tv_sec - pre_tv.tv_sec)*1000000 + (now_tv.tv_usec - pre_tv.tv_usec);
    if(time > 1200000){
        //maybe need more code
    }
    printf("time = %d\n", time);
#endif

    sprintf(buf, "%lld", txbytes);
    modify_value(&status_data[index].pvalue, buf);

    return rval;
}

int io_ipTxPktCnt(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[32];

    sprintf(buf, "%lld", txpackets);
    modify_value(&status_data[index].pvalue, buf);

    return rval;
}

int io_ipTxErrorCnt(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[32];

    sprintf(buf, "%lld", txerrors);
    modify_value(&status_data[index].pvalue, buf);

    return rval;
}

int io_ipRxByteCnt(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[32];

    sprintf(buf, "%lld", rxbytes);
    modify_value(&status_data[index].pvalue, buf);

    return rval;
}

int io_ipRxPktCnt(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[32];

    sprintf(buf, "%lld", rxpackets);
    modify_value(&status_data[index].pvalue, buf);

    return rval;
}

int io_ipRxErrorCnt(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[32];

    sprintf(buf, "%lld", rxerrors);
    modify_value(&status_data[index].pvalue, buf);

    return rval;
}

int io_ipAddress(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[16];
    char wbuf[64];
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *ifinfo;

    if(mode == 0){
        if(getifaddrs(&ifaddr) < 0){
            perror("getifaddr");
            rval = 1;
            goto func_exit;
        }
        for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
            ifinfo = (struct sockaddr_in*)ifa->ifa_addr;
            if((ifinfo->sin_family != AF_INET) || (strcmp(ifa->ifa_name, NET_DEV_NAME) != 0))
                continue;
            inet_ntop(AF_INET, &ifinfo->sin_addr.s_addr, buf, 16);
            //printf("device name:%s\n", ifa->ifa_name);
            //printf("ip address:%s\n", inet_ntop(AF_INET, &ifinfo->sin_addr.s_addr, buf, 16));
            break;
        }

        modify_value(&status_data[index].pvalue, buf);
        freeifaddrs(ifaddr);
    }else{
        sprintf(wbuf, "ifconfig " NET_DEV_NAME " %s", pvalue);
#if PRINT_COMMAND
        printf("%s\n", wbuf);
#endif
#if ON_BOARD
        system(wbuf);
#endif
    }

func_exit:
    return rval;
}

int io_ipMask(int index, char mode, char *pvalue)
{
    int rval = 0;
    char wbuf[64];

    sprintf(wbuf, "ifconfig " NET_DEV_NAME " netmask %s", pvalue);
#if PRINT_COMMAND
    printf("%s\n", wbuf);
#endif
#if ON_BOARD
    system(wbuf);
#endif

    return rval;
}

int io_ipGateway(int index, char mode, char *pvalue)
{
    int rval = 0;
    char wbuf[64];

    sprintf(wbuf, "route del default");

#if PRINT_COMMAND
    printf("%s\n", wbuf);
#endif
#if ON_BOARD
    system(wbuf);
#endif

    sprintf(wbuf, "route add default gw %s", pvalue);

#if PRINT_COMMAND
    printf("%s\n", wbuf);
#endif
#if ON_BOARD
    system(wbuf);
#endif

    return rval;
}

int io_nodeId(int index, char mode, char *pvalue)
{
    int rval = 0;
    int i;
    FILE *fp = NULL;

    if(mode == 0){
        for(i = 0; i < config_cnt; i++){
            if(strcmp(status_data[index].name, config_t[i].name) == 0)
                break;
        }
        if(i >= config_cnt){
            fprintf(stderr, "%s:cannot find %s in config_t\n", __func__, status_data[index].name);
            rval = 1;
            goto func_exit;
        }

        modify_value(&status_data[index].pvalue, config_t[i].pvalue);
    }else{
        rval = modify_file(XD_INIT_PATH, "inode_id=", "=", NULL, pvalue);
        if(rval){
            fprintf(stderr, "%s:modify_file failed, rval = %d\n", __func__, rval);
            rval = 2;
            goto func_exit;
        }
    }

func_exit:
    if(fp != NULL){
        free(fp);
        fp = NULL;
    }
    return rval;
}

int io_nodeName(int index, char mode, char *pvalue)
{
    int rval = 0;
    int i;
    FILE *fp = NULL;

    if(mode == 0){
        for(i = 0; i < config_cnt; i++){
            if(strcmp(status_data[index].name, config_t[i].name) == 0)
                break;
        }
        if(i >= config_cnt){
            fprintf(stderr, "%s:cannot find %s in config_t\n", __func__, status_data[index].name);
            rval = 1;
            goto func_exit;
        }

        modify_value(&status_data[index].pvalue, config_t[i].pvalue);
    }else{
    }

func_exit:
    if(fp != NULL){
        free(fp);
        fp = NULL;
    }
    return rval;
}

int io_chanBW(int index, char mode, char *pvalue)
{
    int rval = 0;

    if(mode == 0){
        rval = 1;
        goto func_exit;
    }else{
        rval = modify_ini(XD_CONFIG_PATH, "BANDW", pvalue);
        if(rval){
            fprintf(stderr, "%s:modify_ini failed, rval = %d\n", __func__, rval);
            rval = 2;
            goto func_exit;
        }
    }

func_exit:
    return rval; 
}

/*
 * func:
 *      modify the value of 'pvalue' according to 'value'
 */
void modify_value(char **pvalue, char *value)
{
    unsigned int len;

    if(value == NULL){
        if(*pvalue != NULL){
            free(*pvalue);
            *pvalue = NULL;
        }
    }else{
        len = strlen(value);

        if(*pvalue == NULL){
            *pvalue = (char*)malloc(len+1);
        }else{
            if(strlen(*pvalue) < len){
                free(*pvalue);
                *pvalue = (char*)malloc(len+1);
            }
        }

        strcpy(*pvalue, value);
    }

    return ;
}

/*
 * func:
 *      according to a keyword, modify the line after its line, use for config.ini
 */
int modify_ini(const char *file_path, const char *p_kw, const char *p_str)
{
    int rval = 0;
    int len, size, i;
    char flag = 0;
    char line[1024];
    char *pkw = NULL;
    char *pstr = NULL;
    char *file_tmp = NULL;
    FILE *fp = NULL;

    if(file_path == NULL || p_kw == NULL || p_str == NULL){
        rval = 1;
        goto func_exit;
    }

    len = strlen(p_kw);
    if(len <= 0){
        rval = 2;
        goto func_exit;
    }
    pkw = (char*)malloc(len+1);
    strcpy(pkw, p_kw);

    len = strlen(p_str);
    if(len <= 0){
        rval = 3;
        goto func_exit;
    }
    pstr = (char*)malloc(len+1);
    strcpy(pstr, p_str);

    size = file_size(file_path);
    len = size + strlen(p_str) + 1;
    file_tmp = (char*)malloc(len);
    file_tmp[0] = 0;

    //open file
    fp = fopen(file_path, "r");
    if(fp == NULL){
        rval = 4;
        goto func_exit;
    }

    while(!feof(fp)){
        if(NULL == fgets(line, sizeof(line), fp)){
            continue;
        }

        len = strlen(line);
        for(i = 0; i < len; i++){
            if(' ' == line[i]) continue;
            else break;
        }

        //ingore commented line and space line
        if('#' == line[i]){
            strcat(file_tmp, line);
            continue;
        }

        switch(flag){
            case 0:
                if(NULL != strstr(line, pkw)){
                    flag = 1;
                }
                break;
            case 1:
                sprintf(line, "%s\n", pstr);
                flag = 2;
                break;
            default:
                break;
        }

        strcat(file_tmp, line);

    }

    //write file
    fclose(fp);
    fp = fopen(file_path, "w");
    fprintf(fp, "%s", file_tmp);

func_exit:
    if(fp != NULL) fclose(fp);
    if(pkw != NULL) free(pkw);
    if(pstr != NULL) free(pstr);
    if(file_tmp != NULL) free(file_tmp);

    return rval;
}

/*
 * function:
 *      according to some keywords, modify a string within an appointed file.
 *      automatic ignore the line heads of '#' or '//'.
 * parameters:
 *      file_path:          path of the file, can not be NULL.
 *      p_kw:               key point to a string stores msg which indicates the line will be modified, it must be a unique key of all lines.
 *      p_head:             point to a string stores msg before the string you want to modify, such as "name=" or "id:".
 *      p_end:              point to a string stores msg after the string you want to modify, such as " " or "\n". defalut "\n".
 *      p_str:              point to a string stores msg you want to add.
 * return:
 *      0:                  success
 *      other:              failure
 */
int modify_file(const char *file_path, const char *p_kw, const char *p_head, const char *p_end, const char *p_str)
{
    //declare and initialze variables
    int rval = 0;
    int len = 0;
    int size = 0;
    int i;
    char flag = 0;
    char line[1024];
    char *pos_h, *pos_e;
    char *file_tmp = NULL;
    char *line_tmp = NULL;
    FILE *fp = NULL;
    char *pkw = NULL;
    char *ph = NULL;
    char *pe = NULL;
    char *str = NULL;

    //check and save parameters
    if(file_path == NULL || p_head == NULL || p_str == NULL){
        rval = 1;
        goto func_exit;
    }

    len = strlen(p_head);
    if(len <= 0){
        rval = 2;
        goto func_exit;
    }
    ph = (char*)malloc(len+1);
    strcpy(ph, p_head);

    if(p_kw == NULL) pkw = ph;
    else{
        len = strlen(p_kw);
        if(len <= 0){
            rval = 3;
            goto func_exit;
        }
        pkw = (char*)malloc(len+1);
        strcpy(pkw, p_kw);
    }

    if(p_end == NULL){
        pe = (char*)malloc(2);
        *pe = '\n';
        *(pe+1) = 0;
    }
    else{
        len = strlen(p_end);
        if(len <= 0){
            rval = 4;
            goto func_exit;
        }
        pe = (char*)malloc(len+1);
        strcpy(pe, p_end);
    }

    len = strlen(p_str);
    if(len <= 0){
        rval = 5;
        goto func_exit;
    }
    str = (char*)malloc(len+1);
    strcpy(str, p_str);

    size = file_size(file_path);
    len = size + strlen(p_str) + 1;
    file_tmp = (char*)malloc(len);
    file_tmp[0] = 0;

    //open file
    fp = fopen(file_path, "r");
    if(fp == NULL){
        rval = 6;
        goto func_exit;
    }

    //read file and insert string
    while(!feof(fp)){
        if(NULL == fgets(line, sizeof(line), fp)){
            continue;
        }
        if((NULL == strstr(line, pkw)) || (flag)){
            strcat(file_tmp, line);
            continue;
        }

        len = strlen(line);
        for(i = 0; i < len; i++){
            if(' ' == line[i]) continue;
            else break;
        }

        //ingore commented line
        if('#' == line[i] || ('/' == line[i] && '/' == line[i+1])){
            strcat(file_tmp, line);
            continue;
        }

        pos_h = strstr(line, ph);
        if(NULL == pos_h){
            rval = 7;
            fprintf(stderr, "%s:line=[%s], ph=[%s]\n", __func__, line, ph);
            goto func_exit;
        }
        pos_h += strlen(ph);
        pos_e = strstr(pos_h, pe);
        if(NULL == pos_e){
            rval = 8;
            goto func_exit;
        }
        len = strlen(pos_e) + 1;
        line_tmp = (char*)malloc(len);
        strcpy(line_tmp, pos_e);
        len = strlen(pos_h);
        //memset(pos_h, 0, len);
        *pos_h = 0;
        strcat(pos_h, str);
        pos_h += strlen(str);
        strcat(pos_h, line_tmp);

        free(line_tmp);
        line_tmp = NULL;
        flag++;

        strcat(file_tmp, line);
    }

    //write file
    fclose(fp);
    fp = fopen(file_path, "w");
    fprintf(fp, "%s", file_tmp);
    fclose(fp);
    fp = NULL;

func_exit:
    if(line_tmp != NULL){
        free(line_tmp);
        line_tmp = NULL;
    }
    if(file_tmp != NULL){
        free(file_tmp);
        file_tmp = NULL;
    }
    if(pkw != NULL){
        if(pkw == ph) pkw = NULL;
        else{
            free(pkw);
            pkw = NULL;
        }
    }
    if(ph != NULL){
        free(ph);
        ph = NULL;
    }
    if(pe != NULL){
        free(pe);
        pe = NULL;
    }
    if(str != NULL){
        free(str);
        str = NULL;
    }
    if(fp != NULL){
        fclose(fp);
        fp = NULL;
    }
    return rval;
}

/*
 * function:
 *      get file length
 * parameters:
 *      filename:               file's path
 * return:
 *      file length
 */
int file_size(const char* filename)
{
    struct stat statbuf;
    int size;

    stat(filename, &statbuf);
    size = statbuf.st_size;

    return size;
}


