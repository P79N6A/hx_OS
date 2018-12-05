#include "sched.h"
#include "heap.h"
#include "debug.h"

extern uint32_t kern_stack_top;
// 可调度进程链表
struct task_struct *running_proc_head = NULL;

// 等待进程链表
struct task_struct *wait_proc_head = NULL;

// 当前运行的任务
struct task_struct *current = NULL;

void init_sched()
{
        // 为当前执行流创建信息结构体 该结构位于当前执行流的栈最低端
        current = (struct task_struct *)(kern_stack_top - STACK_SIZE);

        current->state = TASK_RUNNABLE;
        current->pid = now_pid++;
        current->stack = current; 	// 该成员指向栈低地址
        current->mm = NULL; 		// 内核线程不需要该成员

        // 单向循环链表
        current->next = NULL;

        running_proc_head = current;
}

void schedule()
{

   struct task_struct *tail = NULL;
//    struct task_struct *prev = current;
    if(current->next == NULL) {
        current->next = wait_proc_head;

          tail =  wait_proc_head;
         while (tail->next != NULL){
             tail=tail->next;
         }
         tail->next = current;
          wait_proc_head = wait_proc_head->next;
           current->next->next =NULL;

    }

//            current = current->next;
//            prev->next = NULL;

//            switch_to(&(prev->context), &(current->context));
    if (current) {
                    change_task_to(current->next);
            }


}

void change_task_to(struct task_struct *next)
{
        if (current != next) {
                struct task_struct *prev = current;

                current = next;
                prev->next =NULL;
                switch_to(&(prev->context), &(current->context));
        }
}

