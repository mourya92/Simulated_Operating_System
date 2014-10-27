#include <stdio.h>
#include "simos.h"

int find_pages(int i, int j)
{
    int page_count=0,reminder=0, quotient=0;
    quotient= i/j;
    reminder = i%j;
    
    if(reminder>0)
    {
        page_count= quotient + 1;
        return page_count;
    }
    else{
        page_count= quotient;
        return page_count;
    }
    

}

void print_freelist(NODE *temp)
{
    //temp= head_freelist;
    do
    {
        printf(" %d ->",temp->frame_no);
        if(temp->next!=NULL)
	 temp=temp->next;
        
        
    }while(temp->next!=NULL);
   printf("%d ",temp->frame_no); 
    printf("\n");

    
}
void initialize_system ()
{
    
   FILE *fconfig;

  fconfig = fopen ("config.sys", "r");
  fscanf (fconfig, "%d\n", &Observe);
  fscanf (fconfig, "%d %d\n", &cpuQuantum, &idleQuantum);
  fscanf (fconfig, "%d %d %d %d\n", &pageSize, &memSize, &swapSize, &OSmemSize);
  fscanf (fconfig, "%d\n", &periodAgeScan);
  fscanf (fconfig, "%d\n", &spoolPsize);
  fclose (fconfig);
    
    OriginalPages= find_pages(memSize,pageSize);
    OriginalPages_swapspace = find_pages(swapSize,pageSize);

    rem_pages = OriginalPages;
    rem_pages_swapspace = OriginalPages_swapspace;

    #undef num_pages
    #define num_pages OriginalPages;
    
    OSpages= find_pages(OSmemSize,pageSize);
    
    printf("OS Size is %d , OS pages are %d\n", OSmemSize, OSpages);
    
    rem_pages= OriginalPages-OSpages;
    
    idlePages= find_pages(3, pageSize);

 /*printf("Idle Size is 256, Idle pages are %d, rem pages are %d \n", idlePages, rem_pages);
    
    rem_pages = rem_pages-idlePages;*/
    
    printf("remaining pages are %d\n", rem_pages);
    
    
     #undef num_pages
     #define num_pages rem_pages;
   
   printf("Number of frames in Main memory is %d remaininng pages are %d\n", OriginalPages, rem_pages);
   
    
  initialize_cpu ();
    printf("initialized CPU \n");
  initialize_swap_freelist(rem_pages_swapspace, OriginalPages_swapspace);
     printf("initialized Swap Space \n");
  initialize_freelist(rem_pages, OriginalPages);
    printf("initialized Main Memory \n");  
   initialize_timer ();
     printf("initialized timer \n");
   initialize_memory ();
      printf("initialized memory \n");
  initialize_process ();
      printf("initialized process \n");
     //   initialize_memory ();
   // printf("initialized memory \n");
  
}

void initialize_freelist(int i, int j)

{
    int x=0;
    printf("Initializing freelist \n");
    
    NODE *temp;
    int data = (j-i)+1;
    head_freelist = list_create(data);
    
    if(head_freelist==NULL)
        printf(" Head not created\n");
    
    for(x=(j-i+2);x<=j;x++)
    {
	  addNodeBottom(x, head_freelist);
    }
    printf("free list initialized \n");
    
    print_freelist(head_freelist);
    
}

void initialize_swap_freelist(int i, int j)

{
    int x=0;
    printf("Initializing swap space freelist \n");
    
    NODE *temp;
    int data = (j-i)+1;
    head_freelist_swap = list_create(data);
    
    if(head_freelist_swap==NULL)
        printf(" Head not created\n");
    
    for(x=(j-i+2);x<=i;x++)
    {
        addNodeBottom(x, head_freelist_swap);
    }
    printf(" ********** SWAP SPACE FREE LIST ********** \n");
    
    print_freelist(head_freelist_swap);
    
}

void process_command ()
{ char action;
  char fname[100];
  int pid, time, ret;

  printf ("command> ");
  scanf ("%c", &action);
  while (action != 'T')
  { switch (action)
    { case 's':   // submit
        scanf ("%s", &fname);
        if (Debug) printf ("File name: %s is submitted\n", fname);
        submit_process (fname);
        break;
      case 'x':  // execute
        execute_process ();
        break;
      case 'r':  // dump register
        dump_registers ();
        break;
      case 'q':  // dump ready queue and list of processes completed IO
        dump_ready_queue ();
        dump_doneWait_list ();
        break;
      case 'p':   // dump PCB
        printf ("PCB Dump Starts: Checks from 0 to %d\n", currentPid);
        for (pid=1; pid<currentPid; pid++)
          if (PCB[pid] != NULL) dump_PCB (pid);
        break;
      case 'e':   // dump events in timer
        dump_events ();
        break;
      case 'm':   // dump Memory
        for (pid=1; pid<currentPid; pid++)
          if (PCB[pid] != NULL) dump_memory (pid);
        break;
      case 'w':   // dump Swap Space
         for (pid=1; pid<currentPid; pid++)
            if (PCB[pid] != NULL) dump_swap_memory (pid);
        break;
      case 'l':   // dump Spool
        for (pid=1; pid<currentPid; pid++)
          if (PCB[pid] != NULL) dump_spool (pid);
        break;
      case 'T':  // Terminate, do nothing, terminate in while loop
        break;
      default: 
        printf ("Incorrect command!!!\n");
    }
    printf ("\ncommand> ");
    scanf ("\n%c", &action);
    if (Debug) printf ("Next command is %c\n", action);
  }
}


void main ()
{
  initialize_system ();
  process_command ();
}
