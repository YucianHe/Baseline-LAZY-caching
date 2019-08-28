#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for fork()
#include <sys/wait.h> //for wait()

#include "yc_debug.h"
#include "yc_caching.h"
#include "yc_syssim.h"
#include "yc_ipc.h"
#include "yc_parameter.h"

/*DISKSIM INITIALIZATION*/
void initDisksim();
/*DISKSIM SHUTDOWN*/
double rmDisksim();
/*MESSAGE QUEUE INITIALIZATION*/
void initMSQ();
/*MESSAGE QUEUE REMOVE*/
void rmMSQ();

