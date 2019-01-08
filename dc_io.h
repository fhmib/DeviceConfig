#ifndef _DC_IO_H_
#define _DC_IO_H_

#include "dc_common.h"

#define DEVNAME                 "/dev/mem"

#define FPGA_ADDR_BASE          0x42000000
#define BB_BASE_ADDR            0x8000
#define AD9361_BASE_ADDR        0xF000
#define _FPGA_IO_(ZZ)           (*((int*)(g_FPGA_pntr+ZZ)))
#define ADDR_9361_SPI_BUSY      (AD9361_BASE_ADDR+(0x3FF<<2))

#pragma pack(1)

typedef struct _cost_t {
    U8 mtu;
    U16 queue_len;
    U8 snr;
} cost_t;

typedef struct _citem_t {
    U16 sparse : 1;
    U16 seq : 15;
    cost_t cost[MAX_NODE_CNT];
} citem_t;

typedef struct _ctable_t {
    citem_t item[MAX_NODE_CNT];
} ctable_t;

#pragma pack()

typedef struct _phy_info_t {
    U32 floor_noise[2 * MAX_NODE_CNT];
    U32 distance[MAX_NODE_CNT];
} phy_info_t;

typedef struct _route_info_t {
    ctable_t ct;
    phy_info_t phy;
} route_info_t;

typedef struct _rtable_t{
    U8 mtu[MAX_NODE_CNT];
    U8 snr[MAX_NODE_CNT];
    U32 floor_noise[2*MAX_NODE_CNT];
    U32 distance[MAX_NODE_CNT];
}rtable_t;

int io_undo(int, char, char*);

int io_readInfo(int, char, char*);
int io_readfrominfo(int, char, char*);
int io_nodeHeader(int, char, char*);
int io_getMtu(int, char, char*);
int io_getSnr(int, char, char*);
int io_getNoise0(int, char, char*);
int io_getNoise1(int, char, char*);
int io_getDistance(int, char, char*);
int io_macAddr(int, char, char*);

int io_ipTxByteCnt(int, char, char*);
int io_ipTxPktCnt(int, char, char*);
int io_ipTxErrorCnt(int, char, char*);
int io_ipRxByteCnt(int, char, char*);
int io_ipRxPktCnt(int, char, char*);
int io_ipRxErrorCnt(int, char, char*);

int io_gpsUTCDate(int, char, char*);
int io_gpsUTCTime(int, char, char*);
int io_gpsSateCnt(int, char, char*);
int io_gpsLatitude(int, char, char*);
int io_gpsLatitudeHem(int, char, char*);
int io_gpsLongitude(int, char, char*);
int io_gpsLongitudeHem(int, char, char*);
int io_gpsSpeed(int, char, char*);
int io_gpsCourse(int, char, char*);
int io_gpsHDOP(int, char, char*);
int io_gpsVDOP(int, char, char*);
int io_gpsHeight(int, char, char*);
int io_gpsType(int, char, char*);

int io_voltage(int, char, char*);
int io_temperature(int, char, char*);
int io_ipAddress(int, char, char*);
int io_ipMask(int, char, char*);
int io_ipGateway(int, char, char*);
int io_nodeId(int, char, char*);
int io_nodeName(int, char, char*);
int io_chanBW(int, char, char*);
int io_tfci(int, char, char*);
int io_txPower(int, char, char*);
int io_dataRate(int, char, char*);
int io_dataParity(int, char, char*);
int io_dataWidth(int, char, char*);
int io_stopBit(int, char, char*);
int io_gpsEnable(int, char, char*);
int io_route(int, char, char*);

int io_audioEnable(int, char, char*);
int io_audioVol(int, char, char*);

int modify_ini(const char*, const char*, const char*);
int modify_file(const char*, const char*, const char*, const char*, const char*);
int file_size(const char*);
int config_uart(int);
int uart_open(const char*);
void uart_close(int);
int set_uart(int, int, int, int, int, char);
int ad9361_write(int, int);
int drvFPGA_Read(int);
int drvFPGA_Write(int, int);
int drvFPGA_Init(int*);
int drvFPGA_Close(int*);
void del_route(int);
void config_route(int);
int readvaluefromfile(const char*, int*);
void read_route();
void print_rtable();

#endif
