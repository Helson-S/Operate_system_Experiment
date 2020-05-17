/******************************************************
    > COPYRIGHT
    > AUTHOR：Helson.S
    > DATE：2020/5/6
    > CONTENT：操作系统——存储管理实验
    @ Operate System：Ubuntu18.04
    @ C Library：Glibc2.31
*******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <error.h>
#define NDEBUG
#include <assert.h>

#define MAXQSIZE 1000
#define algorithm "LRU"
// #define algorithm "FIFO"
#define var_Instruction_perPage 10
#define var_program_instruction 320
#define allocate_page ((var_program_instruction/var_Instruction_perPage)+1)
#define physical_block_count 4
#define Execute_controler 10000
#define Test_times 2000

typedef struct cycle_Queue
{
    int *base;
    int front;
    int rear;
}cycle_Queue;

typedef struct page{
    int page_num;
    int instruction[var_Instruction_perPage];
    bool status;
    unsigned int time; // used by LRU
    long * pysical_block;
}page;

int var_physical_block;
int page_fault_count = 0;
unsigned int single_Execute_Counter = 0; // Used by Simulate_Execution,counter of single Execution.
bool Queue_init_check = 0; // Used by initQueue. Mark, if Queue is initiated.
cycle_Queue var_queue;

void physical_block_timeChcker(page *array_page)
{
    var_physical_block = 0;
    for(int i=0; i<allocate_page; i++)
    {
        if(array_page[i].status == true)
        {
            array_page[i].time++;
            var_physical_block++;
            #ifdef DEBUG
            printf("page %d is mapped.\n",i);
            #endif
        }
        else
        {
            array_page[i].time = 0;
        }
    }
    #ifdef DEBUG
    printf("now the var_physical_block is %d.\n",var_physical_block);
    #endif
}

void initQueue(cycle_Queue *ptr_Queue)
{
    if(!Queue_init_check)
    {
        ptr_Queue->base = (int *)malloc(sizeof(int)*MAXQSIZE);
        if(!ptr_Queue->base)
        {
            perror("Queue malloc error!");
            exit(1);
        }
        ptr_Queue->front = ptr_Queue->rear = 0;
        Queue_init_check = true;
    }
}

void DeleteQueue(cycle_Queue *ptr_Queue)
{
    if(Queue_init_check != true)
    {
        perror("can't free a Queue which isn't initiated.\n");
        exit(1);
    }
    free(ptr_Queue->base);
    ptr_Queue->base = NULL;
    Queue_init_check = false;
}

void InQueue(cycle_Queue *ptr_Queue, int page_num)
{
    if((ptr_Queue->rear + 1)%MAXQSIZE == ptr_Queue->front)
    {
        perror("Queue is full!");
        exit(1);
    }
    ptr_Queue->base[ptr_Queue->rear] = page_num;
    ptr_Queue->rear = (ptr_Queue->rear + 1)%MAXQSIZE;
}

void OutQueue(cycle_Queue *ptr_Queue, int &outElem)
{
    if(ptr_Queue->front == ptr_Queue->rear)
    {
        perror("Queue is empty!");
        exit(1);
    }
    outElem = ptr_Queue->base[ptr_Queue->front];
    ptr_Queue->front = (ptr_Queue->front + 1)%MAXQSIZE;
}

void initial_page(page *array_page)
{
    int page_instruction_num = 0;
    for(int i=0; i<allocate_page; i++)
    {
        array_page[i].page_num = i;
        for(int j = 0; j < var_Instruction_perPage; j++)
        {
            array_page[i].instruction[j] = page_instruction_num;
            page_instruction_num++;
        }
        array_page[i].status = false;
        array_page[i].time = 0;
        array_page[i].pysical_block = NULL;
    }
    assert(page_instruction_num == var_Instruction_perPage*allocate_page);
}

int Instruction_Generator(int last_instrion, int program_instruction)
{
    int choice = rand()%4 + 1;
    int Gnerate_Instruction;
    int selection_count;
    switch (choice)
    {
    case 1:
        /**
        Behavior NextInstruction
        */
       if(last_instrion >= program_instruction)
            goto backward;
       Gnerate_Instruction = last_instrion + 1;
       #ifdef DEBUG
       printf("[+] done,next instruction\n");
       #endif
       break;

    case 2:
        /**
        Behavior NextInstruction
        */
        if(last_instrion >= program_instruction)
            goto backward;
        Gnerate_Instruction = last_instrion + 1;
        #ifdef DEBUG
        printf("[+] done,next instruction\n");
        #endif
        break;

    case 3:
        /**
        Behavior BackwardInstruction
        */
backward:
        if(last_instrion == 0)
            goto forward;
        Gnerate_Instruction = rand()%last_instrion;
        #ifdef DEBUG
        printf("[+] done,seek backward\n");
        #endif
        break;

    case 4:
        /**
        Behavior FrontwardInstruction
        */
forward:
        selection_count = program_instruction-(last_instrion+1);
        if(selection_count == 0)
        {
            Gnerate_Instruction = program_instruction;
            #ifdef DEBUG
            printf("[+] done,seek forward\n");
            #endif
            break;
        }
        else
        {
        /* selection_count can not be negative */
        if(selection_count < 0)
            goto backward;
        /* selection_count is positive */
        Gnerate_Instruction = last_instrion + 1 + rand()%(selection_count);
        #ifdef DEBUG
        printf("[+] done,seek forward\n");
        #endif
        break;
        }

    default:
        perror("choice error!");
        exit(1);
        break;
    }
    return Gnerate_Instruction;
}

void processing_unit(page *array_page, int instruction_num)
{
    physical_block_timeChcker(array_page);
    int hit_page= 0;
    int hit_page_idx = 0;
    for(int i=0; i<allocate_page; i++)
    {
        for(int j=0; j<var_Instruction_perPage; j++)
        {
            if(instruction_num == array_page[i].instruction[j])
                {
                    #ifdef DEBUG
                    printf("hit page:%d\n",array_page[i].page_num);
                    printf("hit page index:%d\n",i);
                    #endif
                    hit_page = array_page[i].page_num;
                    hit_page_idx = i;
                    goto found_hitpage;
                }
        }
    }
    perror("page no found!");
    exit(1);
found_hitpage:
    if(array_page[hit_page_idx].status==1)
    {
        assert(var_physical_block <= physical_block_count);
    }
    else
    {
        page_fault_count++;
        #ifdef DEBUG
        printf("page fault occur!\n");
        #endif
        if(var_physical_block == physical_block_count)
        {
            if(algorithm == "LRU")
            {
                int max_time_page = 0;
                for(int i=0; i<allocate_page; i++)
                {
                    if(array_page[i].time > max_time_page)
                        max_time_page = array_page[i].page_num;
                }
                array_page[max_time_page].status = 0; //swap out
            }
            else if(algorithm == "FIFO")
            {
                assert(var_queue.front != var_queue.rear);
                int swapout_page_idx;
                OutQueue(&var_queue,swapout_page_idx);
                array_page[swapout_page_idx].status = 0; //swap out
            }
        }
        array_page[hit_page].status = 1; //swap in
        if(algorithm == "FIFO")
        {
            initQueue(&var_queue);
            InQueue(&var_queue,hit_page_idx);
        }
        assert(var_physical_block <= physical_block_count);
    }
}

double Simulate_Execution(page *array_page, int stru_num_begin)
{
    /* static unsigned int Execute_counter = 0; */
    static double hit_rate;
    if(single_Execute_Counter <= Execute_controler)
    {
        processing_unit(array_page,stru_num_begin);
        int stru_next = Instruction_Generator(stru_num_begin,var_program_instruction);
        single_Execute_Counter++;
        Simulate_Execution(array_page,stru_next);
    }
    else
    {
        hit_rate = double(Execute_controler-page_fault_count)/double(Execute_controler);
        printf("\nLast time page_fault_count: %d\n",page_fault_count);
        /* next is finishing work , do some clearation */
        single_Execute_Counter = 0;
        page_fault_count = 0;
        if(algorithm == "FIFO")
        {
            DeleteQueue(&var_queue);
        }
    }
    return hit_rate;
}

int main()
{
    double total_hit_rate = 0;
    double average_hit_rate = 0;
    srand(time(NULL));
    for(int i=0; i<Test_times; i++)
    {
        /* Initialization */
        page var_array_page[allocate_page];
        initial_page(var_array_page);
        /* Simulate Execution */
        double hit_rate;
        hit_rate = Simulate_Execution(var_array_page,20);
        printf("Hit rate:%f\n",hit_rate);
        total_hit_rate += hit_rate;
    }
    average_hit_rate = total_hit_rate/Test_times;
    printf("\naverage Hit rate:%f\n\n",average_hit_rate);
    return 0;
}