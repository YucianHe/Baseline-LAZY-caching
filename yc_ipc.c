#include "yc_ipc.h"

/*CREATE MESSAGE QUEUE*/
/**
 * [根據指定的Key值和Message Queue Flag建立]
 * @param {key_t} key [Key值]
 * @param {int} msqflag [Message Queue Flag]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int createMessageQueue(key_t key, int msqflag) {
	int msqid = -1;
	msqid = msgget(key, msqflag | 0666);
	if(msqid >= 0) {
		PrintDebug(msqid, "Message Queue Identifier:");
		return msqid;
	}
	else
		return -1;
}

/*REMOVE MESSAGE QUEUE*/
/**
 * [根據指定的Key值和Message Queue Flag刪除]
 * @param {key_t} key [Key值]
 * @param {struct msqid_ds*} msqds [Message Queue Data Structure]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int removeMessageQueue(key_t key, struct msqid_ds *msqds) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);
	if(msgctl(msqid, IPC_RMID, msqds) == 0) {
		PrintDebug(msqid, "Remove message queue:");
		return msqid;
	}
	else
		return -1;
		
}

/*SEND REQUEST BY MESSAGE QUEUE*/
/**
 * [根據指定的Key值代表特定的Message Queue和Message Type傳送Request]
 * @param {key_t} key [Key值]
 * @param {REQ*} r [Request]
 * @param {long} msgtype [Message Queue Type]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int sendRequestByMSQ(key_t key, REQ *r, long msgtype) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF msg;

	msg.msgType = msgtype;

	msg.req.arrivalTime = r->arrivalTime;
	msg.req.devno = r->devno;
	msg.req.diskBlkno = r->diskBlkno;
	msg.req.reqSize = r->reqSize;
	msg.req.reqFlag = r->reqFlag;
	msg.req.userno = r->userno;
	msg.req.responseTime = r->responseTime;
	
	if(msgsnd(msqid, (void *)&msg, MSG_SIZE, 0) == 0) {
		PrintDebug(msqid, "A request is sent to MSQ:");
		return 0;
	}
	else
		return -1;
}


/*RECEIVE REQUEST BY MESSAGE QUEUE*/
/**
 * [根據指定的Key值代表特定的Message Queue和Message Type接收Request]
 * @param {key_t} key [Key值]
 * @param {REQ*} r [Request]
 * @param {long} msgtype [Message Queue Type]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int recvRequestByMSQ(key_t key, REQ *r, long msgtype) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF buf;
	if(msgrcv(msqid, (void *)&buf, MSG_SIZE, msgtype, 0) >= 0) {/*return bytes sent*/
		PrintDebug(msqid, "A request is received from MSQ:");
		r->arrivalTime = buf.req.arrivalTime;
		r->devno = buf.req.devno;
		r->diskBlkno = buf.req.diskBlkno;
		r->reqSize = buf.req.reqSize;
		r->reqFlag = buf.req.reqFlag;
		r->userno = buf.req.userno;
		r->responseTime = buf.req.responseTime;
		return 0;
	}
	else 
		return -1;
}

double sendRequest2SSD_to_getServiceTime(unsigned long page_no, int flag)
{

	//PrintSomething("sendRequest2SSD_to_getServiceTime[1]");

	REQ *PageReq;
	PageReq = malloc(sizeof(REQ));
	PageReq = PageReqMaker(page_no, flag);

	if(sendRequestByMSQ(KEY_MSQ_DISKSIM_SSD, PageReq, MSG_TYPE_DISKSIM_SSD) == -1)
       		PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");
       	//else
       		//PrintREQ(PageReq ,"ipc.c#113");
       	free(PageReq);

             double service = -1;  

       	REQ *rtn;
	rtn = calloc(1, sizeof(REQ));

	//Receive serviced request
	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_SSD, rtn, MSG_TYPE_DISKSIM_SSD_SERVED) == -1)
		PrintError(-1, "[PC]A request not received from MSQ in recv_request_by_MSQ():");
	//else
		//PrintREQ(rtn ,"ipc.c#122");
	//Record service time
	service = rtn->responseTime;

	//fprintf(result, "[ipc.c#SSD]service = %lf\n", service);

	//Release request variable and return service time
	return service;

	//free(rtn);
}

double sendRequest2HDD_to_getServiceTime(unsigned long page_no, int flag)
{

	REQ *PageReq;
	PageReq = malloc(sizeof(REQ));
	PageReq = PageReqMaker(page_no, flag);

	if(sendRequestByMSQ(KEY_MSQ_DISKSIM_HDD, PageReq, MSG_TYPE_DISKSIM_HDD) == -1)
       		PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");
       	//else
       		//PrintREQ(PageReq ,"ipc.c#145");
       	free(PageReq);

       	double service = -1;  

       	REQ *rtn;
	rtn = calloc(1, sizeof(REQ));

	//Receive serviced request
	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_HDD, rtn, MSG_TYPE_DISKSIM_HDD_SERVED) == -1)
		PrintError(-1, "[PC]A request not received from MSQ in recv_request_by_MSQ():");
	//else
		//PrintREQ(rtn ,"ipc.c#152");



	//Record service time
	service = rtn->responseTime;

	//fprintf(result, "[ipc.c#HDD]service = %lf\n", service);

	//Release request variable and return service time
	//free(rtn);
	return service;

}

/*TEST MESSAGE QUEUE*/

/*SEND FINISH CONTROL MESSAGE BY MESSAGE QUEUE*/
/**
 * [根據指定的Key值代表特定的Message Queue和Message Type傳送一Message(帶有特定Flag的Request)，其代表告知simulator應進行Shutdown之工作]
 * @param {key_t} key [Key值]
 * @param {long} msgtype [Message Queue Type]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int sendFinishControl(key_t key, long msgtype) {
	REQ *ctrl;
	ctrl = calloc(1, sizeof(REQ));
	ctrl -> reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH;
	if(sendRequestByMSQ(key, ctrl, msgtype) == -1) {
		PrintError(key, "A control request is not sent to MSQ in sendRequestByMSQ() return:");
		return -1;
	}
	
}

double send_to_get_period_SSDTime(key_t key, long msgtype) {
	
	REQ *ctrl;
	ctrl = calloc(1, sizeof(REQ));
	ctrl -> reqFlag = MSG_REQUEST_PERIOD_SSD_TIME;
	if(sendRequestByMSQ(key, ctrl, msgtype) == -1) {
		PrintError(key, "A control request is not sent to MSQ in sendRequestByMSQ() return:");
		return -1;
	}

	REQ *ctrl_period_ssd;
	ctrl_period_ssd = calloc(1, sizeof(REQ));
	double ssd_period_time = 0.0;

	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_SSD, ctrl_period_ssd, MSG_TYPE_DISKSIM_SSD_SERVED) == -1)
		PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
	else
		PrintREQ(ctrl_period_ssd ,"ipc.c#211");
	
	ssd_period_time = ctrl_period_ssd->responseTime;

	return ssd_period_time;

}
double send_to_get_period_HDDTime(key_t key, long msgtype) {
	REQ *ctrl;
	ctrl = calloc(1, sizeof(REQ));
	ctrl -> reqFlag = MSG_REQUEST_PERIOD_HDD_TIME;
	if(sendRequestByMSQ(key, ctrl, msgtype) == -1) {
		PrintError(key, "A control request is not sent to MSQ in sendRequestByMSQ() return:");
		return -1;
	}
	
	REQ *ctrl_period_hdd;
	ctrl_period_hdd = calloc(1, sizeof(REQ));
	double hdd_period_time = 0.0;

	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_SSD, ctrl_period_hdd, MSG_TYPE_DISKSIM_SSD_SERVED) == -1)
		PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
	else
		PrintREQ(ctrl_period_hdd ,"ipc.c#232");
	
	hdd_period_time = ctrl_period_hdd->responseTime;

	return hdd_period_time;
}

