#ifndef _XBD_MULTIPACKET_H
#define _XBD_MULTIPACKET_H

#include "xbh.h"
#include "xbh_prot.h"
#include <inttypes.h>

#define ADDRSIZE sizeof(uint32_t) //bytes used to store address 
#define LENGSIZE sizeof(uint32_t) //bytes used to store length
#define SEQNSIZE sizeof(uint32_t) //bytes used to store sequence counter
#define TYPESIZE sizeof(uint32_t)
#define TIMESIZE sizeof(uint32_t)
#define NUMBSIZE sizeof(uint32_t)
#define REVISIZE 5		//GIT/SVN revisions are 5 digit hex  numbers

#define CRC16SIZE 2

#define XBD_ANSWERLENG_MAX 32
#define XBD_PACKET_SIZE_MAX 255


#define XBD_RESULTLEN_EBASH (XBD_COMMAND_LEN+1+2+CRC16SIZE)

uint32_t XBD_genSucessiveMultiPacket(const uint8_t* srcdata, uint8_t* dstbuf, uint32_t dstlenmax, const prog_char *code);

uint32_t XBD_genInitialMultiPacket(const uint8_t* srcdata, uint32_t srclen, uint8_t* dstbuf,const prog_char *code, uint32_t type, uint32_t addr);

uint8_t XBD_recSucessiveMultiPacket(const uint8_t* recdata, uint32_t reclen, uint8_t* dstbuf, uint32_t dstlenmax, const prog_char *code);

uint8_t XBD_recInitialMultiPacket(const uint8_t* recdata, uint32_t reclen, const prog_char *code, uint8_t hastype, uint8_t hasaddr);

#endif /* _XBD_MULTIPACKET_H */