//================= general definitions =========================

#define Debug 1
#define num_pages 10000
typedef unsigned *genericPtr;
          // when passing pointers externally, use genericPtr
          // to avoid the necessity of exposing internal structures


//============ sytem configuration parameters ===================
int Observe;  // whether to print more information for better observation
              // = 0 turn off observation mode; = 1 turn on

int pageSize, memSize, swapSize, OSmemSize;
                   // sizes related to memory and memory management
int periodAgeScan; // the period for scanning and shifting the age vectors
                   // defined in # instruction-cycles

int cpuQuantum;    // time quantum, defined in # instruction-cycles
int idleQuantum;   // time quantum for the idle process

int spoolPsize;    // spool space size for each process
//int num_pages=0;

int rem_pages, rem_pages_swapspace;
int OriginalPages,OSpages, idlePages, OriginalPages_swapspace;
int process_counter;
typedef struct node_s {
	int *frame_no;
	struct node_s *next;
} NODE;

NODE *list_create(int);
int list_remove(NODE *list, NODE *node);
int addNodeBottom(int val, NODE *head);
NODE *addNodeTop(int val, NODE *head);
int find_pages(int, int);
void print_freelist(NODE*);
void calculate_physical_addr(int);
void calculate_physical_addr_swap(int);
void remove_frame(int, int);
NODE *head_freelist, *tail_freelist;
NODE *head_freelist_swap, *tail_freelist_swap;

int phy_addr[2], phy_addr_swap[2];


typedef struct node {
        char age;
        int dirty;
} type_AGE_Dirty;

type_AGE_Dirty *AGE[10000];

void remove_page(int);
//================= CPU related definitions ======================


struct
{ int Pid;
  int PC;
  float AC;
  float MBR;
  int IRopcode;
  int pages_per_process;
  int IRoperand;
  int mem_locs[100000];  
  int mem_locs_swap[100000];
  int *pageTableptr;
  int *pageTableptr_swap;
  int Mbase;
  int MDbase;
  int Mbound;
  char *spoolPtr; 
  int spoolPos; 
  int exeStatus;
  unsigned interruptV;
  int numCycles;  // this is a global register, not for each process
} CPU;


// define interrupt set bit for interruptV in CPU structure
// 1 means bit 0, 4 means bit 2, ...

#define tqInterrupt 1      // for time quantum
#define ageInterrupt 2     // for age scan
#define doneWaitInterrupt 4  // for any IO completion, including page fault
        // before setting doneWait, caller should add the pid to doneWait list


// define exeStatus in CPU structure
#define eRun 1
#define eReady 2
#define ePFault 3
#define eWait 4
#define eEnd 0
#define eError -1


// definition related to numCycles
#define maxCPUcycles 1024*1024*1024 // = 2^30


// cpu function definitions

void initialize_cpu ();
void dump_registers ();
void cpu_execution ();

void set_interrupt (unsigned bit);

#define maxProcess 1024
#define fixMsize 256
#define MemSize maxProcess*fixMsize


//=============== process related definitions ====================

typedef struct
{ int Pid;
  int PC;
  float AC;
  int pages_per_process;
  int *pageTableptr;
  int *pageTableptr_swap;
  int mem_locs[100000];
  int mem_locs_swap[100000];
  int counter;
  int flag;
  int page_counter;
  int Mbase;
  int MDbase;
  int Mbound;
  char *spoolPtr;
  int spoolPos;
  int exeStatus;
  int numInstr;
  int numStaticData;
  int numData;
      // numData is not in use anywhere
      // it is useful if there is dynamic space used during run time
      // but here we do not consider it
  int timeUsed;
} typePCB;


typePCB *PCB[maxProcess];
  // the system can have #maxProcess processes,
  // first one is OS, so, pid of any user process starts from 1
  // each process get a PCB, allocate PCB space upon process creation

#define osPid 0
#define idlePid 1
int currentPid;    // user pid should start from 2, pid=0/1 are defined above


// define process manipulation functions

void dump_PCB (int pid);
void dump_ready_queue ();

void insert_doneWait_process (int pid);
void dump_doneWait_list ();

void initialize_process ();
void submit_process (char* fname);
void execute_process ();



//=============== memory related definitions ====================

typedef union
{ float mData;
  int mInstr;
} mType;

#define SwapSize 10000
mType SwapSpace[SwapSize];

mType Memory[MemSize];


#define mNormal 0
#define mError -1
#define mPFault 1

// memory related function definitions

int get_data (int offset);
int put_data (int offset);
int put_data_swap(int);

int get_instruction (int offset);

int load_instruction (int pid, int offset, int*);
int load_data (int pid, int offset,int*);

int load_instruction_idle (int pid, int offset, int opcode, int operand);
int load_data_idle (int pid, int offset, float data);


int load_instruction_swap (int, int , int, int );
int load_data_swap (int , int , float );
void load_RAM(int);

void memory_agescan ();
void init_pagefault_handler (int pid);
void handle_pagefault(int);

int allocate_memory (int pid, int);
int allocate_swap_memory(int pid,int msize,int numinstr,int);
int free_memory (int pid);
int free_swap_memory (int pid);

int search_for_page(int, int);
void initialize_memory ();
void dump_memory (int pid);

void dump_swap_memory (int pid);

void initialize_freelist(int, int);
void initialize_swap_freelist(int, int);
int  allocate_pages(int, int);



//=============== timer related definitions ====================

#define oneTimeTimer 0

// define the action codes for timer
#define actTQinterrupt 1
#define actAgeInterrupt 2
#define actReadyInterrupt 3
#define actNull 0


// define the timer functions 

void dump_events ();

void initialize_timer ();
genericPtr add_timer (int time, int pid, int action, int recurperiod);
void check_timer ();
void deactivate_timer (genericPtr castedevent);



//=============== spooler related definitions ====================

#define spNormal 0
#define spError -1

#define prNormal 0
#define prError -1


// define the spooler functions 

int spool (char* str);

void allocate_spool (int pid);
void free_spool (int pid);
void print_spool (int pid);

void dump_spool (int pid);

int printer (int pid, int status, char* prstr, int len);



