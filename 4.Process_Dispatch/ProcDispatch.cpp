/******************************************************
    > COPYRIGHT
    > AUTHOR：Helson.S
    > DATE：2020/5/7
    > CONTENT：操作系统——进程调度实验
    @ Operate System：Ubuntu18.04
    @ C Library：Glibc2.31
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define NDEBUG
#define MAXQSIZE 6000
// #define algorithm "FCFS"
#define algorithm "SJF"
// #define algorithm "FPF"
#define Global_task_count 2
#define FirstTask_submitNbegin_time 8
#define submit_time_deviation 0.4
#define random_runtime_upperLimit 5
#define priority_level_count 10 //the lowest priority

typedef struct task
{
    int task_num;
    double submit_time;
    double run_time;
    double start_time;
    double wait_time;
    double accomplish_time;
    double total_time;
    double weght_total_time;
    unsigned int priority;
}task;

typedef struct task_Queue
{
    task **base;
    int front;
    int rear;
    int task_count;
}task_Queue;

typedef struct task_seqList_Node
{
    task *ptr_task;
    task_seqList_Node *next;
}task_seqList_Node;

typedef struct task_sqList
{
    task_seqList_Node *head;
    task_seqList_Node *rear;
    int task_count; //head Node uncount
}task_sqList;

void initQueue(task_Queue *task_Queue)
{
    task_Queue->base = (task **)malloc(sizeof(task *)*MAXQSIZE);
    if(!task_Queue->base)
    {
        perror("malloc error!\n");
        exit(1);
    }
    task_Queue->front=task_Queue->rear=0;
    task_Queue->task_count = 0;
}

void InQueue(task_Queue *task_Queue, task *ptr_task)
{
    if((task_Queue->rear + 1)%MAXQSIZE == task_Queue->front)
    {
        perror("task Queue is full!\n");
        exit(1);
    }
    task_Queue->base[task_Queue->rear] = ptr_task;
    task_Queue->rear = (task_Queue->rear + 1)%MAXQSIZE;
    task_Queue->task_count++;
}

task *OutQueue(task_Queue *task_Queue)
{
    task *out_element_ptr;
    if(task_Queue->front == task_Queue->rear)
    {
        perror("task Queue is empty!\n");
        exit(1);
    }
    out_element_ptr = task_Queue->base[task_Queue->front];
    task_Queue->front = (task_Queue->front + 1)%MAXQSIZE;
    task_Queue->task_count--;
    return out_element_ptr;
}

void Insert_sqList(task_sqList *task_sqList, task *param_task)
{
    if(task_sqList->task_count == 0)
    {
        // It means that task_sqList is empty.
        task_seqList_Node *tmp;
        tmp = (task_seqList_Node *)malloc(sizeof(task_seqList_Node)*1);
        if(!tmp)
        {
            perror("malloc error!\n");
            exit(1);
        }
        tmp->ptr_task = param_task;
        tmp->next = NULL;
        task_sqList->head->next = tmp;
        task_sqList->rear = tmp;
        task_sqList->task_count++;
        return; //done, Insert the first Node.
    }
    /**
     * while task_sqList have at least one Node.
     */
    task_seqList_Node *prev_ptr, *ptr;
    if(algorithm == "SJF")
    {
        for(prev_ptr = task_sqList->head, ptr = task_sqList->head->next; ptr && ptr->ptr_task->run_time<=param_task->run_time;)
        {
            prev_ptr = ptr;
            ptr = ptr->next;
        }
    }
    else if(algorithm == "FPF")
    {
        for(prev_ptr = task_sqList->head, ptr = task_sqList->head->next; ptr && ptr->ptr_task->priority<=param_task->priority;)
        {
            prev_ptr = ptr;
            ptr = ptr->next;
        }
    }
    task_seqList_Node *tmp;
    tmp = (task_seqList_Node *)malloc(sizeof(task_seqList_Node)*1);
    if(!tmp)
    {
        perror("malloc error!\n");
        exit(1);
    }
    tmp->ptr_task = param_task;
    tmp->next = ptr;
    prev_ptr->next = tmp;
    if(!ptr)
    {
        task_sqList->rear = tmp;
    }
    task_sqList->task_count++;
    return;
}

void Insert_sqList_rightBack(task_sqList *task_sqList, task *param_task)
{
    task_seqList_Node *tmp,*position;
    tmp = (task_seqList_Node *)malloc(sizeof(task_seqList_Node)*1);
    if(!tmp)
    {
        perror("malloc error!\n");
        exit(1);
    }
    tmp->ptr_task = param_task;
    tmp->next = NULL; //as a rear Node.
    for(position = task_sqList->head; position->next;)
    {
        position = position->next;
    }
    position->next = tmp;
    task_sqList->rear = tmp;
    task_sqList->task_count++;
}

task* Out_sqList_from_head(task_sqList *param_task_sqList)
{
    task_seqList_Node *ptr;
    task *result;
    ptr = param_task_sqList->head->next;
    param_task_sqList->head->next = ptr->next;
    result = ptr->ptr_task;
    free(ptr);
    param_task_sqList->task_count--;
    return result;
}

void init_TaskList(task *task, task_Queue *task_Queue)
{
    /**
     * We do 2 things in Function initTaskList
     * 1、set buf of task and fill some data in it.
     * 2、put the task in Queue.
     */
    srand(time(NULL));
    for(int i=0; i<Global_task_count; i++)
    {
        task[i].task_num = i+1;
        task[i].submit_time = FirstTask_submitNbegin_time + (submit_time_deviation*i); 
        task[i].run_time = double( rand() % (random_runtime_upperLimit*10) )/10.0;
        // task[i].run_time = 0;
        task[i].start_time = 0;
        task[i].wait_time = 0;
        task[i].accomplish_time = 0;
        task[i].total_time = 0;
        task[i].weght_total_time = 0;
        task[i].priority = rand() % priority_level_count;
        InQueue(task_Queue,&task[i]);
    }
    return;
}

void init_taskReady_sqList(task_sqList *task_sqList, task_Queue *task_Queue)
{
    task_sqList->head = (task_seqList_Node *)malloc(sizeof(task_seqList_Node)*1);
    if(!task_sqList->head)
    {
        perror("malloc error!\n");
        exit(1);
    }
    task_sqList->head->next = NULL;
    task_sqList->task_count = 0;
    task_sqList->rear = task_sqList->head;
    assert(task_Queue->front != task_Queue->rear);
    Insert_sqList(task_sqList,OutQueue(task_Queue));
}

bool renew_sqList(task_sqList *param_task_sqList, task_Queue *param_task_Queue, double threshold)
{
    task *tmp;
    if(param_task_Queue->front == param_task_Queue->rear)
    {
        return false;
    }
    else
    {
        while(param_task_Queue->front != param_task_Queue->rear && param_task_Queue->base[param_task_Queue->front]->submit_time < threshold)
        {
            if(algorithm == "SJF" || algorithm == "FPF")
            {
                tmp = OutQueue(param_task_Queue);
                Insert_sqList(param_task_sqList,tmp);
            }
            else if(algorithm == "FCFS")
            {
                tmp = OutQueue(param_task_Queue);
                Insert_sqList_rightBack(param_task_sqList,tmp);
            }
        }
        return true; //The Queue can put task into Link list.
    }
}

void Processing_unit(task_sqList *param_task_sqList, task_Queue *param_task_Queue)
{
    static double old_accomplish_Time = FirstTask_submitNbegin_time;
Execution:
    while(param_task_sqList->head->next != NULL)
    {
        task *current_task; //current task is current task to process.
        current_task = Out_sqList_from_head(param_task_sqList);
        double random_value;
        double var_threshold;
        /**************************************** value simulation ********************************/
        current_task->start_time = old_accomplish_Time;
        current_task->wait_time = current_task->start_time - current_task->submit_time;
        // srand(time(NULL));
        // random_value = double(rand()%30)/10.0;
        // current_task->run_time = random_value;
        current_task->accomplish_time = current_task->start_time + current_task->run_time;
        old_accomplish_Time = current_task->accomplish_time; //renew old accomplish time
        current_task->total_time = current_task->accomplish_time - current_task->submit_time;
        current_task->weght_total_time = current_task->total_time/current_task->run_time;
        /**************************************** value simulation *********************************/
        var_threshold = current_task->submit_time + current_task->run_time;
        printf("threshold:%f\n",var_threshold);
        renew_sqList(param_task_sqList,param_task_Queue,var_threshold);
    }
    if(param_task_Queue->front != param_task_Queue->rear)
    {
        Insert_sqList(param_task_sqList,OutQueue(param_task_Queue));
        old_accomplish_Time = param_task_Queue->base[param_task_Queue->front]->submit_time;
        goto Execution;
    }
    return;
}

static inline void print_performance_information(task *task_array)
{
    printf("\n");
    for(int i=0; i<Global_task_count; i++)
    {
        printf("任务编号:%d\n",task_array[i].task_num);
        printf("提交时间:%f\n",task_array[i].submit_time);
        printf("开始时间:%f\n",task_array[i].start_time);
        printf("运行时间:%f\n",task_array[i].run_time);
        printf("等待时间:%f\n",task_array[i].wait_time);
        printf("完成时间:%f\n",task_array[i].accomplish_time);
        printf("周转时间:%f\n",task_array[i].total_time);
        printf("带权周转时间:%f\n",task_array[i].weght_total_time);
        printf("任务优先等级:%d\n",task_array[i].priority);
        printf("任务优先等级为0级(最高)，到%d级(最低)\n",priority_level_count);
        printf("\n");
    }
}

int main()
{
    task var_task[Global_task_count];
    task_Queue var_taskQueue;
    task_sqList var_sqList;
    initQueue(&var_taskQueue);
    init_TaskList(var_task,&var_taskQueue); //Put the task in Queue;
    init_taskReady_sqList(&var_sqList,&var_taskQueue);
    Processing_unit(&var_sqList,&var_taskQueue);
    print_performance_information(var_task);
    return 0;
}