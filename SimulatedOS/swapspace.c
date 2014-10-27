#include <stdio.h>
#include "simos.h"

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


int inst_count_swap =0, loc_swap=0;

void calculate_physical_addr_swap(int offset)
{
    int reminder=0, quotient=0;
    
    reminder = offset%pageSize;
    
    quotient = offset/pageSize;
    
    
    
    phy_addr_swap[0] = reminder;
    phy_addr_swap[1] = quotient + 1;
   
    /*if(offset<pageSize)
        phy_addr[1] = 1;
    
    if(offset==pageSize)
        phy_addr[1] = 2; */
    
    printf("offset => %d means page %d, offset %d \n",offset, phy_addr_swap[1], phy_addr_swap[0]);
    
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
/*
int check_address (maddr)
int maddr;
{ 
  if (Debug) printf ("Memory access: %d; ", maddr);
  if (maddr < OSsize)
  { printf ("Process %d accesses %d. In OS region!\n", CPU.Pid, maddr);
    return (mError);
  }
  else if (maddr > CPU.Mbound)
  { printf ("Process %d accesses %d. Outside addr space!\n", CPU.Pid, maddr);
    return (mError);
  }
  else if (maddr >= memSize) 
  { printf ("Process %d accesses %d. Outside memory!\n", CPU.Pid, maddr);
    return (mError);
  }
  else 
  { if (Debug) printf ("content = %x, %.2f\n",
                        SwapSpace[maddr].mData, SwapSpace[maddr].mInstr);
    return (mNormal);
  }
}

int get_data (offset)
int offset;
{
    int maddr,page_no=0,temp_offset=0,temp=0,temp1=0,y=0, newoffset=0;
       
    temp_offset= offset+ CPU.MDbase;

    calculate_physical_addr(temp_offset);

    
    newoffset = phy_addr[0];
    
    page_no = phy_addr[1];
    
    
    printf( " newoffset is %d, page_no is %d\n", newoffset, page_no);
 
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
  
  maddr = (temp1)*pageSize + newoffset;


  if (check_address (maddr) == mError) return (mError);
  else
  { CPU.MBR = SwapSpace[maddr].mData;
    return (mNormal);
  }
}
*/
int put_data_swap (offset)
int offset;
{ int maddr,temp_offset=0, page_no=0,temp=0,temp1=0,y=0, newoffset=0;
      
    temp_offset= offset+ CPU.MDbase;

    calculate_physical_addr(temp_offset);

    newoffset = phy_addr[0];
    
    page_no = phy_addr[1];
    
    printf("newoffset is %d, page_no is %d \n", newoffset, page_no);
    
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
	
	maddr = (temp1-1)*pageSize + newoffset;

    if (check_address (maddr) == mError) return (mError);
  else
  { SwapSpace[maddr].mData = CPU.AC;
    return (mNormal);
  }
}
/*
int get_instruction (offset)
int offset;
{ int maddr, instr, temp=0, temp1=0,page_no=0, y=0,newoffset=0;

    calculate_physical_addr(offset);
    newoffset = phy_addr[0];
    
    page_no = phy_addr[1];
    
    printf(" newoffset is %d, page_no is %d\n", newoffset, page_no);

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

    maddr = (temp1)*pageSize + newoffset;
  
  if (check_address (maddr) == mError) return (mError);
  else
  { instr =SwapSpace[maddr].mInstr;
    printf("instruction  obtained is %d\n", instr);
    CPU.IRopcode = instr >> opcodeShift; 
    CPU.IRoperand = instr & operandMask;
    printf(" CPU operand is %d\n", CPU.IRoperand);
    return (mNormal);
  }
}
*/
//==========================================
// load instructions and data into memory 
// a specific pid is needed for loading, since registers are not for this pid
//==========================================

int check_load_address_swap (pid, maddr)
int pid, maddr;
{ 
  /*if (maddr < OSmemSize)
  { printf ("Process %d accesses %d. In OS region!\n", pid, maddr);
    return (mError);
  }*/
   if (maddr > PCB[pid]->Mbound)
  { printf ("Process %d accesses %d. Outside address space!\n", pid, maddr);
    return (mError);
  }
  else if (maddr >= swapSize) 
  { printf ("Process %d accesses %d. Outside SWAP SPACE memory!\n", pid, maddr);
    return (mError);
  }
  else return (mNormal);
}

int load_instruction_swap (pid, offset, opcode, operand)
int pid, offset, opcode, operand;
{ int  maddr, page_no=0,temp=0,temp1, y=0,newoffset=0;

    calculate_physical_addr_swap(offset);
    
    newoffset = phy_addr_swap[0];
    page_no = phy_addr_swap[1];
    
    printf(" SWAPSPACE : process %d in newoffset is %d, page_no is %d\n", pid, newoffset, page_no);
    
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
 
    PCB[pid]->mem_locs_swap[loc_swap] = maddr;
    loc_swap++;
 
    if (check_load_address_swap (pid, maddr) == mError) return (mError);
  else
  { opcode = opcode << opcodeShift;
    operand = operand & operandMask;
    SwapSpace[maddr].mInstr = opcode | operand;
    printf(" SWAPSPACE: INSTRUCTION: %d loaded into memory at %d location in memory\n", SwapSpace[maddr].mInstr,maddr);
    return (mNormal);
  }
}

int load_data_swap (pid, offset, data)
int pid, offset;
float data;
{ int maddr,temp=0,temp1,temp_offset=0, page_no=0,y=0, newoffset=0;
    
   temp_offset= offset+ PCB[pid]->numInstr;
    
    calculate_physical_addr_swap(temp_offset);
    
    newoffset = phy_addr_swap[0];

    page_no = phy_addr_swap[1];
    
    printf("SWAPSPACE process %d in newoffset is %d, page_no is %d, data is %f\n", pid, newoffset, page_no, data);
    
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
    
    PCB[pid]->mem_locs_swap[loc_swap] = maddr;
    loc_swap++;
    
  if (check_load_address_swap (pid, maddr) == mError) return (mError);
  else
  { SwapSpace[maddr].mData = data;
      
      printf(" SWAPSPACE : DATA : %f loaded into memory at %d location in memory \n", data , maddr);
    return (mNormal);
  }
}


//==========================================
// memory management functions -- to be implemented for paging
/*
void memory_agescan ()
{
  printf ("Scan and update age vectors for memory pages.\n");
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
*/
//==========================================
// memory management functions -- general

/*
void initialize_memory ()
{ 
  add_timer (periodAgeScan, osPid, actAgeInterrupt, periodAgeScan);
  // in demand paging, some more initialization is probably needed
}
*/
void allocate_pages_swapspace(int pid,int pages_process)

{
    int x=0,counter=0;
    NODE *list_freelist, *temp;
    list_freelist = head_freelist_swap;
    temp = head_freelist_swap;
    printf("%d \n", list_freelist->frame_no);
    printf("allocating pages for process %d in swap space, pages needed is %d\n", pid, pages_process);
        for(x=0;x< pages_process; x++)
        {
            printf("%d \n", list_freelist->frame_no);
            
            (PCB[pid]->pageTableptr_swap[x]) = list_freelist->frame_no;
            printf(" %d , value %d, address %d", (PCB[pid]->pageTableptr_swap), (PCB[pid]->pageTableptr_swap[x]), &((PCB[pid]->pageTableptr_swap[x])));
            temp = head_freelist_swap->next;
            list_remove(list_freelist,head_freelist_swap);
            head_freelist_swap = temp;
            list_freelist= list_freelist->next;
        }
    tail_freelist_swap = list_freelist;
    print_freelist(head_freelist_swap);
    
}



// We always allocate a fixed memory size at a fix starting location
// Need to be changed to a better memory allocation
int allocate_swap_memory(pid, msize, numinstr, pages_process)
int pid, msize, numinstr, pages_process;
{
  if (pid >= maxProcess) 
  { printf ("Invalid pid: %d\n", pid); return(mError); }
  else if (msize > fixMsize) 
  { printf ("Invalid memory size %d for process %d\n", msize, pid);
    return(mError);
  }
  else 
  {
    PCB[pid]->pageTableptr_swap = (int*)malloc(pages_process*sizeof(int));
    

    allocate_pages_swapspace(pid, pages_process);

  int tempo=0;
  loc_swap=0;
  tempo = tail_freelist_swap->frame_no;

    inst_count_swap = numinstr ;
    
              printf("pages per process is %d\n", pid);
     
      PCB[pid]->pages_per_process= pages_process;  
      PCB[pid]->Mbase =  PCB[pid]->pageTableptr_swap;
      PCB[pid]->Mbound =  tempo*pageSize-1;;
      PCB[pid]->MDbase = numinstr;
     /*
      PCB[pid]->Mbase = pid * fixMsize;
      PCB[pid]->Mbound = PCB[pid]->Mbase + msize - 1;
      PCB[pid]->MDbase = PCB[pid]->Mbase + numinstr;*/
          // here we let the first part of process memory be instructions
          // MDbase starts after numinstr instructions
    
    return (mNormal);
  }
}

// Due to our simple allocation, nothing to do for deallocation
int free_swap_memory (pid)
int pid;
{
    int x=0;
    
    for(x=0;x<PCB[pid]->pages_per_process;x++)
    {
        addNodeBottom(PCB[pid]->pageTableptr_swap[x], head_freelist_swap);
    }
    
  return (mNormal);
}
void dump_swap_memory (pid)
int pid;
{
  int i, x=0,start, pagenumber=0;
  int p=0,q=0,pp=0,qq=0,count = 0;

    printf("***************Free list of frames is ************ \n");
    NODE* temp;
    temp=head_freelist_swap;
    while(temp!=NULL)
    {
        printf("%d->",temp->frame_no);
        temp=temp->next;
    }
if(pid!=1){
  printf("\n***************Page Table of process %d in SWAP SPACE is  ************ \n", pid);

    for(x=0; x< PCB[pid]->pages_per_process; x++)
    {
	p= PCB[pid]->pageTableptr_swap +x;
        q=*(int*)p;
        printf("%d->", q);
    }
    printf("\n");
  printf ("************ Instruction Memory Dump for Process %d ***********\n", pid);
  x=0;
  for (i=0; i<PCB[pid]->numInstr; i++)
  {
    start = PCB[pid]->mem_locs_swap[i];
    printf("location in SWAP SPACE is %d \n", start);
    if(x==pageSize)
    {
        pagenumber++;
        x=0;
	 
    }

    pp= PCB[pid]->pageTableptr_swap + pagenumber;
    qq=*(int*)pp;

    printf ("PAGE: %d,  %x ", qq ,SwapSpace[start].mInstr);
    x++;
    printf ("\n");
  }
  printf ("************ Data Memory Dump for Process %d *************\n", pid);

  count = PCB[pid]->numInstr;
  for (i=0; i<PCB[pid]->numStaticData; i++)
  {
    start = PCB[pid]->mem_locs_swap[i+count];
      if(x==pageSize)
      {
          pagenumber++;
          x=0;
      }
       pp= PCB[pid]->pageTableptr_swap + pagenumber;
       qq=*(int*)pp;

       printf (" PAGE : %d  %.2f ", qq, SwapSpace[start].mData);
       x++;
    printf ("\n");
  }
}
}



