/*LRU+CFLRU(PageCache) and LARC(SSD)*/
#include <stdlib.h>
#include <stdio.h> 

#include "yc_caching.h"

int T1ReadHit[PageCacheSize+1] = {0} , T2ReadHit[PageCacheSize+1] = {0} ;
int T1WriteHit[PageCacheSize+1] = {0} , T2WriteHit[PageCacheSize+1]  = {0} ;

int Old_GBSize = 0;
int New_GBSize = 0;

PCnode *searchPageCachequeue(unsigned long page_no, int flag)//
{
	printf("[searchPageCache]want to search in PageCache : %lu\n",  page_no);
	PCnode *searchPC;
	searchPC = malloc(sizeof(PCnode));

	searchPC = PChead;
	int p = 0;
	while(searchPC != NULL)
	{
		//fprintf(result, "searchPC : %lu, c = %d\n", search -> page_no, p);
		//printf("searchPC : %lu, c = %d\n", search -> page_no, p);
		if(searchPC -> page_no == page_no)  
		{
			printf("Hit in PC\n");
			return searchPC;		
		}
		else
		{
			searchPC = searchPC -> PC_next;
			p++;
		}
	}
	printf("miss in PC\n");
	return NULL;	
}

SSDnode *searchSSD(unsigned long page_no)//
{
	printf("[search SSD]want to search in SSD : %lu\n",  page_no);
	SSDnode *search;
	search = malloc(sizeof(SSDnode));

	int c = 0;
	search = SSDhead;
	while(search != NULL)
	{
		//fprintf(result, "searchSSD : %lu, c = %d\n", search->page_no, c);
		if(search -> page_no == page_no)
		{
			printf("Hit in SSD\n");
			return search;		
		}
		else
		{
			search = search -> SSD_next;
			c++;
		}
	}
	printf("miss in SSD\n");
	return NULL;
}

GHOSTbuffernode *searchGHOSTbuffer(unsigned long page_no)//
{
	printf("[search GB]want to search in GB : %lu\n",  page_no);

	GHOSTbuffernode *search;
	search = malloc(sizeof(GHOSTbuffernode));

	int c = 0;
	search = Ghead;
	while(search != NULL)
	{
		//if( c = 0 || c > GHOSTbufferUsed - 5 )
			//fprintf(result, "searchGB : %lu, c = %d\n", search->page_no,c);
		if(search -> page_no == page_no)
		{
			printf("Hit in GB\n");
			return search;
		} 					
		else
		{
			search = search -> ghost_next;
			c++;
		}				
	}
	//printf("miss in GB\n");
	return NULL;
}

int CheckSSDTable(unsigned long page_no, int flag)
{
	printf("[CheckSSDTable] %lu\n",  page_no);
	
	SSDnode *search;
	search = malloc(sizeof(SSDnode));
	search = searchSSD(page_no);

	/*
	if(search  == NULL)
		printf("[CheckSSDTable]search result: NULL\n");
	else
		printf("[CheckSSDTable]search result: %lu\n", search -> page_no);
	*/

	if(search == NULL)//the page miss in SSD
	{
		GHOSTsize1 = 0.9 * SSDSize;
		GHOSTsize2 = GHOSTbufferSize + ( SSDSize / GHOSTbufferSize );
	
		Old_GBSize = GHOSTbufferSize;
		GHOSTbufferSize = min(GHOSTsize1, GHOSTsize2);
		New_GBSize = GHOSTbufferSize;

		if(Old_GBSize != New_GBSize)
			fprintf(result, "[Adjust GBSize]GHOSTbufferSize = %d\n", GHOSTbufferSize);

		/*
		printf(COLOR_MAGENTA"[B is not in Q]Adjust the GHOSTbufferSize\n"COLOR_RESET);
		printf(COLOR_MAGENTA"GHOSTbufferSize = %d\n", GHOSTbufferSize);
		printf(COLOR_RESET);
		*/
		
		return 0;	
	}
	else//the page hit in SSD
	{

		//fprintf(result, "[caching.c#121]Hit in SSD Table\n");

		//Move to MRU of SSD
		//printf(COLOR_YELLOW"Move the page to MRU of SSD\n"COLOR_RESET);
		if(search->SSD_pre != NULL)//not the first node
		{
			if(search->SSD_next == NULL)//hit the last node
			{
				printf("Hit in SSD last!\n");
				SSDlast = SSDlast -> SSD_pre;
				SSDlast -> SSD_next = NULL;		

				fprintf(result, "[After set]SSDlast -> page_no = %lu\n", SSDlast ->page_no);											
			}
			else//hit in the middle node
			{
				printf("Hit in SSD middle!\n");						
				search->SSD_pre->SSD_next = search->SSD_next;
				search->SSD_next->SSD_pre = search->SSD_pre;				
			}
			SSDhead->SSD_pre = search;
			search->SSD_next = SSDhead;						
			search->SSD_pre=NULL;
			SSDhead = SSDhead->SSD_pre;
		}

		//printf(COLOR_MAGENTA"[B is in Q]Adjust the Size of GHOSTbuffer!!!\n"COLOR_RESET);

		GHOSTsize1 = 0.1 * SSDSize;
		GHOSTsize2 = GHOSTbufferSize - ( SSDSize / ( SSDSize - GHOSTbufferSize ));

		Old_GBSize = GHOSTbufferSize;
		GHOSTbufferSize = max(GHOSTsize1, GHOSTsize2);
		New_GBSize = GHOSTbufferSize;

		if(New_GBSize < Old_GBSize)
		{
			printf("New_GBSize(%d) < Old_GBSize(%d)\n", New_GBSize, Old_GBSize);
			Adjust_Glast(New_GBSize);
		}
		
		//printf(COLOR_MAGENTA"GHOSTbufferSize = %d\n", GHOSTbufferSize);
		//printf(COLOR_RESET);
		
		return 1;
	}
}

void Adjust_Glast(int New_GBSize)
{	
	//fprintf(result, "Adjust_Glast\n");
	//fprintf(result, "=======================================\n");
	//fprintf(result, "New_GBSize = %d\n", New_GBSize);
	//fprintf(result, "GHOSTbufferSize = %d\n", GHOSTbufferSize);
	//fprintf(result, "GHOSTbufferUsed = %d\n", GHOSTbufferUsed);

	if(GHOSTbufferUsed > New_GBSize)
	{
		fprintf(result, "GHOSTbufferUsed > New_GBSize\n");

		GHOSTbuffernode *tmp;
		tmp = malloc(sizeof(GHOSTbuffernode));

		tmp = Ghead;
		int p = 1;
		while(tmp != NULL)
		{
			if(p == New_GBSize)  
			{
				//fprintf(result, "p(%d) == New_GBSize(%d)\n", p, New_GBSize);
				
				//fprintf(result, "tmp = %lu, p = %d\n", tmp->page_no, p);
				
				Glast = tmp;
				Glast -> ghost_pre = tmp -> ghost_pre;
				Glast -> ghost_next = NULL;			

				//fprintf(result, "Glast-> pre = %lu, p = %d\n", Glast->ghost_pre->page_no, p-1);
				//fprintf(result, "[After set]Glast = %lu, p = %d\n", Glast->page_no, p);

				if(Glast -> ghost_next == NULL)
					fprintf(result, "Glast -> ghost_next == NULL\n");
				else
					printf("caching.c#176 error!\n");

				GHOSTbufferUsed = New_GBSize;
				fprintf(result, "GHOSTbufferUsed = %d\n", GHOSTbufferUsed);
				
				//displayGHOSTbuffer();

				break;	
			}
			else
			{
				tmp = tmp -> ghost_next;
				p++;
			}
		}
	}
	
	//fprintf(result, "=======================================\n");
		
	
}
int checkGHOSTbuffer(unsigned long page_no, int flag)
{
	printf("[checkGHOSTbuffer] %lu\n",  page_no);
	
	//fprintf(result, "checkGHOSTbuffer\n");
	//check if exist in GHOSTbuffer or not
	GHOSTbuffernode *search;
	search = malloc(sizeof(GHOSTbuffernode));
	search = searchGHOSTbuffer(page_no);

	if( search == NULL )//[B is not in Qr]metadata put in MRU of ghost buffer
	{	
		printf("Miss in GHOSTbuffer\n");
		//fprintf(result, "Miss in GHOSTbuffer\n");
		return  0;
	}
	else//[B is in Qr]find in ghost buffer, means the page is be access twice//1.get rid of form ghost buffer//2.put in SSD
	{
		
		/*printf(COLOR_MAGENTA"===========================================\n"COLOR_RESET);
		printf(COLOR_MAGENTA"hit in GHOST buffer, so remove it from GHOST buffer!\n"COLOR_RESET);
		printf(COLOR_MAGENTA"===========================================\n"COLOR_RESET);
		*/
		printf("[check256]Ghead = %lu\n", Ghead -> page_no);
		printf("[check257]Glast = %lu\n", Glast -> page_no);
		
		//evcit the node in ghost buffer queue
		if(search->ghost_pre != NULL)//not the first node
		{
			if(search->ghost_next == NULL)//hit the last node
			{
				printf("hit the GB last node\n");
				fprintf(result, "Hit in GHOSTbuffer last & remove the page\n");
				Glast = Glast -> ghost_pre;	
				Glast -> ghost_next = NULL;
					
			}
			else//hit in the middle node
			{
				printf("hit the GB middle node\n");
				fprintf(result, "Hit in GHOSTbuffer middle & remove the page\n");

				search->ghost_pre->ghost_next = search->ghost_next;
				search->ghost_next->ghost_pre = search->ghost_pre;
			}
		}
		else//hit in first node of ghost buffer
		{
			printf("hit in GB head\n");
			Ghead = Ghead -> ghost_next;
			Ghead -> ghost_pre = NULL;
		}
		//fprintf(result, "Hit in GHOSTbuffer\n");
		GHOSTbufferUsed--;
		
		//fprintf(result, "Ghead = %lu\n", Ghead -> page_no);
		//fprintf(result, "Glast = %lu\n", Glast -> page_no);

		return 1;	
	}

}

int t_pages = 0;
int CheckPageCache(unsigned long page_no, int flag)
{
	PCnode *searchPC;
	searchPC = malloc(sizeof(PCnode));

	printf(COLOR_BLUE"A NEW PAGE COMES!!! page_no: %lu, flag = %d\n"COLOR_RESET, page_no, flag);
	t_pages++;
	
	fprintf(result, "\n");
	//fprintf(result, "++++++++++++++++++++++++++++++\n");
	fprintf(result, "A NEW PAGE COMES!!! %d.page_no: %lu, flag = %d\n", t_pages, page_no, flag);
			
	searchPC = searchPageCachequeue(page_no, flag);

	if(searchPC == NULL)//coming page miss in PageCache 
	{
		return 0;
	}		
	else
	{
		//Hit in PageCache
		//if request page is write, change flag to dirty
		switch(flag)
		{
			case CLEAN_PAGE:				
				break;
			case DIRTY_PAGE:				
				searchPC -> flag = flag;
				break;
			default:
				printf(COLOR_RED"Caching Error\n"COLOR_RESET);
		}

		if(searchPC -> PC_pre != NULL)//not the first node
		{
			
			if(searchPC -> PC_next == NULL)//hit the last node
			{
				fprintf(result, "Hit in PC last!\n");
				PClast = searchPC -> PC_pre;
				PClast -> PC_next = NULL;	
			}
			else//hit in the middle node
			{
				fprintf(result, "Hit in PC middle!\n");
				searchPC -> PC_pre -> PC_next = searchPC -> PC_next;
				searchPC -> PC_next -> PC_pre = searchPC -> PC_pre;				
			}
			PChead -> PC_pre = searchPC;
			searchPC -> PC_next = PChead;						
			searchPC -> PC_pre = NULL;
			PChead = PChead ->PC_pre;

			//printf("[AdjustPageCache]PChead = %lu\n", PChead -> page_no);
			//printf("[AdjustPageCache]PClast = %lu\n", PClast -> page_no);	
		}

		//fprintf(result, "[check336]Ghead = %lu\n", Ghead -> page_no);
		//fprintf(result, "[check336]Glast = %lu\n", Glast -> page_no);



		return 1;
	}

	
}		

int insertPageCache(unsigned long page_no, int flag)
{
	fprintf(result, "[insertPageCache] %lu\n",  page_no);

	c_PC_NODE++;
	//printf("c_PC_NODE = %d\n", c_PC_NODE);

	if(PChead == NULL)
	{
		PChead = malloc(sizeof(PCnode));

		PChead -> page_no = page_no;
		if(flag == 2)
		{
			PChead -> flag = 1;
			PChead -> LAZY = 1;

			fprintf(result, "[check flag = 2]PChead->page_no = %lu, PChead->flag = %d, PChead->LAZY = %d\n", PChead->page_no, PChead->flag, PChead->LAZY);
		}
		else
		{
			PChead -> flag = flag;
			PChead -> LAZY = 0;
		}

		//printf("PChead = %lu\n", PChead -> page_no);

		//printf(COLOR_YELLOW"After set the PChead = %lu\n", PChead -> page_no);
		//printf(COLOR_RESET);

		PChead -> PC_next = NULL;
		PChead -> PC_pre = NULL;
		
		PClast = malloc(sizeof(PCnode));
		PClast = PChead;
		
		//printf("PClast = %lu\n", PClast -> page_no);

		PCUsed++;
		fprintf(result, "PCUsed = %d\n", PCUsed);
	}	
	else
	{
		//put this node in MRU of SSD list
		PCnode *tmp;
		tmp = malloc(sizeof(PCnode));

		tmp -> page_no = page_no;
		if(flag == 2)
		{
			tmp -> flag = 1;
			tmp -> LAZY = 1;

			fprintf(result, "[check flag = 2]page_no = %lu, flag = %d, LAZY = %d\n", tmp->page_no, tmp->flag, tmp->LAZY);
		}
		else
		{
			tmp -> flag = flag;
			tmp -> LAZY = 0;
		}


		PChead -> PC_pre = tmp;
		tmp -> PC_next = PChead;
		tmp -> PC_pre = NULL;
		PChead = PChead -> PC_pre;

		PCUsed++;	
		//fprintf(result, "PCUsed = %d\n", PCUsed);

		//fprintf(result, "After insertPageCache : PChead = %lu\n", PChead -> page_no);
		//fprintf(result, "After insertPageCache : PClast = %lu\n", PClast -> page_no);

		printf("[insertPageCache]PChead = %lu\n", PChead -> page_no);	
		printf("[insertPageCache]PClast = %lu\n", PClast -> page_no);	

		if( PCUsed > PageCacheSize )
		{
			//evict the last node in queue
			fprintf(result, "[PageCache is full](PageCacheSize = %d)\n", PageCacheSize);	
			return 1;			
		}			
	}
		
}

int insertGHOSTbuffer(unsigned long page_no, int flag)
{
	printf("[insertGHOSTbuffer] %lu\n", page_no);
	
	c_GB_NODE++;
	//printf("c_GB_NODE = %d\n", c_GB_NODE);

	if(Ghead == NULL)
	{
		printf("Ghead is NULL \n");

		Ghead = malloc(sizeof(GHOSTbuffernode));
		Ghead -> page_no = page_no;
		printf("Ghead = %lu\n", Ghead -> page_no);
		Ghead -> ghost_next = NULL;
		Ghead -> ghost_pre = NULL;
		
		Glast = malloc(sizeof(GHOSTbuffernode));
		Glast = Ghead;
		printf("Glast = %lu\n", Glast -> page_no);
	}
	else
	{
		//check if out of GHOSTbuffer Size
		if( GHOSTbufferUsed >= GHOSTbufferSize )
		{
			printf("GB is full\n");

			fprintf(result, "GB is full\n");
			
			//evict the last node in queue
			Glast = Glast -> ghost_pre;
			Glast -> ghost_next = NULL;		

			//fprintf(result, "[After set]Ghead = %lu\n", Ghead -> page_no);
			//fprintf(result, "[After set]Glast = %lu\n", Glast -> page_no);

			GHOSTbufferUsed--;
			fprintf(result, "GHOSTbufferUsed = %d\n", GHOSTbufferUsed);
		}

		//put this node in MRU of GHOSTbuffer list
		GHOSTbuffernode *tmp;
		tmp = malloc(sizeof(GHOSTbuffernode));

		tmp -> page_no = page_no;

		Ghead->ghost_pre = tmp;
		tmp->ghost_next = Ghead;
		tmp->ghost_pre = NULL;
		Ghead = Ghead->ghost_pre;	

		//fprintf(result, "[After insert GB]Ghead = %lu\n", Ghead -> page_no);
		//fprintf(result, "[After insert GB]Glast = %lu\n", Glast -> page_no);	
			
	}

	GHOSTbufferUsed++;
	fprintf(result, "GHOSTbufferUsed = %d(/%d)\n", GHOSTbufferUsed, GHOSTbufferSize);

	//displayGHOSTbuffer();	

}
int insertSSD(unsigned long page_no, int flag)
{
	printf("[insertSSD] %lu, flag= %d\n",  page_no, flag);

	c_SSD_NODE++;
	printf("c_SSD_NODE = %d\n", c_SSD_NODE);
	
	//Only insert the page in SSD MRU end

	if(SSDhead == NULL)
	{
		//printf(COLOR_YELLOW"SSDhead is NULL \n"COLOR_RESET);
		SSDhead = malloc(sizeof(SSDnode));
		SSDhead -> page_no = page_no;
		SSDhead -> flag = flag;
		if(flag == 2)
			SSDhead -> flag = 1;
		
		//printf(COLOR_YELLOW"After set the SSDhead = %lu\n", SSDhead -> page_no);
		//printf(COLOR_RESET);

		SSDhead -> SSD_next = NULL;
		SSDhead -> SSD_pre = NULL;
		
		SSDlast = malloc(sizeof(SSDnode));
		SSDlast = SSDhead;
		//printf("SSDlast = %lu\n", SSDlast -> page_no);

		SSDUsed++;
		fprintf(result, "SSDUsed = %d\n", SSDUsed);

		fprintf(result, "SSDhead -> page_no = %lu\n", SSDhead->page_no);
		fprintf(result, "SSDlast -> page_no = %lu\n", SSDlast->page_no);

	}
	else
	{
		//put this node in MRU of SSD list
		SSDnode  *tmp;
		tmp = malloc(sizeof(SSDnode));
		tmp -> flag = flag;
		tmp -> page_no = page_no;
		if(flag == 2)
			tmp -> flag = 1;
		
		//printf(COLOR_YELLOW"the node = %lu\n", tmp -> page_no);
		//printf(COLOR_RESET);


		SSDhead->SSD_pre = tmp;
		tmp->SSD_next = SSDhead;
		tmp->SSD_pre = NULL;
		SSDhead = SSDhead->SSD_pre;	

		SSDUsed++;
		fprintf(result, "SSDUsed = %d\n", SSDUsed);

		if( SSDUsed > SSDSize )
			return 1;	

		fprintf(result, "SSDhead -> page_no = %lu\n", SSDhead ->page_no);
		fprintf(result, "SSDlast -> page_no = %lu\n", SSDlast ->page_no);						
	}

}


PCnode *FindingPageCachevictim()
{
	//printf("[FindingPageCachevictim]\n");
	return PClast;
}

void *Evict_PageCachevictim(PCnode *eviction)
{
	//printf("[Evict_PageCachevictim] \n");
	//fprintf(result, "eviction = %lu\n", eviction -> page_no);

	PClast = eviction -> PC_pre;
	PClast -> PC_next = NULL;
	//printf("[T1 is full]After set : PClast = %lu\n", PClast -> page_no);

	//fprintf(result, "After eviction : PChead = %lu\n", PChead -> page_no);
	//fprintf(result, "After eviction : PClast = %lu\n", PClast -> page_no);

	PCUsed--;
	fprintf(result ,"PCUsed = %d\n", PCUsed);

}

SSDnode *FindingSSDvictim()
{
	//printf("[FindingSSDvictim]");

	return SSDlast;
}
void  *Evict_SSDvictim(SSDnode *eviction)
{
	//printf("[Evict_SSDvictim]\n");

	SSDlast = SSDlast -> SSD_pre;
	SSDlast -> SSD_next = NULL;
	//printf("SSDlast = %lu\n", SSDlast -> page_no);

	//printf("SSD victim is be evicted.\n");
	fprintf(result, "SSD victim is be evicted.\n");
	SSDUsed--;
	//printf("SSDUsed = %d\n", SSDUsed);	
	fprintf(result, "SSDUsed = %d\n", SSDUsed);
}

void displayPageCacheTable()
{
	int PC_c = 0;
	unsigned long diskBlkno;
	if (PChead != NULL)
	{
		PCnode *tmp;
		fprintf(result ,"<PageCache Table>\n");
		fprintf(result ,"PageCache Size = %d\n", PageCacheSize);
		fprintf(result ,"Flag 1 = Clean Page, Flag 0 = Dirty Page \n");
		fprintf(result ,"<MRU>=============================================<MRU>\n");
		
		for(tmp = PChead; tmp->PC_next != NULL; tmp = tmp -> PC_next)//tmp->T1_next != NULL
		{
			PC_c++;
			diskBlkno = (tmp -> page_no)*8;
			fprintf(result ,"%d, Page_no = %4lu\t <=> HDD Block Number = %4lu\t <=> Flag = %d\n",PC_c , tmp->page_no, diskBlkno, tmp->flag);
		}
		PC_c++;
		diskBlkno = (tmp -> page_no)*8;
		fprintf(result ,"%d, Page_no = %4lu\t <=> HDD Block Number = %4lu\t <=> Flag = %d\n",PC_c , tmp->page_no, diskBlkno, tmp->flag);
		fprintf(result ,"<LRU>=============================================<LRU>\n");
	}
	else
	{
		fprintf(result ,"No data in the T1 queue\n");
	}
}

void displayGHOSTbuffer()
{
	int GHOST_c = 0;
	if (Ghead != NULL)
	{
		GHOSTbuffernode *tmp;
		fprintf(result ,"<GHOST buffer Table>\n");
		fprintf(result ,"GHOSTbufferSize = %d\n", GHOSTbufferSize);
		fprintf(result ,"GHOSTbufferUsed = %d\n", GHOSTbufferUsed);
		fprintf(result ,"<MRU>=============================================<MRU>\n");
		
		
		for(tmp = Ghead; tmp->ghost_next != NULL ; tmp = tmp->ghost_next)
		{
			GHOST_c++;
			if(GHOST_c > GHOSTbufferUsed - 20)
				fprintf(result ,"%d, Page_no = %4lu\n",GHOST_c , tmp->page_no);
		}
		GHOST_c++;		
		fprintf(result ,"%d, Page_no = %4lu\n",GHOST_c , tmp->page_no);
		fprintf(result ,"<LRU>=============================================<LRU>\n");
	}
	else
	{
		fprintf(result ,"No data in the GHOST buffer\n");
	}
}
void displaySSDTable()
{
	int SSD_c = 0;
	if (SSDhead != NULL)
	{
		SSDnode *tmp;
		fprintf(result ,"<SSD Table>\n");
		fprintf(result ,"SSDSize = %d\n", SSDSize);
		fprintf(result ,"Flag 1 = Clean Page, Flag 0 = Dirty Page \n");
		fprintf(result ,"<MRU>=============================================<MRU>\n");
		
		
		for(tmp = SSDhead; tmp->SSD_next != NULL ; tmp = tmp->SSD_next)
		{
			SSD_c++;
			fprintf(result ,"%d, Page_no = %4lu\t <=> Flag = %d\n",SSD_c, tmp->page_no, tmp->flag);
		}
		SSD_c++;		
		fprintf(result ,"%d, Page_no = %4lu\t <=> Flag = %d\n",SSD_c, tmp->page_no, tmp->flag);
		fprintf(result ,"<LRU>=============================================<LRU>\n");
	}
	else
	{
		fprintf(result ,"No data in the SSD Table\n");
	}
}

int max(int Gs1, int Gs2)
{
	if(Gs1 > Gs2)
		return Gs1;
	else
		return Gs2;
}
int min(int Gs1, int Gs2)
{
	if(Gs1 < Gs2)
		return Gs1;
	else
		return Gs2;
}

REQ *PageReqMaker(unsigned long page_no, int flag)
{
	REQ *PageReq;
	PageReq = malloc(sizeof(REQ));

	PageReq -> diskBlkno = page_no*SSD_PAGE2SECTOR;
	PageReq -> reqFlag = flag;
	PageReq -> reqSize = 8;

	return PageReq;
}

int get_NODE_counts()
{	
	printf("c_PC_NODE = %d\n", c_PC_NODE);
	printf("c_GB_NODE = %d\n", c_GB_NODE);
	printf("c_SSD_NODE = %d\n", c_SSD_NODE);
	
	int total_NODE_counts = c_PC_NODE + c_GB_NODE + c_SSD_NODE;
	return total_NODE_counts;
}





