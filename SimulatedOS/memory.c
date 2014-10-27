#include <stdio.h>
#include "simos.h"
#include <string.h>
//======================================================================
// Our memory addressing is in WORDs, because of the Memory structure def
// - so all addressing is computed in words
//======================================================================

//#define fixMsize 256  // each process has a fixed memory size
                      // first part of the process memory is for program
                      // remaining is for data
                      // this size is actually 256*4 = 1024B
#define dataSize 4    // each memroy unit is of 4 bytes
#define OSsize OSmemSize // first physical memory segment is for OS
                   // so first user physical memory starts at 1024


/*typedef union
{ float mData;
  int mInstr;
} mType;

#define memSize maxProcess*fixMsize
mType Memory[memSize];
*/
int inst_count =0, loc=0;
NODE *list_create(int data)
{
	NODE *node;
	//if(!(node=malloc(sizeof(NODE)))) return NULL;
	node=(NODE *)malloc(sizeof(NODE));
    node->frame_no=data;
    
	node->next=NULL;
	return node;
}

int list_remove(NODE *list, NODE *node)
{
	while(list->next && list->next!=node) list=list->next;
	if(list->next) {
		list->next=node->next;
		free(node); 
		return 1;
	} else return 0;
}

int addNodeBottom(int val, NODE *head)
{
    NODE *current = head;
    NODE *newNode = (NODE *) malloc(sizeof(NODE));
    if (newNode == NULL) {
        printf("malloc failed\n");
        exit(-1);
    }
    
    newNode->frame_no = val;
    
    newNode->next = NULL;
    
    while (current->next) {
        current = current->next;
    }
    current->next = newNode;
    return 0;
}

NODE *addNodeTop(int val, NODE *head) {
  NODE *current = head;
    NODE *newNode = (NODE *) malloc(sizeof(NODE));
    if (newNode == NULL) {
        printf("malloc failed\n");
        exit(-1);
    }

    newNode->frame_no = val;

    newNode->next = head;

    return newNode;
}


void calculate_physical_addr(int offset)
{
    int reminder=0, quotient=0;
    
    reminder = offset%pageSize;
    
    quotient = offset/pageSize;
    
    
    
    phy_addr[0] = reminder;
    phy_addr[1] = quotient + 1;
   
    /*if(offset<pageSize)
        phy_addr[1] = 1;
    
    if(offset==pageSize)
        phy_addr[1] = 2; */
    printf("offset => %d means page %d, offset %d \n",offset, phy_addr[1], phy_addr[0]);
    //return phy_addr;
    
    
}



#define opcodeShift 24
#define operandMask 0x00ffffff



//============================
// Our memory implementation is a mix of memory manager and physical memory.
// get_instr, put_instr, get_data, put_data are the mix
//   for instr, instr is fetched into registers: IRopcode and IRoperand
//   for data, data is fetched into registers: MBR (need to retain AC value)
//             but stored directly from AC
//   -- one reason is because instr and data do not have the same types
//      also for convenience
// allocate_memory, deallocate_memory are pure memory manager activities
//============================


//==========================================
// run time memory access operations
// Note:
//==========================================

int check_address (maddr,offset)
int maddr,offset;
{ 
  if (Debug) printf ("Memory access: %d; ", maddr);
  if (maddr < OSsize)
  { printf ("Process %d accesses %d. In OS region!\n", CPU.Pid, maddr);
    return (mError);
  }
  else if (maddr > swapSize)
  { printf ("Process %d accesses %d. Outside addr space!\n", CPU.Pid, maddr);
    return (mError);
  }
  else if (offset >= memSize) 
  { printf ("Process %d accesses %d. Outside memory!\n", CPU.Pid, maddr);
    return (mError);
  }
  else 
  { //  maddr= *(int*)maddr;
     //if (Debug) printf ("content = %x, %.2f\n",
       //                 Memory[maddr].mData, Memory[maddr].mInstr);
    return (mNormal);
  }
}

int get_data (offset)
int offset;
{
    int maddr,page_no=0,temp_offset=0,temp=0,temp1=0,y=0, newoffset=0;
       
    temp_offset= offset+ PCB[CPU.Pid]->numInstr;

    calculate_physical_addr(temp_offset);

    
    newoffset = phy_addr[0];
    
    page_no = phy_addr[1];
    
    
 
  if(temp_offset>=pageSize)
    {
    temp = CPU.Mbase + (sizeof(int))*(page_no-1);
    temp1 = *(int*)temp;
    }
  else
  {
  temp = CPU.Mbase;
  temp1 = *(int*)temp;
  }

  //maddr = (temp1-1)*pageSize + newoffset;
       if(temp1!=0) 
         y=search_for_page(CPU.Pid,temp1);
       else
         y=0;
 if(y==0)
    {
	NODE *temp1,*temp,*list_freelist;
        list_freelist= head_freelist;
        temp1= head_freelist;
	//temp1= temp1->next;
	int next_page=1,frame=0,pid= CPU.Pid, x=0, y=0;
        printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PAGE FAULT OCCURED IN GET DATA !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	PCB[pid]->counter= (phy_addr[1]-1)*pageSize;
        if(temp1->frame_no==0)
            {
              next_page=0;
            }
            if(next_page==0)
            {
                printf("Sorry !! .. no pages to allocate ... try removing an allocated page using Age Vector... \n");
                remove_page(pid);
	     }
	PCB[pid]->pageTableptr[page_no-1]= head_freelist->frame_no;
	frame= head_freelist->frame_no;
        AGE[frame]->age |= 128;
	PCB[pid]->page_counter++;
       	/*if(PCB[pid]->page_counter == PCB[pid]->pages_per_process)
          {
                CPU.Mbound = (PCB[pid]->pageTableptr[PCB[pid]->pages_per_process-1])*pageSize -1;
                PCB[pid]->Mbound = CPU.Mbound; 
          }*/
	temp = head_freelist->next;
        list_remove(list_freelist,head_freelist);
        head_freelist = temp;
	x= phy_addr[1]; 
	y= phy_addr[0];
	handle_pagefault(CPU.Pid);
        phy_addr[1]= x;
 	phy_addr[0]= y; 
	get_data(offset);
    }

if(temp_offset>=pageSize)
    {
    temp = CPU.Mbase + (sizeof(int))*(page_no-1);
    temp1 = *(int*)temp;
    }
  else
  {
  temp = CPU.Mbase;
  temp1 = *(int*)temp;
  }
  AGE[temp1]->age |= 128;
maddr = (temp1-1)*pageSize + newoffset;
 if(Observe)
 printf("-----------------------INSIDE GET DATA: FRAME NUMBER IS %d IT IS AT INDEX %d OF PROCESS %d PAGETABLE----------------\n", temp1, page_no, CPU.Pid);
 if(Observe)
 printf("------------------------INSIDE GET DATA: MEMORY LOCATION %d IS BEING ACCESSED------------------\n",maddr);
 if(Observe)
 printf("------------------------ FRAME %d's AGE VECTOR IS UPDATED -------------------\n",temp1);
 if (check_address (maddr,temp_offset) == mError) return (mError);
  else
  { CPU.MBR = Memory[maddr].mData;
    return (mNormal);
  }
}

int search_for_page(int pid, int page)
{
int i=0 , p;

for(i=0;i<PCB[pid]->page_counter;i++)
{
    if(PCB[pid]->pageTableptr[i]==page)
      {
        printf("============================ PAGE %d FOUND !!!!! ===================\n",page); 
      return 1;
     
      }
}
        printf("============================ PAGE %d NOT FOUND !!!!! ===================\n",page);                            

return 0;
}


int put_data (offset)
int offset;
{ int frame,maddr,temp_offset=0, page_no=0,temp=0,temp1=0,y=0, newoffset=0;
      
    temp_offset= offset+ PCB[CPU.Pid]->numInstr;
    
    calculate_physical_addr(temp_offset);

    newoffset = phy_addr[0];
    
    page_no = phy_addr[1];
    
    
        if(temp_offset>=pageSize)
        {
	temp = CPU.Mbase + (sizeof(int))*(page_no-1);
	temp1 = *(int*)temp;
	}
	else
	{
	temp = CPU.Mbase;
	temp1 = *(int*)temp;
	}


       if(temp1!=0)
         y=search_for_page(CPU.Pid,temp1);
       else
         {y=0; printf("------------------ PAGE NOT FOUND IN PAGE TABLE..... PAGE FAULT !!!!! ......... \n.");}
  if(y==0)
    {
        NODE *temp1,*temp,*list_freelist;
	list_freelist= head_freelist; 
   	temp1= head_freelist;
        //temp1= temp1->next;
        int pid= CPU.Pid,next_page=1,frame=0,x=0,y=0;
        printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PAGE FAULT OCCURED IN PUT DATA !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        PCB[pid]->counter= (phy_addr[1]-1)*pageSize;
	if(temp1->frame_no==0)
            {
              next_page=0;
            }
            if(next_page==0)
            {
                printf("Sorry !! .. no pages to allocate ... try removing an allocated page using Age Vector... \n");
                remove_page(pid);
            }
	PCB[pid]->pageTableptr[page_no-1]= head_freelist->frame_no;
        frame= head_freelist->frame_no;
        AGE[frame]->age |= 128;
	PCB[pid]->page_counter++;	
	/*if(PCB[pid]->page_counter == PCB[pid]->pages_per_process)
          {
                CPU.Mbound = (PCB[pid]->pageTableptr[PCB[pid]->pages_per_process-1])*pageSize -1;
                PCB[pid]->Mbound = CPU.Mbound; 
          }*/
        temp = head_freelist->next;
        list_remove(list_freelist,head_freelist);
        head_freelist = temp;
     	x= phy_addr[1]; 
        y= phy_addr[0];
        printf("----------------------------PAGE FAULT WILL GET HANDLED ---------------------------\n");
        handle_pagefault(CPU.Pid);
        phy_addr[1]= x;
        phy_addr[0]= y;put_data(offset);
    }
if(temp_offset>=pageSize)
    {
    temp = CPU.Mbase + (sizeof(int))*(page_no-1);
    temp1 = *(int*)temp;
    }
  else
  {
  temp = CPU.Mbase;
  temp1 = *(int*)temp;
  }

    AGE[temp1]->age |= 128;

maddr = (temp1-1)*pageSize + newoffset;

AGE[temp1]->dirty= 1; 

if(Observe)
printf("-----------------------INSIDE PUT DATA: FRAME NUMBER IS %d IT IS AT INDEX %d OF PROCESS %d PAGETABLE----------------\n", temp1, page_no, CPU.Pid);
if(Observe)
printf("------------------------INSIDE PUT DATA: MEMORY LOCATION %d IS BEING ACCESSED------------------\n",maddr); 
if(Observe)
printf("------------------------ FRAME %d's AGE VECTOR IS UPDATED -------------------\n",temp1);
 printf("------------------------ FRAME %d's DIRTY BIT IS SET TO 1 -------------------\n",temp1);

    if (check_address (maddr,temp_offset) == mError) return (mError);
  else
  { Memory[maddr].mData = CPU.AC;
    return (mNormal);
  }
}

int get_instruction (offset)
int offset;
{ int maddr, instr, temp=0, temp1=0,page_no=0, y=0,newoffset=0;

    calculate_physical_addr(offset);
    newoffset = phy_addr[0];
    
    page_no = phy_addr[1];
    

   if(offset>=pageSize)
      {
      temp = CPU.Mbase + (sizeof(int))*(page_no-1);
      temp1 = *(int*)temp;
      }
    else
    {
    temp = CPU.Mbase;
    temp1 = *(int*)temp;
    }

              
       if(temp1!=0) 
         y=search_for_page(CPU.Pid,temp1);
       else
         y=0; 
  if(y==0)
    {
	NODE *temp,*temp1,*list_freelist;
        list_freelist= head_freelist;
        temp1= head_freelist;
        //temp1= temp1->next;
	int next_page=1,pid= CPU.Pid,x=0,y=0,frame=0;
        PCB[pid]->counter= (phy_addr[1]-1)*pageSize;
       printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PAGE FAULT OCCURED IN PUT DATA !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
 
	if(temp1->frame_no==0)
            {
              next_page=0;
            }
            if(next_page==0)
            {
                printf("Sorry !! .. no pages to allocate ... try removing an allocated page using Age Vector... \n");
                remove_page(pid);
            }

	PCB[pid]->pageTableptr[page_no-1]= head_freelist->frame_no;
        frame= head_freelist->frame_no;
        AGE[frame]->age |= 128;
	PCB[pid]->page_counter++;
	/*if(PCB[pid]->page_counter == PCB[pid]->pages_per_process)
          {
		CPU.Mbound = (PCB[pid]->pageTableptr[PCB[pid]->pages_per_process-1])*pageSize -1; 
  	  	PCB[pid]->Mbound = CPU.Mbound; 
	  }*/
        temp = head_freelist->next;
        list_remove(list_freelist,head_freelist);
        head_freelist = temp;
        x= phy_addr[1]; 
        y= phy_addr[0];
        handle_pagefault(CPU.Pid);
        phy_addr[1]= x;
        phy_addr[0]= y;
	get_instruction(offset);
    }
if(offset>=pageSize)
    {
    temp = CPU.Mbase + (sizeof(int))*(page_no-1);
    temp1 = *(int*)temp;
    }
  else
  {
  temp = CPU.Mbase;
  temp1 = *(int*)temp;
  }
    AGE[temp1]->age |= 128;

	maddr = (temp1-1)*pageSize + newoffset;
printf("-----------------------INSIDE GET INSTRUCTION: FRAME NUMBER IS %d IT IS AT INDEX %d OF PROCESS %d PAGETABLE----------------\n", temp1, page_no, CPU.Pid);
 printf("------------------------INSIDE GET INSTRUCTION: MEMORY LOCATION %d IS BEING ACCESSED------------------\n",maddr);
 printf("------------------------ FRAME %d's AGE VECTOR IS UPDATED -------------------",temp1);

int pid = CPU.Pid;
int addr_chk= CPU.Mbase+((sizeof(int))*offset);  

 if (check_address (maddr,offset) == mError) return (mError);
  else
  { instr = Memory[maddr].mInstr;
    printf("instruction  obtained is %d\n", instr);
    CPU.IRopcode = instr >> opcodeShift; 
    CPU.IRoperand = instr & operandMask;
   printf(" CPU opcode is %d\n", CPU.IRopcode);
   printf(" CPU operand is %d\n", CPU.IRoperand);
    return (mNormal);
  }
}

//==========================================
// load instructions and data into memory 
// a specific pid is needed for loading, since registers are not for this pid
//==========================================

int check_load_address (pid, maddr,offset)
int pid, maddr,offset;
{ 
  if (maddr < OSmemSize)
  { printf ("Process %d accesses %d. In OS region!\n", pid, maddr);
    return (mError);
  }
  else if (maddr > swapSize)
  { printf ("Process %d accesses %d. Outside address space!\n", pid, maddr);
    return (mError);
  }
  else if (offset >= memSize) 
  { printf ("Process %d accesses %d. Outside memory!\n", pid, maddr);
    return (mError);
  }
  else return (mNormal);
}


int load_instruction_idle (pid, offset, opcode, operand)
int pid, offset, opcode, operand;
{ int  maddr, page_no=0,temp=0,temp1, y=0,newoffset=0;
    calculate_physical_addr(offset);
    newoffset = phy_addr[0];
    page_no = phy_addr[1];

   if(offset>=pageSize)
   {
    temp = PCB[pid]->Mbase + (sizeof(int))*(page_no-1);
    temp1 = *(int*)temp;
   }
   else
   {
      temp = PCB[pid]->Mbase;
      temp1 = *(int*)temp;
   }

    maddr = (temp1-1)*pageSize + newoffset;

    PCB[pid]->mem_locs[loc] = maddr;
    loc++;

    if (check_load_address (pid,maddr,offset) == mError) return (mError);
  else
  { opcode = opcode << opcodeShift;
    operand = operand & operandMask;
    Memory[maddr].mInstr = opcode | operand;
    printf(" INSTRUCTION: %d loaded into memory at %d location in memory\n", Memory[maddr].mInstr,maddr);
    return (mNormal);
  }
}

int load_instruction(int pid ,int offset, int* ptr)
{ 

  int  opcode, operand;
  int  maddr, page_no=0,temp=0,temp1, y=0,newoffset=0;
  int maddr_swap, temp_swap=0, temp1_swap=0;
    
    
    calculate_physical_addr(offset);
    
    newoffset = phy_addr[0];
    page_no = phy_addr[1];
    
    
   if(offset>=pageSize)
   {
    temp = PCB[pid]->Mbase + (sizeof(int))*(page_no-1);
    temp_swap = ptr + (page_no-1);
    temp1 = *(int*)temp;
    temp1_swap= *(int *)temp_swap;
   }
   else
   {
      temp = PCB[pid]->Mbase;
      temp_swap= ptr; 
      temp1 = *(int*)temp;
      temp1_swap = *(int*)temp_swap;
   }
    
    maddr = (temp1-1)*pageSize + newoffset;
    maddr_swap =(temp1_swap-1)*pageSize + newoffset;

    PCB[pid]->mem_locs[loc] = maddr;
    loc++;
    if(Observe)

printf("--------------- FRAME of MAIN MEMORY %d FRAME of SWAP SPACE %d: LOADING INSTRUCTION FROM SWAP SPACE LOCATION %d TO MAIN MEMORY LOCATION %d ----------------\n",temp1,temp1_swap,maddr,maddr_swap); 
    if (check_load_address (pid,maddr,offset) == mError) return (mError);
  else
  { //opcode = opcode << opcodeShift;
    //operand = operand & operandMask;
    //Memory[maddr].mInstr = opcode | operand;
    
    Memory[maddr].mInstr = SwapSpace[maddr_swap].mInstr;

    printf(" INSTRUCTION: %d loaded into memory at %d location in memory\n", Memory[maddr].mInstr,maddr);
    return (mNormal);
  }
}

int load_data_idle (int pid,int offset,float data)
{ 
   int maddr,temp=0,temp1,temp_offset=0, page_no=0,y=0, newoffset=0;
   
   temp_offset= offset+ PCB[pid]->numInstr;
    calculate_physical_addr(temp_offset);
    newoffset = phy_addr[0];
    page_no = phy_addr[1];
        if(temp_offset>=pageSize)
        {
           temp = PCB[pid]->Mbase + (sizeof(int))*(page_no-1);
           temp1 = *(int*)temp;
        }
        else
        {
        temp = PCB[pid]->Mbase;
        temp1 = *(int*)temp;
        }


    maddr = (temp1-1)*pageSize + newoffset;
    PCB[pid]->mem_locs[loc] = maddr;
    loc++;

  if (check_load_address (pid,maddr,temp_offset) == mError) return (mError);
  else
  { Memory[maddr].mData = data;

      printf(" DATA : %f loaded into memory at %d location in memory \n", data , maddr);
    return (mNormal);
  }
}


int load_data (int pid, int offset, int *ptr)
{ 
     float data;

      int maddr,temp=0,temp1,temp_offset=0, page_no=0,y=0, newoffset=0;
      int maddr_swap, temp_swap=0, temp1_swap=0;

   temp_offset= offset;
    
    calculate_physical_addr(temp_offset);
    
    newoffset = phy_addr[0];

    page_no = phy_addr[1];
    
    
	if(temp_offset>=pageSize)
   	{
       	   temp = PCB[pid]->Mbase + (sizeof(int))*(page_no-1);
           temp_swap = ptr + (page_no-1);
	   temp1 = *(int*)temp;
	   temp1_swap=*(int*)temp_swap;
        }
	else
	{
	temp = PCB[pid]->Mbase;
	temp_swap=ptr;
	temp1 = *(int*)temp;
	temp1_swap=*(int*)temp_swap;
	}
    
       
    maddr = (temp1-1)*pageSize + newoffset;
    maddr_swap= (temp1_swap-1)*pageSize + newoffset;
    
    PCB[pid]->mem_locs[loc] = maddr;
    loc++;
    if(Observe)

   printf("--------------- FRAME of MAIN MEMORY %d FRAME of SWAP SPACE %d: LOADING DATA FROM SWAP SPACE LOCATION %d TO MAIN MEMORY LOCATION %d ----------------\n",temp1,temp1_swap,maddr,maddr_swap); 
  if (check_load_address (pid, maddr,offset) == mError) return (mError);
  else
  { Memory[maddr].mData = SwapSpace[maddr_swap].mData;
      
      printf(" DATA : %f loaded into memory at %d location in memory \n", Memory[maddr].mData , maddr);
    return (mNormal);
  }
}

//==========================================
// memory management functions -- to be implemented for paging

void memory_agescan ()
{

 printf("-------------------------------------------MEMORY AGE SCAN-------------------------------\n");
  int i=0,y=0,x=0,page=0;
for(i=OSpages+idlePages+1;i<=OriginalPages;i++)
{
   AGE[i]->age = AGE[i]->age>>1;
}
printf("--------------------ALL AGE VECTORS ARE SHIFTED TO RIGHT BY 1---------------------\n");
   for(i=2;i<=process_counter;i++)
   {
       x = PCB[i]->page_counter;
       
       for(y=0;y<x;y++)
       {
         page= PCB[i]->pageTableptr[y];
         if(page!=0)
          if(AGE[page]->age==0)
          {
	printf(" !!!!!!!!!!!!!!!!!!!!!!!! FRAME %d HAS AGE VECTOR 0 ... IT HAS TO BE MOVED TO FREE LIST !!!!!!!!!!!!! \n", page);
	    remove_frame(page,process_counter);
           
          }
     }
   }
}

void init_pagefault_handler (pid)
{
  printf ("Page fault handler is being activated.\n");
}

void pagefault_complete (pid)
{ 
  insert_doneWait_process (pid);
  set_interrupt (doneWaitInterrupt);
}

void handle_pagefault(int pid)
{

  load_RAM(pid);
  
  printf("----------------- LOADED A NEW PAGE FROM SWAP SPACE TO MAIN MEMORY -----------------\n");
}

//==========================================
// memory management functions -- general


void initialize_memory ()
{ 
  add_timer (periodAgeScan, osPid, actAgeInterrupt, periodAgeScan);
 int i=0;
 int frame=0;
 NODE *temp_node;
 temp_node= head_freelist;
 for(i=idlePages+OSpages+1;i<=OriginalPages;i++)
 {
 	 frame= i;  
	 AGE[frame]= (type_AGE_Dirty *)malloc(sizeof(type_AGE_Dirty));
	 AGE[frame]->age='\0';
	 AGE[frame]->dirty=0;

 }

printf("------------------- ADDED TIMER and ALL FRAMES ARE INITIALIZED WITH THEIR AGE VECTORS AND DIRTY BITS------------\n");
 // in demand paging, some more initialization is probably needed
}

void remove_page(int pid)
{
int start_frame=0,i,y,x,page,req_page=0,req_i=0,req_y=0;
char z, min='\0';
NODE *temp;
start_frame= PCB[2]->pageTableptr[0]; 
min= AGE[start_frame]->age;
for(i=2; i<=process_counter;i++)
{
 x = PCB[i]->page_counter;
 for(y=0;y<x;y++)
 {
   page= PCB[i]->pageTableptr[y];
if(page!=0){
   z=AGE[page]->age;
   if(z<=min){
     min=z;
     req_page= page;
     req_i=i;
     req_y=y;
     }
  }
 }

}
if(Observe)

printf("-----------------INSIDE PROCESS %d PAGE NO: %d HAS FRAME %d IN IT.... ITS AGE IS MINIMUM OF ALL IT SHOULD BE GIVEN BACK TO FREE LIST-------------\n", req_i, req_y, req_page );
temp= addNodeTop(req_page, head_freelist);
head_freelist = temp;
if(Observe)

printf("---------------- FRAME %d GIVEN BACK TO FREE LIST..... NEW LIST IS AS FOLLOWS : \n",req_page);
if(Observe)
print_freelist(head_freelist);
if(AGE[req_page]->dirty==1)
{
  int p=0,q=0,r=0,rem_instr,i, *source, *dest;
printf(" !!!!!!!!!!!! OOPS DIRTY BIT OF FRAME %d IS 1 !!!!!!!!!! \n", req_page);

/*q= PCB[pid]->pageTableptr_swap[y];
 r= req_page;
 q= (q-1)*pageSize;
 r= (r-1)*pageSize; 
*/ 
 q= PCB[req_i]->pageTableptr_swap[req_y];
 r= PCB[req_i]->pageTableptr[req_y];
 q= (q-1)*pageSize;
 r= (r-1)*pageSize;
printf("------------------------- SWAP SPACE SHOULD BE UPDATED WITH NEW DATA -------------------\n");
if(PCB[req_i]->numInstr>=pageSize)
 {
    for(i=0;i<pageSize;i++){
      SwapSpace[q].mData= Memory[r].mData;
    if(Observe)
    printf(" %f of MAIN MEMORY at LOCATION %d -----COPY TO----->>>>>SWAP SPACE TO LOCATION %d\n",  Memory[r].mData, r, q);
        if((i+1)==PCB[req_i]->numData) break;
      q++; r++;
    }
 }
else 
{
  rem_instr= PCB[req_i]->numInstr-pageSize;
   for(i=0;i<rem_instr;i++){
     SwapSpace[q].mInstr= Memory[r].mInstr;
    if(Observe)
    printf(" %d of MAIN MEMORY at LOCATION %d -----COPY TO----->>>>>SWAP SPACE TO LOCATION %d\n",  Memory[r].mInstr, r, q);
q++; r++;
   }
      for(i=rem_instr;i<=pageSize;i++)
       { SwapSpace[q].mData= Memory[r].mData;
    if(Observe)
    printf(" %f of MAIN MEMORY at LOCATION %d -----COPY TO----->>>>>SWAP SPACE TO LOCATION %d\n",  Memory[r].mData, r, q);
         if(i==PCB[req_i]->numInstr+PCB[req_i]->numData)
        q++; r++;
      }
} 
/*numinstr=  PCB[req_i]->numInstr;

 source= &(Memory[r]);
 dest= &(SwapSpace[q]);

memcpy(dest,source,32);
 for(p=0;p<8;p++)
    {
    SwapSpace[q]=Memory[r];
    printf("Data  %d in memory at location %d copied into  %d Swap Space to location %d \n", Memory[r], r, SwapSpace[q], q);
    q++;
    r++;
    }*/
if(Observe)
printf("------------------ FRAME %d IS COPIED TO SWAP SPACE-----------------\n", req_page);
AGE[req_page]->dirty=0;
if(Observe)
printf("---------------------DIRTY BIT MADE 0 AGAIN------------------------\n");
}

PCB[req_i]->pageTableptr[req_y] =0;
}

void remove_frame(int frame, int proc_count)
{
int i,x,y,page, req_page, req_i, req_y;
for(i=2; i<=proc_count;i++)
{
 x = PCB[i]->page_counter;
 for(y=0;y<x;y++)
 {
   page= PCB[i]->pageTableptr[y];
  
 if(page!=0)
    {
     if(page==frame)
  {
     addNodeBottom(frame, head_freelist);
     req_page= frame;
     req_i=i;
     req_y=y;
     }
   }
   }
 }


if(AGE[req_page]->dirty==1)
{

printf(" !!!!!!!!!!!!!!!! OOPS DIRTY BIT IS 1 ...... COPY FRAME %d  TO SWAP SPACE !!!!!!!!!!!!!!!!! \n", req_page);
  int p=0,q=0,r=0,rem_instr,i, *source, *dest;

 q= PCB[req_i]->pageTableptr_swap[req_y];
 r= PCB[req_i]->pageTableptr[req_y];
 q= (q-1)*pageSize;
 r= (r-1)*pageSize;

if(PCB[req_i]->numInstr>=pageSize)
 {
    for(i=0;i<pageSize;i++){
      SwapSpace[q].mData= Memory[r].mData;
      if(Observe)
      printf(" %f of MAIN MEMORY at LOCATION %d -----COPY TO----->>>>>SWAP SPACE TO LOCATION %d\n",  Memory[r].mData, r, q);
     if((i+1)==PCB[req_i]->numData) break;
      q++; r++;
    }
 }
else
{
  rem_instr= PCB[req_i]->numInstr-pageSize;
   for(i=0;i<rem_instr;i++){
     SwapSpace[q].mInstr= Memory[r].mInstr;
     if(Observe)
     printf(" %d of MAIN MEMORY at LOCATION %d -----COPY TO----->>>>>SWAP SPACE TO LOCATION %d\n",  Memory[r].mInstr, r, q);
q++; r++;
   }
      for(i=rem_instr;i<=pageSize;i++)
       { SwapSpace[q].mData= Memory[r].mData;
          if(Observe)
	  printf(" %f of MAIN MEMORY at LOCATION %d -----COPY TO----->>>>>SWAP SPACE TO LOCATION %d\n",  Memory[r].mData, r, q);
if(i==PCB[req_i]->numInstr+PCB[req_i]->numData)
        q++; r++;
      }
}




/* q= PCB[req_i]->pageTableptr_swap[y];
 r= PCB[req_i]->pageTableptr[y];
 q= (q-1)*pageSize;
 r= (r-1)*pageSize;
source= &(Memory[r]);
dest= &(SwapSpace[q]);

memcpy(dest,source,32);

  for(p=0;p<8;p++)
    {
    SwapSpace[q]=Memory[r];
    printf("Data  %f in memory at location %d copied into  %d Swap Space to location %d \n", Memory[r], r, SwapSpace[q], q);
    q++;
    r++;
    }*/
if(Observe)
printf("------------------------------ FREE LIST AFTER REMOVING FRAME %d OF PROCESS %d FROM ITS PAGE TABLE---------------------\n", req_page, req_i);
if(Observe)
printf("------------------------------ DIRTY BIT MADE 0 AGAIN -----------------------\n");
AGE[req_page]->dirty=0;
}

PCB[req_i]->pageTableptr[req_y] =0;

}
int allocate_pages(int pid,int pages_process)

{
    int x=0,frame=0,counter=0,next_page=1;
    NODE *temp1,*list_freelist, *temp;
    list_freelist = head_freelist;
    temp = head_freelist;
    //temp1= temp->next;
    printf("----------------------- ALLOCATING PAGE FOR PROCESS %d -----------------\n", pid);
        for(x=1;x<=PCB[pid]->pages_per_process; x++)
	          (PCB[pid]->pageTableptr[x]) = 0;

	for(x=0;x<1; x++)
        {
            if(temp==NULL)
	    {
	      next_page=0;
	    }
	    if(next_page==0)
	    {
		printf("!!!!!! SORRY .... NO PAGES LEFT IN FREE LIST...... HAVE TO LOOK INTO AGE VECTORS AND REPLACE A FRAME !!!!!!!!!! \n");
		remove_page(pid);
	    }
	    list_freelist = head_freelist;
    	    temp = head_freelist;
	    PCB[pid]->pageTableptr[x] = list_freelist->frame_no;
             printf("-------------------------- FRAME %d IS ALLOCATED TO PROCESS %d -----------------\n",list_freelist->frame_no, pid );
	    frame= list_freelist->frame_no;
	    if(pid!=1)
	      AGE[frame]->age |= 128;

	if(Observe)
	printf("---------------------------- AGE VECTOR OF FRAME %d UPDATED ------------------------", frame);
	    PCB[pid]->page_counter++;
            temp = head_freelist->next;
 	if(Observe)
	printf("---------------------------- UPDATED FREE LIST IS : -----------------------\n");
            list_remove(list_freelist,head_freelist);
            head_freelist = temp;
            list_freelist= list_freelist->next;
        }

       // for(x=1;x<=PCB[pid]->pages_per_process; x++)
         // (PCB[pid]->pageTableptr[x]) = 0;
    tail_freelist= list_freelist;
    print_freelist(head_freelist);
 return 1;
}



// We always allocate a fixed memory size at a fix starting location
// Need to be changed to a better memory allocation
int allocate_memory (int pid, int pages_process)
{

int  msize, numinstr;

  numinstr= PCB[pid]->numInstr;
  msize= numinstr + PCB[pid]->numData;
  
  if (pid >= maxProcess) 
  { printf ("Invalid pid: %d\n", pid); return(mError); }
  else if (msize > fixMsize) 
  { printf ("Invalid memory size %d for process %d\n", msize, pid);
    return(mError);
  }
  else 
  { 
    PCB[pid]->pageTableptr = (int*)malloc(pages_process*sizeof(int));

     int tempo=0;
     loc=0;
    tempo=allocate_pages(pid, pages_process);
    if(tempo==0)
    {
	printf("process cannot be submitted ... no free pages available ... \n");
	return mError;
    }
    //tempo = tail_freelist->frame_no;
    inst_count = numinstr ;

      PCB[pid]->Mbase =  PCB[pid]->pageTableptr;
      PCB[pid]->Mbound =  PCB[pid]->Mbase + (sizeof(int))*(msize);
      PCB[pid]->MDbase = PCB[pid]->Mbase + (sizeof(int))*(numinstr);
      PCB[pid]->pages_per_process = pages_process;
      CPU.Mbase= PCB[pid]->Mbase; 
      CPU.Mbound= PCB[pid]->Mbound; 
      CPU.MDbase= PCB[pid]->MDbase; 
    
    
    return (mNormal);
  }
}

// Due to our simple allocation, nothing to do for deallocation
int free_memory (pid)
int pid;
{
    int x=0;
    printf("*********************process %d freed the memory **************\n", pid);
    for(x=0;x< PCB[pid]->pages_per_process ;x++)
    {
        if(PCB[pid]->pageTableptr[x]!=0)
         addNodeBottom(PCB[pid]->pageTableptr[x], head_freelist);
    }
    PCB[pid]->page_counter=0;
    print_freelist(head_freelist);
  return (mNormal);
}

void dump_memory (pid)
int pid;
{ 
  int i, x=0,start, pagenumber=0;
  int p=0,q=0,pp=0,qq=0,count = 0;
    
    printf("***************Free list of frames is ************ \n");
    NODE* temp;
    temp=head_freelist;
    while(temp!=NULL)
    {
        printf("%d->",temp->frame_no);
        temp=temp->next;
    }

  printf("\n***************Page Table of process %d is  ************ \n", pid);
    
    for(x=0; x< PCB[pid]->pages_per_process; x++)
    {
        p= PCB[pid]->pageTableptr +x; 
	q=*(int*)p; 
        printf("%d->", q);
    }
    printf("\n");
  
  
   printf("*************** AGE VECTORS AND DIRTY BITS *************** \n");
    
  for(x=idlePages+OSpages+1; x<= OriginalPages; x++)
    printf("FRAME NUMBER %d, AGE VECTOR %d, DIRTY BIT %d \n", x, AGE[x]->age, AGE[x]->dirty); 


  printf ("************ Instruction Memory Dump for Process %d ***********\n", pid);
  x=0;
  for (i=0; i<PCB[pid]->numInstr; i++)
  {
    start = PCB[pid]->mem_locs[i];
    if(x==pageSize)
    {
        pagenumber++;
        x=0;
    }

    pp= PCB[pid]->pageTableptr + pagenumber;
    qq=*(int*)pp;
    
    if(qq!=0){printf("location in memory is %d \n", start);
    printf ("PAGE : %d  FRAME : %d  %x ", pagenumber,qq ,Memory[start].mInstr);
   }else printf (" PAGE : %d Has No Frame ", pagenumber);
   x++;
    printf ("\n");
  }
  printf ("************ Data Memory Dump for Process %d *************\n", pid);
  
  count = PCB[pid]->numInstr;
  for (i=0; i<PCB[pid]->numStaticData; i++)
  {
    start = PCB[pid]->mem_locs[i+count];
      if(x==pageSize)
      {
          pagenumber++;
          x=0;
      }
       pp= PCB[pid]->pageTableptr + pagenumber;
       qq=*(int*)pp;
   
      if(qq!=0) {printf("location in memory is %d \n", start); 
      printf (" PAGE : %d FRAME : %d    %.2f ", pagenumber, qq, Memory[start].mData);}
       else printf (" PAGE : %d Has No Frame ", pagenumber); 
       
       x++;
    printf ("\n");
  }
}



