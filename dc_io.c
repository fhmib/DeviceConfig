#include "dc_io.h"

extern pthread_mutex_t pmutex;
extern U8 sa;

extern sdata_s status_data[];
extern int status_cnt;

extern U64 txbytes, txpackets, txerrors; 
extern U64 rxbytes, rxpackets, rxerrors; 
extern struct timeval now_tv, pre_tv;

extern sdata_s config_t[];
extern int config_cnt;

char buf_capvol[] = "/home/bin/amixer cset numid=90,iface=MIXER,name='Capture Volume' ";
char buf_playvol[] = "/home/bin/amixer cset numid=108,iface=MIXER,name='Lineout Playback Volume' ";
char buf_alcvol[] = "/home/bin/amixer cset numid=102,iface=MIXER,name='ALC Capture Noise Gate Threshold Volume' ";
char buf_audio[] = "/home/rzxt_mesh/aaf0216/";

extern char port_flag[];

int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};




/*
 * describe:
 *      io_*** is the function of read/config device parameters.
 * parameters:
 *      index:              index of status_data[] or config_t[]
 *      mode:               0 --- read para
 *                          1 --- config device(boot)
 *                          2 --- config device(config.json)
 *      pvalue:             config value
 */




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
        fprintf(stderr, "%s\n", wbuf);
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
    fprintf(stderr, "%s\n", wbuf);
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
    fprintf(stderr, "%s\n", wbuf);
#endif
#if ON_BOARD
    system(wbuf);
#endif

    sprintf(wbuf, "route add default gw %s", pvalue);

#if PRINT_COMMAND
    fprintf(stderr, "%s\n", wbuf);
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
        rval = 0;
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

int io_dataRate(int index, char mode, char *pvalue)
{
    int rval = 0;
    int i, cnt, value;

    if(mode == 0){
        rval = 1;
        goto func_exit;
    }else{
        //fprintf(stderr, "%s:%d\n", __func__, i);
        sscanf(pvalue, "%d", &value);
        cnt = sizeof(name_arr)/sizeof(int);
        for(i = 0; i < cnt; i++){
            if(value == name_arr[i]){
                break;
            }
        }

        if(i >= cnt){
            rval = 2;
            goto func_exit;
        }else{
            i = getnumfromstr(config_t[index].name);
            port_flag[i] = 1;
        }
    }

func_exit:
    return rval;
}

int io_dataParity(int index, char mode, char *pvalue)
{
    int rval = 0;
    int i;

    if(mode == 0){
        rval = 1;
        goto func_exit;
    }else{
        //fprintf(stderr, "%s:%d\n", __func__, i);
        if((strcmp(pvalue, "odd") != 0) && (strcmp(pvalue, "even") != 0) && (strcmp(pvalue, "none") != 0)){
            rval = 2;
            goto func_exit;
        }else{
            i = getnumfromstr(config_t[index].name);
            port_flag[i] = 1;
        }
    }

func_exit:
    return rval;
}

int io_audioEnable(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[128];
    int value;

    if(mode == 0){
        rval = 1;
        goto func_exit;
    }else if(mode == 2){
        sprintf(buf, "killall " AUDIO_NAME);

#if PRINT_COMMAND
        fprintf(stderr, "%s\n", buf);
#endif
#if ON_BOARD
        system(buf);
#endif

        sscanf(pvalue, "%d", &value);
        if(value != 0){
            sprintf(buf, "%s" AUDIO_NAME " %d", buf_audio, sa);
            usleep(100000);

#if PRINT_COMMAND
            fprintf(stderr, "%s\n", buf);
#endif
#if ON_BOARD
            system(buf);
#endif
        }

    }else{
        rval = 0;
    }

func_exit:
    return rval;
}

int io_audioVol(int index, char mode, char *pvalue)
{
    int rval = 0;
    char buf[128];
    int value;

    if(mode == 0){
        rval = 1;
        goto func_exit;
    }else{
        sscanf(pvalue, "%d", &value);

        if(strcmp(config_t[index].name, CNAME_AUDIOPLAY) == 0){
            if((value < 0) || (value > 63)){
                rval = 2;
                goto func_exit;
            }
            sprintf(buf, "%s%s", buf_playvol, pvalue);
        }else if(strcmp(config_t[index].name, CNAME_AUDIOMIC) == 0){
            if((value < 0) || (value > 63)){
                rval = 2;
                goto func_exit;
            }
            sprintf(buf, "%s%s", buf_capvol, pvalue);
        }else if(strcmp(config_t[index].name, CNAME_AUDIOALC) == 0){
            if((value < 0) || (value > 31)){
                rval = 2;
                goto func_exit;
            }
            sprintf(buf, "%s%s", buf_alcvol, pvalue);
        }

#if PRINT_COMMAND
        fprintf(stderr, "%s\n", buf);
#endif
#if ON_BOARD
        system(buf);
#endif
    }

func_exit:
    return rval;
}

int getnumfromstr(char *arg)
{
    char value = -1;
    char *p;

    p = arg;
    while(*p != 0){
        if((*p >= '0') && (*p <= '9')){
            sscanf(p, "%c", &value);
            value -= '0';
            break;
        }
        p++;
    }

    return (int)value;
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

int config_uart(int which)
{
    int rval = 0;
    int fd = -1;
    int i;
    int speed;
    char *port_path, *sparity;
    char parity;

    switch(which){
        case 0:
            port_path = (char*)malloc(strlen(UART0_PATH)+1);
            strcpy(port_path, UART0_PATH);
            for(i = 0; i < config_cnt; i++){
                if(strcmp(config_t[i].name, CNAME_UART0RATE) == 0){
                    sscanf(config_t[i].pvalue, "%d", &speed);
                }else if(strcmp(config_t[i].name, CNAME_UART0PARITY) == 0){
                    //fprintf(stderr, "%s\n", config_t[i].pvalue);
                    sparity = config_t[i].pvalue;
                }
            }
            break;
    }

    if(strcmp("even", sparity) == 0){
        parity = 'e';
    }else if(strcmp("odd", sparity) == 0){
        parity = 'o';
    }else{
        parity = 'n';
    }

#if PRINT_COMMAND
    fprintf(stderr, "%s: port_path=%s, which=%d, speed=%d, parity=%c\n", __func__, port_path, which, speed, parity);
#endif
#if ON_BOARD
    fd = uart_open(port_path);
    if(0 == fd){
        fprintf(stderr, "%s:uart open failed\n", __func__);
        fd = -1;
        rval = 1;
        goto func_exit;
    }

    rval = set_uart(fd, speed, 0, 8, 1, parity);
    if(rval){
        rval = 2;
        goto func_exit;
    }else{
        fprintf(stderr, "%s:config uart success\n", __func__);
    }
#endif

func_exit:
    if(fd != -1){
        uart_close(fd);
    }
    return rval;
}

/* function:
 *      open file and return file descriptor
 * parameters:
 *      fd:         file descriptor
 *      port:       path of uart file
 * return:
 *      0:          failure
 *      other:      value of file descriptor
 */
int uart_open(const char *port_path)
{
    int rval = 0;
    int fd;

    fd = open(port_path, O_RDWR | O_NOCTTY | O_NDELAY);
    //fd = open(port_path, O_RDWR);
    if(-1 == fd){
        fprintf(stderr, "%s:open file failed\n", __func__);
        rval = 1;
        goto func_exit;
    }

    if(fcntl(fd, F_SETFL, 0) < 0){
        fprintf(stderr, "%s:fcntl failed\n", __func__);
        rval = 2;
        goto func_exit;
    }

func_exit:
    if(rval) return 0;
    else return fd;
}

void uart_close(int fd)
{
    close(fd);
}

/*
 * functions:
 *      set parameters to fd
 * parameters:
 *      fd:         file descriptor
 *      speed:      uart speed
 *      flow_ctrl:  whether uart has flow control
 *      databits:   data width
 *      stopbits:   whether uart has stop bit
 *      parity:     type of parity
 * return:
 *      0:          success
 *      other:      failure
 */
int set_uart(int fd, int speed, int flow_ctrl, int databits, int stopbits, char parity)
{
    int rval = 0;
    int i;
    int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};

    struct termios options;

    if(tcgetattr(fd, &options) != 0){
        rval = 1;
        fprintf(stderr, "%s:tcgetattr failed\n", __func__);
        goto func_exit;
    }

    for(i = 0; i < (int)(sizeof(speed_arr)/sizeof(int)); i++)
    {
        if(speed == name_arr[i]){
            //fprintf(stderr, "%s:speed is %d\n", __func__, speed);
            rval = cfsetispeed(&options, speed_arr[i]);
            if(rval == -1){
                fprintf(stderr, "%s:cfsetispeed falied\n", __func__);
            }
            rval = cfsetospeed(&options, speed_arr[i]);
            if(rval == -1){
                fprintf(stderr, "%s:cfsetispeed falied\n", __func__);
            }
            
            break;
        }
    }
    if(i >= (int)(sizeof(speed_arr)/sizeof(int))){
        fprintf(stderr, "%s:input speed error\n", __func__);
        rval = 2;
        goto func_exit;
    }

    /*
    speed = cfgetispeed(&options);
    fprintf(stderr, "%s:ispeed is %d\n", __func__, speed);
    speed = cfgetospeed(&options);
    fprintf(stderr, "%s:ospeed is %d\n", __func__, speed);
    */
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;

    switch(flow_ctrl)
    {
        case 0:
            options.c_cflag &= ~CRTSCTS;
            break;
        case 1:
            options.c_cflag |= CRTSCTS;
            break;
        /*
        case 2:
            options.c_iflag |= IXON | IXOFF | IXANY;
            break;
        */
    }

    options.c_cflag &= ~CSIZE;
    switch(databits)
    {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "%s:Unsupported data size\n", __func__);
            rval = 2;
            goto func_exit;
    }

    switch(parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= (INPCK | ISTRIP);
            break;
        default:
            fprintf(stderr, "%s:Unsupported parity\n", __func__);
            rval = 3;
            goto func_exit;
    }

    switch(stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "%s:Unsupported stop bits\n", __func__);
            rval = 4;
            goto func_exit;
    }

    options.c_cflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(INLCR | IGNCR | ICRNL);
    options.c_iflag &= ~(ONLCR | OCRNL);
    options.c_oflag &= ~(INLCR | IGNCR | ICRNL);
    options.c_oflag &= ~(ONLCR | OCRNL);


    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_cc[VTIME] = 2;
    options.c_cc[VMIN] = 0;

    //clean input and output buffer
    tcflush(fd, TCIFLUSH);

    if(tcsetattr(fd, TCSANOW, &options) != 0){
        fprintf(stderr, "%s:com set error\n", __func__);
        rval = 5;
        goto func_exit;
    }

func_exit:
    return rval;
}

