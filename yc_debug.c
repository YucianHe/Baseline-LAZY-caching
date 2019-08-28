#include "yc_debug.h"

/**
 * [印出字串]
 * @param {char*} str [顯示字串]
 */
void PrintSomething(char* str) {
    printf("[DEBUG]%s\n", str);
}

/**
 * [印出字串與錯誤碼並結束程式]
 * @param {int} rc  [錯誤碼]
 * @param {char*} str [顯示字串]
 */
void PrintError(int rc, char* str) {
	printf(COLOR_RED"[ERROR]%s%d\n"COLOR_RESET, str, rc);
	exit(1);
}

/**
 * [印出字串與錯誤碼]
 * @param {int} rc  [錯誤碼]
 * @param {char*} str [顯示字串]
 */
void PrintDebug(int rc, char* str) {
	printf(COLOR_YELLOW"[DEBUG]%s%d\n"COLOR_RESET, str, rc);
}

/**
 * [印出Request資訊]
 * @param {REQ} r [Request]
 * @param {char*} str [顯示字串]
 */
void PrintREQ(REQ *r, char* str) {

	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("[DEBUG][%s]:arrivalTime=%lf\n", str, r->arrivalTime);
	printf("[DEBUG][%s]:devno=%u\n", str, r->devno);
	printf("[DEBUG][%s]:diskBlkno=%8lu\n", str, r->diskBlkno);
	printf("[DEBUG][%s]:reqSize=%u\n", str, r->reqSize);
	printf("[DEBUG][%s]:reqFlag=%u\n", str, r->reqFlag);
	printf("[DEBUG][%s]:userno=%u\n", str, r->userno);
	printf("[DEBUG][%s]:responseTime=%lf\n", str, r->responseTime);
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

void PrintREQinFile(REQ *r, char* str) 
{

	fprintf(result,"++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(result,"[DEBUG][%s]:arrivalTime=%lf\n", str, r->arrivalTime);
	fprintf(result,"[DEBUG][%s]:devno=%u\n", str, r->devno);
	fprintf(result,"[DEBUG][%s]:diskBlkno=%8lu\n", str, r->diskBlkno);
	fprintf(result,"[DEBUG][%s]:reqSize=%u\n", str, r->reqSize);
	fprintf(result,"[DEBUG][%s]:reqFlag=%u\n", str, r->reqFlag);
	fprintf(result,"[DEBUG][%s]:responseTime=%lf\n", str, r->responseTime);
	fprintf(result,"++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

/**
 * [印出進度]
 * @param {unsigned long} currentREQ [目前數量]
 * @param {unsigned long} totalREQ [整體數量]
 */
void printProgress(unsigned long currentREQ, unsigned long totalREQ, unsigned long currentMeta, unsigned long currentCache) {
    printf ("\rProgress:%9lu / %9lu Meta: %6lu Cache: %8lu", currentREQ, totalREQ, currentMeta, currentCache);
    fflush (stdout);
}

/*
void PrintINFO(pcSTAT in, char* str) {

	fprintf(statistics, "++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(statistics, "[%s]\n", str);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]period=%d\n", in.period);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]PageCache_Hit = %d\n", in.PageCache_Hit);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]T1_Read_Hit_Count = %d\n", in.T1_Read_Hit_Count);
	fprintf(statistics, "[DEBUG]T1_Write_Hit_Count = %d\n", in.T1_Write_Hit_Count);
	fprintf(statistics, "[DEBUG]T2_Read_Hit_Count = %d\n", in.T2_Read_Hit_Count);
	fprintf(statistics, "[DEBUG]T1_Write_Hit_Count = %d\n", in.T2_Write_Hit_Count);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]PageCache_Miss = %d\n", in.PageCache_Miss);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]PCReadMiss_to_SSDRead = %d\n", in.PCReadMiss_to_SSDRead);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]PCReadMiss_to_SSDRead_Hit = %d\n", in.PCReadMiss_to_SSDRead_Hit);
	fprintf(statistics, "[DEBUG]T2victim_c3 = %d\n", in.T2victim_c3);
	fprintf(statistics, "[DEBUG]T2victim_WSSD_c3 = %d\n", in.T2victim_WSSD_c3);
	fprintf(statistics, "[DEBUG]SSDvictim_c3 = %d\n", in.SSDvictim_c3);
	fprintf(statistics, "[DEBUG]SSDvictim_WHDD_c3 = %d\n", in.SSDvictim_WHDD_c3);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]PCReadMiss_to_SSDRead_Miss = %d\n", in.PCReadMiss_to_SSDRead_Miss);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]upingpage_GB_Hit = %d\n", in.upingpage_GB_Hit);
	fprintf(statistics, "[DEBUG]T2victim_c4 = %d\n", in.T2victim_c4);
	fprintf(statistics, "[DEBUG]T2victim_WSSD_c4 = %d\n", in.T2victim_WSSD_c4);
	fprintf(statistics, "[DEBUG]T2e_SSDvictim_c4 = %d\n", in.T2e_SSDvictim_c4);
	fprintf(statistics, "[DEBUG]T2e_SSDvictim_WHDD_c4 = %d\n", in.T2e_SSDvictim_WHDD_c4);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]upingpage_GB_Miss = %d\n", in.upingpage_GB_Miss);
	fprintf(statistics, "[DEBUG]T1victim_c4 = %d\n", in.T1victim_c4);
	fprintf(statistics, "[DEBUG]T1e_GB_Hit_c4 = %d\n", in.T1e_GB_Hit_c4);
	fprintf(statistics, "[DEBUG]T1victim_WSSD_c4 = %d\n", in.T1victim_WSSD_c4);
	fprintf(statistics, "[DEBUG]T1e_SSDvictim_c4 = %d\n", in.T1e_SSDvictim_c4);
	fprintf(statistics, "[DEBUG]T1e_SSDvictim_WHDD_c4 = %d\n", in.T1e_SSDvictim_WHDD_c4);
	fprintf(statistics, "[DEBUG]T1e_GB_Miss_c4 = %d\n", in.T1e_GB_Miss_c4);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]PCWrite_Miss = %d\n", in.PCWrite_Miss);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]T1victim_c7 = %d\n", in.T1victim_c7);
	fprintf(statistics, "[DEBUG]T2victim_WSSD_c7 = %d\n", in.T2victim_WSSD_c7);
	fprintf(statistics, "[DEBUG]SSDvictim_c7 = %d\n", in.SSDvictim_c7);
	fprintf(statistics, "[DEBUG]SSDvictim_WHDD_c7 = %d\n", in.SSDvictim_WHDD_c7);
	fprintf(statistics, "\n" );
	fprintf(statistics, "[DEBUG]T1victim_c8 = %d\n", in.T1victim_c8);
	fprintf(statistics, "[DEBUG]T1victim_WSSD_c8 = %d\n", in.T1victim_WSSD_c8);
	fprintf(statistics, "[DEBUG]T1e_GB_Hit_c8 = %d\n", in.T1e_GB_Hit_c8);
	fprintf(statistics, "[DEBUG]SSDvictim_c8 = %d\n", in.SSDvictim_c8);
	fprintf(statistics, "[DEBUG]SSDvictim_WHDD_c8 = %d\n", in.SSDvictim_WHDD_c8);
	fprintf(statistics, "[DEBUG]T1e_GB_Miss_c8 = %d\n", in.T1e_GB_Miss_c8);

	fprintf(statistics, "++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

void PrintMissRatio_in_BLOCKS(BLOCK_miss_record mrib, char* str) {

	fprintf(statistics, "%s\n", str);
	fprintf(statistics, "NUM_BLOCKS : %d\n", NUM_BLOCKS);

	int i;
	for(i = 0; i < NUM_BLOCKS; i++)	{	

		fprintf(statistics, "++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

		fprintf(statistics, "C1rm[%d] = %d\n", i, mrib.C1rm[i]);
		fprintf(statistics, "C1wm[%d] = %d\n", i, mrib.C1wm[i]);
		fprintf(statistics, "C2rm[%d] = %d\n", i, mrib.C2rm[i]);
		fprintf(statistics, "C2wm[%d] = %d\n", i, mrib.C2wm[i]);

		fprintf(statistics, "++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	}
	
	
}
*/