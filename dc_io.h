#ifndef _DC_IO_H_
#define _DC_IO_H_

#include "dc_common.h"

#define DEVNAME                 "/dev/mem"

#define FPGA_ADDR_BASE          0x42000000
#define BB_BASE_ADDR            0x8000
#define AD9361_BASE_ADDR        0xF000
#define _FPGA_IO_(ZZ)           (*((int*)(g_FPGA_pntr+ZZ)))
#define ADDR_9361_SPI_BUSY      (AD9361_BASE_ADDR+(0x3FF<<2))

int io_undo(int, char, char*);

int io_readInfo(int, char, char*);
int io_macAddr(int, char, char*);

int io_ipTxByteCnt(int, char, char*);
int io_ipTxPktCnt(int, char, char*);
int io_ipTxErrorCnt(int, char, char*);
int io_ipRxByteCnt(int, char, char*);
int io_ipRxPktCnt(int, char, char*);
int io_ipRxErrorCnt(int, char, char*);

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
int io_route(int, char, char*);

int io_audioEnable(int, char, char*);
int io_audioVol(int, char, char*);

int getnumfromstr(char*);
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













#endif
