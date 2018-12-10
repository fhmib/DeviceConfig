#ifndef _DC_IO_H_
#define _DC_IO_H_

#include "dc_common.h"

int io_undo(int, char, char*);

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

void modify_value(char**, char*);
int modify_ini(const char*, const char*, const char*);
int modify_file(const char*, const char*, const char*, const char*, const char*);
int file_size(const char*);


















#endif
