#ifndef YC_IPC_H
#define YC_IPC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "yc_debug.h"
#include "yc_caching.h"
#include "yc_parameter.h"

/*STRUCTURE DEFINITION: MESSAGE*/
typedef struct msgbuf {
	long msgType;
	REQ req;
} MSGBUF;

//MESSAGE SIZE
#define MSG_SIZE sizeof(MSGBUF)-sizeof(long)

/*CREATE MESSAGE QUEUE*/
int createMessageQueue(key_t key, int msqflag);
/*REMOVE MESSAGE QUEUE*/
int removeMessageQueue(key_t key, struct msqid_ds *msqds);
/*SEND REQUEST BY MESSAGE QUEUE*/
int sendRequestByMSQ(key_t key, REQ *r, long msgtype);
/*RECEIVE REQUEST BY MESSAGE QUEUE*/
int recvRequestByMSQ(key_t key, REQ *r, long msgtype);
/*******
EXAMPLE:
if(sendRequestByMSQ(KEY_MSQ_DISKSIM_2, tmp, MSG_TYPE_DISKSIM_2) == -1)
    PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");
sendSimulateControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
    PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
*******/
//double get_service_time(key_t key, long msgtype, REQ *r);

double sendRequest2SSD_to_getServiceTime(unsigned long page_no, int flag);
double sendRequest2HDD_to_getServiceTime(unsigned long page_no, int flag);

/*TEST MESSAGE QUEUE*/
//[測試使用，僅參考，可忽略]
void testMessageQueue();

/*SEND FINISH CONTROL MESSAGE BY MESSAGE QUEUE*/
int sendFinishControl(key_t key, long msgtype); //Called by rmDisksim()
double send_to_get_period_SSDTime(key_t key, long msgtype);
double send_to_get_period_HDDTime(key_t key, long msgtype);
#endif
