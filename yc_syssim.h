#ifndef YC_SYSSIM_H
#define YC_SYSSIM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>//for structure 'stat'
#include <math.h>

#include "disksim_interface.h"
#include "disksim_global.h"

#include "yc_debug.h"
#include "yc_ipc.h"
#include "yc_parameter.h"

typedef	double SysTime;		/* system time in seconds.usec */
struct Stat {
	SysTime stime;
	unsigned long pendIORequest;
	unsigned long servedIORequest;
}st;

//structure:<pre><->r<->r<-><next>
typedef struct intq_buf_req{
	struct disksim_request req;
	struct intq_buf_req *pre;
	struct intq_buf_req *next;
} IntqBufReq;

static SysTime now = 0;     /* current time */
static SysTime next_event = -1; /* next event */
static int completed = 0;   /* last request was completed */

static IntqBufReq *IBRhead = NULL;
static int IntqBufSize = 0;

/* routines for translating between the system-level simulation's simulated */
/* time format (whatever it is) and disksim's simulated time format (a      */
/* double, representing the number of milliseconds from the simulation's    */
/* initialization).                                                         */

/* In this example, system time is in seconds since initialization */
// #define SYSSIMTIME_TO_MS(syssimtime)    (syssimtime*1e3)
// #define MS_TO_SYSSIMTIME(curtime)       (curtime/1e3)

/* exported by syssim_driver.c */
void syssim_schedule_callback(disksim_interface_callback_t, SysTime t, void *);
void ssdsim_report_completion(SysTime t, struct disksim_request *r, void *);
void hddsim_report_completion(SysTime t, struct disksim_request *r, void *);
void syssim_deschedule_callback(double, void *);

void panic(const char *s);
void exec_SSDsim(char *name, const char *parm_file, const char *output_file);
void exec_HDDsim(char *name, const char *parm_file, const char *output_file);

void IntqBuffering(IntqBufReq *req);
void DeIntqBuffering();

#endif