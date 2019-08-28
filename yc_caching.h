#ifndef YC_CACHING_H
#define YC_CACHING_H

#include <stdlib.h> 
#include <stdio.h>

#include "yc_parameter.h"


/*STRUCTURE DEFINITION:REQUEST*/
typedef struct req {
	double arrivalTime;		//抵達時間
	unsigned devno;		//裝置編號(預設為0)
	unsigned long diskBlkno;	//Block編號(根據Disksim格式)
	unsigned reqSize;		//Block連續數量(至少為1)
	unsigned reqFlag;		//讀:1;寫:0
	unsigned userno;		//使用者編號(1~n)
	double responseTime;		//反應時間(初始為0)
} REQ;

typedef struct PageCache_list_node {
	unsigned long page_no;
	int flag;
	int LAZY;

	struct PageCache_list_node *PC_next; //LRU
	struct PageCache_list_node *PC_pre;   //MRU
} PCnode;

typedef struct GHOSTbuffer_list_node
{	
	unsigned long page_no;
	int flag;
	int LAZY;

   	struct GHOSTbuffer_list_node *ghost_next; //LRU
   	struct GHOSTbuffer_list_node *ghost_pre;   //MRU	
}GHOSTbuffernode;

typedef struct SSD_list_node
{	
	unsigned long page_no;
	int flag;
	int LAZY;

   	struct SSD_list_node *SSD_next; //LRU
   	struct SSD_list_node *SSD_pre;   //MRU	
}SSDnode;

/*
typedef struct node {
	//unsigned long diskBlkno;
	unsigned long page_no;
	int flag;				//1 :  clean,  0 : dirty,  2 :  LAZY + clean
	
	struct  node *PC_next; 		//(PC_HDD)LRU
	struct node *PC_pre;   		//(PC_HDD)MRU
	struct  node *ghost_next; 	//LRU
	struct node *ghost_pre;   	//MRU
	struct  node *SSD_next; 	//LRU
	struct node *SSD_pre;   		//MRU
} NODE;
*/

static PCnode *PChead = NULL, *PClast = NULL;
static GHOSTbuffernode *Ghead = NULL, *Glast = NULL;
static SSDnode *SSDhead = NULL, *SSDlast = NULL;

static int PCUsed = 0;
static int GHOSTbufferUsed = 0;
static int SSDUsed = 0;

static int GHOSTbufferSize = SSDSize*0.1;
static int GHOSTsize1, GHOSTsize2;

//for deal with the coming request 
int CheckPageCache(unsigned long page_no, int flag);

//for search 
PCnode *searchPageCachequeue(unsigned long page_no, int flag);
SSDnode *searchSSD(unsigned long page_no);
GHOSTbuffernode *searchGHOSTbuffer(unsigned long page_no);

//for checking the page is in which area
int CheckSSDTable(unsigned long page_no, int flag);
void Adjust_Glast(int New_GBSize);
int checkGHOSTbuffer(unsigned long page_no, int flag);

//metain the queues
int insertPageCache(unsigned long page_no, int flag);
int insertGHOSTbuffer(unsigned long page_no, int flag);
int insertSSD(unsigned long page_no, int flag);

PCnode *FindingPageCachevictim();
SSDnode *FindingSSDvictim();
void *Evict_PageCachevictim(PCnode *eviction);
void  *Evict_SSDvictim(SSDnode *eviction);

void displayPageCacheTable();
void displayGHOSTbuffer();
void displaySSDTable();

REQ *PageReqMaker(unsigned long page_no, int flag);

extern int T1ReadHit[PageCacheSize+1], T2ReadHit[PageCacheSize+1];
extern int T1WriteHit[PageCacheSize+1], T2WriteHit[PageCacheSize+1];

static int c_PC_NODE = 0, c_GB_NODE = 0, c_SSD_NODE = 0;
int get_NODE_counts();

#endif