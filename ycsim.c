#include "ycsim.h"

pid_t SSDsimProc, HDDsimProc;              //Sub-process id: SSD and HDD simulator
//FILE *trace,*statistics, *result;      	//Trace file; Stat file; Result file
char *par[6];                           		//Arguments in yusim execution
//double scheduleTime = 0;                	//Simulation time
//unsigned long period = 0;               	//The counter of time period

int PCHit = 0;
int PCMiss = 0;
int SSD_R_Hit = 0, SSD_W_Hit = 0;
int SSD_R_Miss = 0, SSD_W_Miss = 0;

double Time_SSDRead = 0.0;
double Time_SSDWrite = 0.0;
double Time_HDDRead = 0.0;
double Time_HDDWrite = 0.0;

int counts_SSDRead = 0;
int counts_SSDWrite = 0;
int counts_HDDRead = 0;
int counts_HDDWrite = 0;

int total_pages = 0;
double avg_response_time = 0.0;
double total_response_time = 0.0;

int c_LAZY = 0;
int c_LAZY_victim = 0;



/*DISKSIM INITIALIZATION*/
/**
 * [Disksim的初始化，利用兩個Process各自執行Disksim，作為SSDsim和HDDsim，
 *  接續MESSAGE QUEUE INITIALIZATION]
 */
void initDisksim() {
	pid_t procid;
	//Fork process to execute SSD simulator
	procid = fork();
	if (procid == 0) {
		SSDsimProc = getpid();
		//printf("SSDsimProc ID: %d\n", SSDsimProc);
		exec_SSDsim("SSDsim", par[1], par[2]);
		exit(0);
	}
	else if (procid < 0) {
		PrintError(-1, "SSDsim process fork() error");
		exit(1);
	}

	//Fork process to execute HDD simulator
	procid = fork();
	if (procid == 0) {
		HDDsimProc = getpid();
		//printf("HDDsimProc ID: %d\n", HDDsimProc);
		exec_HDDsim("HDDsim", par[3], par[4]);
		exit(0);
	}
	else if (procid < 0) {
		PrintError(-1, "HDDsim process fork() error");
		exit(1);
	}

	//After the initialization of simulators, initialize message queue
	initMSQ();
}

/*DISKSIM SHUTDOWN*/
/**
 * [Disksim的關閉，傳送Control message告知其Process進行Shutdown，並等待回傳結果message]
 */
double rmDisksim() {
	REQ *ctrl, *ctrl_rtn;
	ctrl = calloc(1, sizeof(REQ));
	ctrl_rtn = calloc(1, sizeof(REQ));      //Receive message after control message
	ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH; //Assign a finish flag (ipc)
	
	//Send a control message to finish SSD simulator
	sendFinishControl(KEY_MSQ_DISKSIM_SSD, MSG_TYPE_DISKSIM_SSD);

	//Receive the last message from SSD simulator
	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_SSD, ctrl_rtn, MSG_TYPE_DISKSIM_SSD_SERVED) == -1)
		PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
	//else
		//PrintREQ(ctrl_rtn ,"ycsim.c#63");
	double SSD_total_response_time = ctrl_rtn->responseTime;
	
	printf("[YCSIM]SSDsim response time = %lf\n", SSD_total_response_time);
	//fprintf(result, "[YCSIM]SSDsim response time = %lf\n", ctrl_rtn->responseTime);

	//Send a control message to finish HDD simulator
	sendFinishControl(KEY_MSQ_DISKSIM_HDD, MSG_TYPE_DISKSIM_HDD);

	//Receive the last message from HDD simulator
	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_HDD, ctrl_rtn, MSG_TYPE_DISKSIM_HDD_SERVED) == -1)
		PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
	//else
		//PrintREQ(ctrl_rtn ,"ycsim.c#75");

	double HDD_total_response_time = ctrl_rtn->responseTime;
	
	printf("[YCSIM]HDDsim response time = %lf\n", HDD_total_response_time);
	//fprintf(result, "[YCSIM]HDDsim response time = %lf\n", ctrl_rtn->responseTime);
	
	//After that, remove message queues
	rmMSQ();

	total_response_time = SSD_total_response_time + HDD_total_response_time;
	return total_response_time;
	
	
}

/*MESSAGE QUEUE INITIALIZATION*/
/**
 * [Message queue初始化，使用系統定義的Key值、Type和IPC function]
 */
void initMSQ() {
	//Create message queue for SSD simulator
	if(createMessageQueue(KEY_MSQ_DISKSIM_SSD, IPC_CREAT) == -1)
		PrintError(-1, " MSQ create error in createMessageQueue():");
	
	//Create message queue for HDD simulator
	if(createMessageQueue(KEY_MSQ_DISKSIM_HDD, IPC_CREAT) == -1)
		PrintError(-1, " MSQ create error in createMessageQueue():");
}

/*MESSAGE QUEUE REMOVE*/
/**
 * [Message queue刪除，使用系統定義的Key值和IPC function]
 */
void rmMSQ() {
	struct msqid_ds ds;
	//Remove message queue for SSD simulator
	if(removeMessageQueue(KEY_MSQ_DISKSIM_SSD, &ds) == -1)
		PrintError(KEY_MSQ_DISKSIM_SSD, "Not remove message queue:(key)");
	//Remove message queue for HDD simulator
	if(removeMessageQueue(KEY_MSQ_DISKSIM_HDD, &ds) == -1)
		PrintError(KEY_MSQ_DISKSIM_HDD, "Not remove message queue:(key)");
}


int main(int argc, char *argv[]) {
	//Check arguments
	if (argc != 8) {
		fprintf(stderr, "usage: %s <Trace file> <param file for SSD> <output file for SSD> <param file for HDD> <output file for HDD><output file for result><output file for statistics>\n", argv[0]);
		exit(1);
	}

	//Record arguments by variables
	par[0] = argv[1]; //trace
	par[1] = argv[2]; //SSD prav
	par[2] = argv[3]; //SSD output
	par[3] = argv[4]; //HDD prav
	par[4] = argv[5]; //HDD output
	par[5] = argv[6]; //r.txt
	par[6] = argv[7]; //s.txt

	//Open trace file
	trace = fopen(par[0], "r");
	if (!trace)
		PrintError(-1, "[YCSIM]trace file open error");

	//Open result file
	result = fopen(par[5], "w");

	//Open result file
	statistics = fopen(par[6], "w");

	//Initialize Disksim(SSD and HDD simulators)
	initDisksim();


	//PrintMissRatio_in_BLOCKS(MRB, "yc.c#init");
	fprintf(result, "BASIC+LAZY!!!!\n");	

	fprintf(result, "PageCacheSize : %d\n", PageCacheSize);
	fprintf(result, "SSDSize : %d\n", SSDSize);

	//From trace file, insert user requests into user queues
	REQ *input;
	input = calloc(1, sizeof(REQ));

	printf("[YCSIM]Trace loading...............\n");
	while(!feof(trace)) {
		//get every request form trace
		fscanf(trace, "%lf%u%lu%u%u", &input->arrivalTime, &input->devno, &input->diskBlkno, &input->reqSize, &input->reqFlag);
		
		//DEBUG:
		//PrintREQ(input, "Trace");
		//PrintREQinFile(input, "Trace");

		int page_count = input->reqSize / 8;//r->reqSize/SSD_PAGE2SECTOR;
		total_pages += page_count;

		printf("\ntotal_pages = %d\n", total_pages);

		int i = 0;
		for (i = 0; i < page_count; i++)
		{			
			int page_no = (input -> diskBlkno + i*8) / SSD_PAGE2SECTOR;
			int flag = input -> reqFlag;

			int isPCHit = CheckPageCache(page_no, flag);
			if(isPCHit == 1)
			{
				//fprintf(result, "hit in Page Cache!\n");		
				PCHit++;		
			}
			else//isPCHit == 0
			{
				PCMiss++;
				
				if(flag == 1)//PC Read miss
				{
					printf("read miss in Page Cache!\n");
					fprintf(result, "Read miss in Page Cache!\n");					

					if(CheckSSDTable(page_no, flag) == 1)//3. Read Hit in SSD
					{
						SSD_R_Hit++;

						counts_SSDRead++;
						Time_SSDRead = sendRequest2SSD_to_getServiceTime(page_no, flag);
						fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);

						fprintf(result, "Read miss in Page Cache but hit in SSD!\n");
					
						if(insertPageCache(page_no, flag) == 1)//PageCache is full
						{
							PCnode  *e;
							e = malloc(sizeof(PCnode));
							e = FindingPageCachevictim();							
							
							if(e -> LAZY == 1)
							{																			
								fprintf(result, "LAZY page need to write back to SSD\n");
								c_LAZY_victim++;

								counts_SSDWrite++;
								Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, 0);
								fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);							
													
								if(insertSSD(e -> page_no, e -> flag) == 1)
								{
									fprintf(result, "SSD is full!\n");
									SSDnode *e_SSD;
									e_SSD = malloc(sizeof(SSDnode));
									e_SSD = FindingSSDvictim();								
									
									if(e_SSD -> flag == 0)//SSD last is dirty
									{
										printf("SSDlast is dirty!\n");
										fprintf(result, "SSD ecivt dirty victim!\n");

										counts_SSDRead++;
										Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
										fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
										
										counts_HDDWrite++;
										Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
										fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
									}
									else
										fprintf(result, "SSD ecivt clean victim!\n");
									//evciton SSDlast
									Evict_SSDvictim(e_SSD);	
								}									
							}	
							else if(e -> flag == 0)
							{
								fprintf(result, "Page Cache evict dirty victim!\n");

								//dirty victim hit SSD or not
								if(CheckSSDTable(e->page_no, e->flag) == 0)
								{
									//dirty victim miss in SSD
									SSD_W_Miss++;
									//check GB for dirty victim is hit or not
									if(checkGHOSTbuffer(e -> page_no, e ->flag) == 1)
									{
										fprintf(result, "Dirty victim page is hit in GB!\n");
										//dirty eviction hit in GB, so write the eviction in SSD
							
										counts_SSDWrite++;
										Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
										fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);
										if(insertSSD(e -> page_no, e ->flag) == 1)
										{
											fprintf(result, "SSD is full!\n");
											SSDnode  *e_SSD;
											e_SSD = malloc(sizeof(SSDnode));
											e_SSD = FindingSSDvictim();

											if(e_SSD -> flag == 0)//SSD last is dirty
											{
												printf("SSD last is dirty!\n");
												fprintf(result, "SSD ecivt dirty victim!\n");

												counts_SSDRead++;
												Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
												fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
												
												counts_HDDWrite++;
												Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
												fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
											}
											else
												fprintf(result, "SSD ecivt clean victim!\n");
											//evciton SSDlast
											Evict_SSDvictim(e_SSD);																			
										}
									}
									else
									{
										fprintf(result, "Dirty victim page is miss in GB!\n");
										fprintf(result, "Victim page insert to GB!\n");
										insertGHOSTbuffer(e -> page_no, e ->flag);													

										//dirty victim write in HDD
										counts_HDDWrite++;
										Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e -> page_no, e -> flag);
										fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);										
									}
								}
								else
								{
									//dirty victim hit in SSD
									SSD_W_Hit++;

									counts_SSDWrite++;
									Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
									fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);											
								}
							}
							else
							{
								//do nothing
							}														
							Evict_PageCachevictim(e);						
						}												
					}
					else//4. Read Miss in SSD
					{
						SSD_R_Miss++;

						fprintf(result, "read miss in Page Cache and miss in SSD too!\n");
						
						counts_HDDRead++;
						Time_HDDRead = sendRequest2HDD_to_getServiceTime(page_no, flag);
						//fprintf(result, "Time_HDDRead = %lf\n", Time_HDDRead);
												
						//check the page is in GB or not
						if(checkGHOSTbuffer(page_no, flag) == 1)//Hit in GB, has accessed
						{
							fprintf(result, "Missing page is hit in GB need to inserted to SSD!\n");

							//LAZY caching
							/*	
							counts_SSDWrite++;
							Time_SSDWrite = sendRequest2SSD_to_getServiceTime(page_no, 0);
							fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);							
												
							if(insertSSD(page_no, flag) == 1)
							{
								fprintf(result, "SSD is full!\n");
								SSDnode *e_SSD;
								e_SSD = malloc(sizeof(SSDnode));
								e_SSD = FindingSSDvictim();								
								
								if(e_SSD -> flag == 0)//SSD last is dirty
								{
									printf("SSDlast is dirty!\n");
									fprintf(result, "SSD ecivt dirty victim!\n");

									counts_SSDRead++;
									Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
									fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
									
									counts_HDDWrite++;
									Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
									//fprintf(result, "Time_HDDWrite = %lf\n", Time_HDDWrite);
								}
								else
									fprintf(result, "SSD ecivt clean victim!\n");
								//evciton SSDlast
								Evict_SSDvictim(e_SSD);	
							}
							*/

							flag = 2;
							c_LAZY++;
							if(insertPageCache(page_no, flag) == 1)
							{
								PCnode  *e;
								e = malloc(sizeof(PCnode));
								e = FindingPageCachevictim();							
								
								if(e -> LAZY == 1)
								{																			
									fprintf(result, "LAZY page need to write back to SSD\n");
									c_LAZY_victim++;

									counts_SSDWrite++;
									Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, 0);
									fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);							
														
									if(insertSSD(e -> page_no, e -> flag) == 1)
									{
										fprintf(result, "SSD is full!\n");
										SSDnode *e_SSD;
										e_SSD = malloc(sizeof(SSDnode));
										e_SSD = FindingSSDvictim();								
										
										if(e_SSD -> flag == 0)//SSD last is dirty
										{
											printf("SSDlast is dirty!\n");
											fprintf(result, "SSD ecivt dirty victim!\n");

											counts_SSDRead++;
											Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
											fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
											
											counts_HDDWrite++;
											Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
											fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
										}
										else
											fprintf(result, "SSD ecivt clean victim!\n");
										//evciton SSDlast
										Evict_SSDvictim(e_SSD);	
									}									
								}	
								else if(e -> flag == 0)
								{
									fprintf(result, "Page Cache evict dirty victim!\n");

									//dirty victim hit SSD or not
									if(CheckSSDTable(e->page_no, e->flag) == 0)
									{
										//dirty victim miss in SSD
										SSD_W_Miss++;
										//check GB for dirty victim is hit or not
										if(checkGHOSTbuffer(e -> page_no, e ->flag) == 1)
										{
											fprintf(result, "Dirty victim page is hit in GB!\n");
											//dirty eviction hit in GB, so write the eviction in SSD
								
											counts_SSDWrite++;
											Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
											//fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);
											if(insertSSD(e -> page_no, e ->flag) == 1)
											{
												fprintf(result, "SSD is full!\n");
												SSDnode  *e_SSD;
												e_SSD = malloc(sizeof(SSDnode));
												e_SSD = FindingSSDvictim();

												if(e_SSD -> flag == 0)//SSD last is dirty
												{
													printf("SSD last is dirty!\n");
													fprintf(result, "SSD ecivt dirty victim!\n");

													counts_SSDRead++;
													Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
													fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
													
													counts_HDDWrite++;
													Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
													fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
												}
												else
													fprintf(result, "SSD ecivt clean victim!\n");
												//evciton SSDlast
												Evict_SSDvictim(e_SSD);																			
											}
										}
										else
										{
											fprintf(result, "Dirty victim page is miss in GB!\n");
											fprintf(result, "Victim page insert to GB!\n");
											insertGHOSTbuffer(e -> page_no, e ->flag);													

											//dirty victim write in HDD
											counts_HDDWrite++;
											Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e -> page_no, e -> flag);
											fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);									
										}
									}
									else
									{
										//dirty victim hit in SSD
										SSD_W_Hit++;

										counts_SSDWrite++;
										Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
										//fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);											
									}
								}
								else
								{
									//do nothing
								}														
								Evict_PageCachevictim(e);									
							}	
						}	
						else//miss in GB, first access in the period
						{
							fprintf(result, "Missing page miss in GB need to inserted GHOSTbuffer!\n");
							insertGHOSTbuffer(page_no, flag);	

							if(insertPageCache(page_no, flag) == 1)
							{						
								PCnode *e;
								e = malloc(sizeof(PCnode));
								e = FindingPageCachevictim();							
								
								if(e -> LAZY == 1)
								{																			
									fprintf(result, "LAZY page need to write back to SSD\n");
									c_LAZY_victim++;

									counts_SSDWrite++;
									Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, 0);
									fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);							
														
									if(insertSSD(e -> page_no, e -> flag) == 1)
									{
										fprintf(result, "SSD is full!\n");
										SSDnode *e_SSD;
										e_SSD = malloc(sizeof(SSDnode));
										e_SSD = FindingSSDvictim();								
										
										if(e_SSD -> flag == 0)//SSD last is dirty
										{
											printf("SSDlast is dirty!\n");
											fprintf(result, "SSD ecivt dirty victim!\n");

											counts_SSDRead++;
											Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
											fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
											
											counts_HDDWrite++;
											Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
											fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
										}
										else
											fprintf(result, "SSD ecivt clean victim!\n");
										//evciton SSDlast
										Evict_SSDvictim(e_SSD);	
									}									
								}	
								else if(e -> flag == 0)
								{
									fprintf(result, "Page Cache evict dirty victim!\n");

									//dirty victim hit SSD or not
									if(CheckSSDTable(e->page_no, e->flag) == 0)
									{
										//dirty victim miss in SSD
										SSD_W_Miss++;
										//check GB for dirty victim is hit or not
										if(checkGHOSTbuffer(e -> page_no, e ->flag) == 1)
										{
											fprintf(result, "Dirty victim page is hit in GB!\n");
											//dirty eviction hit in GB, so write the eviction in SSD
								
											counts_SSDWrite++;
											Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
											//fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);
											if(insertSSD(e -> page_no, e ->flag) == 1)
											{
												fprintf(result, "SSD is full!\n");
												SSDnode  *e_SSD;
												e_SSD = malloc(sizeof(SSDnode));
												e_SSD = FindingSSDvictim();

												if(e_SSD -> flag == 0)//SSD last is dirty
												{
													printf("SSD last is dirty!\n");
													fprintf(result, "SSD ecivt dirty victim!\n");

													counts_SSDRead++;
													Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
													fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
													
													counts_HDDWrite++;
													Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
													fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
												}
												else
													fprintf(result, "SSD ecivt clean victim!\n");
												//evciton SSDlast
												Evict_SSDvictim(e_SSD);																			
											}
										}
										else
										{
											fprintf(result, "Dirty victim page is miss in GB!\n");
											fprintf(result, "Victim page insert to GB!\n");
											insertGHOSTbuffer(e -> page_no, e ->flag);													

											//dirty victim write in HDD
											counts_HDDWrite++;
											Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e -> page_no, e -> flag);
											fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);										
										}
									}
									else
									{
										//dirty victim hit in SSD
										SSD_W_Hit++;

										counts_SSDWrite++;
										Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
										//fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);											
									}
								}
								else
								{
									//do nothing
								}														
								Evict_PageCachevictim(e);								
							}
						}		
					}
				}
				else//PC Write miss 
				{
					printf("Write miss in Page Cache!\n");
					fprintf(result, "Write miss in Page Cache!\n");				
					
					fprintf(result, "insert into PageCache!\n");						
			
					if(insertPageCache(page_no, flag) == 1)
					{					
						PCnode *e;
						e = malloc(sizeof(PCnode));
						e = FindingPageCachevictim();						
				
						if(e -> LAZY == 1)
						{
							fprintf(result, "LAZY page need to write back to SSD\n");
							c_LAZY_victim++;
							
							counts_SSDWrite++;
							Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, 0);
							fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);							
												
							if(insertSSD(e -> page_no, e -> flag) == 1)
							{
								fprintf(result, "SSD is full!\n");
								SSDnode *e_SSD;
								e_SSD = malloc(sizeof(SSDnode));
								e_SSD = FindingSSDvictim();								
								
								if(e_SSD -> flag == 0)//SSD last is dirty
								{
									printf("SSDlast is dirty!\n");
									fprintf(result, "SSD ecivt dirty victim!\n");

									counts_SSDRead++;
									Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
									fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
									
									counts_HDDWrite++;
									Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
									fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
								}
								else
									fprintf(result, "SSD ecivt clean victim!\n");
								//evciton SSDlast
								Evict_SSDvictim(e_SSD);	
							}									
						}	
						else if(e -> flag == 0)
						{
							fprintf(result, "Page Cache evict dirty victim!\n");

							//dirty victim hit SSD or not
							if(CheckSSDTable(e->page_no, e->flag) == 0)
							{
								//dirty victim miss in SSD
								SSD_W_Miss++;
								//check GB for dirty victim is hit or not
								if(checkGHOSTbuffer(e -> page_no, e ->flag) == 1)
								{
									fprintf(result, "Dirty victim page is hit in GB!\n");
									//dirty eviction hit in GB, so write the eviction in SSD
						
									counts_SSDWrite++;
									Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
									//fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);
									if(insertSSD(e -> page_no, e ->flag) == 1)
									{
										fprintf(result, "SSD is full!\n");
										SSDnode  *e_SSD;
										e_SSD = malloc(sizeof(SSDnode));
										e_SSD = FindingSSDvictim();

										if(e_SSD -> flag == 0)//SSD last is dirty
										{
											printf("SSD last is dirty!\n");
											fprintf(result, "SSD ecivt dirty victim!\n");

											counts_SSDRead++;
											Time_SSDRead = sendRequest2SSD_to_getServiceTime(e_SSD -> page_no, 1);
											fprintf(result, "Time_SSDRead = %lf\n", Time_SSDRead);
											
											counts_HDDWrite++;
											Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e_SSD -> page_no, e_SSD -> flag);
											fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);
										}
										else
											fprintf(result, "SSD ecivt clean victim!\n");
										//evciton SSDlast
										Evict_SSDvictim(e_SSD);																			
									}
								}
								else
								{
									fprintf(result, "Dirty victim page is miss in GB!\n");
									fprintf(result, "Victim page insert to GB!\n");
									insertGHOSTbuffer(e -> page_no, e ->flag);													

									//dirty victim write in HDD
									counts_HDDWrite++;
									Time_HDDWrite = sendRequest2HDD_to_getServiceTime(e -> page_no, e -> flag);
									fprintf(result, "Time_HDDWrite = %lf, counts_HDDWrite = %d\n", Time_HDDWrite, counts_HDDWrite);									
								}
							}
							else
							{
								//dirty victim hit in SSD
								SSD_W_Hit++;

								counts_SSDWrite++;
								Time_SSDWrite = sendRequest2SSD_to_getServiceTime(e -> page_no, e -> flag);
								//fprintf(result, "Time_SSDWrite = %lf\n", (double)Time_SSDWrite);											
							}
						}
						else
						{
							//do nothing
						}														
						Evict_PageCachevictim(e);
					}	
				}

			}//end of is_PC_Hit == 0

		}//end of page_counts

	}//end of while read trace		
	/*
	displayPageCacheTable();
	displayGHOSTbuffer();
	displaySSDTable();

	*/

	printf("[Trace Reading Finish-----Basic+LAZY]\n\n");

	printf("PageCacheSize : %d\n", PageCacheSize);
	printf("SSDSize : %d\n\n", SSDSize);

	double PC_Hit_Ratio = (double)PCHit / (PCHit + PCMiss);
	printf("PC_Hit_Ratio = %lf\n", PC_Hit_Ratio);

	printf("\ncounts_SSDRead = %d\n", counts_SSDRead);
	printf("SSD_R_Hit = %d\n", SSD_R_Hit);
	printf("SSD_R_Miss = %d\n", SSD_R_Miss);
	printf("counts_SSDWrite = %d\n", counts_SSDWrite);
	printf("SSD_W_Hit = %d\n", SSD_W_Hit);	
	printf("SSD_W_Miss = %d\n", SSD_W_Miss);	

	double SSD_Hit_Ratio = (double)(SSD_R_Hit + SSD_W_Hit) / (SSD_R_Hit + SSD_W_Hit + SSD_R_Miss + SSD_W_Miss);
	printf("SSD_Hit_Ratio = %lf\n", SSD_Hit_Ratio);	

	printf("\ncounts_HDDRead = %d\n", counts_HDDRead);
	printf("counts_HDDWrite = %d\n", counts_HDDWrite);

	printf("c_LAZY = %d\n", c_LAZY);
	printf("c_LAZY_victim = %d\n", c_LAZY_victim);
	
	//Remove Disksim(SSD and HDD simulators)
	total_response_time = rmDisksim();
	avg_response_time = (total_response_time) / (double)total_pages;
	printf("avg_response_time = %lf\n", avg_response_time);

	int total_NODE_counts = get_NODE_counts();
	printf("total_NODE_counts = %d\n", total_NODE_counts);
	//Total requests output
	//printf("[YCSIM]Receive requests:%lu\n", getTotalReqs());
	
	//Statistics output in Prize caching (prize)
	//pcStatistics();

	//User statistics output
	//printUSERSTAT(scheduleTime);

	//Result file
	//fprintf(result, "[YUSIM]Receive requests:%lu\n", getTotalReqs());
	//CACHEWriteResultFile(&result);
	//pcWriteResultFile(&result);
	//writeResultFile(&result, scheduleTime);

	// Waiting for SSDsim and HDDsim process
	wait(NULL);
	wait(NULL);
	//OR
	//printf("Main Process waits for: %d\n", wait(NULL));
	//printf("Main Process waits for: %d\n", wait(NULL));

	//Release files items
	fclose(trace);
	fclose(statistics);
	fclose(result);

	exit(0);

}
