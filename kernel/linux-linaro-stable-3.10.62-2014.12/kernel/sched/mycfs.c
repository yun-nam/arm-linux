#include <linux/printk.h>
#include "sched.h"
#include <linux/syscalls.h>

#define LIST_SIZE 3000
static struct sched_entity * schedule_list[LIST_SIZE];
static int pick_point=-1;
static int end=0;


static inline struct task_struct *task_of(struct sched_entity *se)
 {
 	return container_of(se, struct task_struct, se);
 }
static void resched_task_mycfs(struct task_struct * p){
	resched_task(p);
}

/*
 * The enqueue_task method is called before nr_running is
 * increased. Here we update the fair scheduling stats and
 * then put the task into the rbtree:
 */

static void
enqueue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
	struct cfs_rq *cfs_rq;
	struct sched_entity *se = &p->se;
	//if (se->on_rq)
	//	break;
	

	printk(KERN_EMERG "[YUN Debug]: enqueue_task_mycfs\n");
	//update_rq_runnable_avg(rq, rq->nr_running);
	if (!se) {
		//update_rq_runnable_avg(rq, rq->nr_running);
		schedule_list[end]=&p->se;
		end++;
		inc_nr_running(rq);
	}

}

/*
 * The dequeue_task method is called before nr_running is
 * decreased. We remove the task from the rbtree and
 * update the fair scheduling stats:
 */

static void dequeue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
	struct cfs_rq *cfs_rq;
	struct sched_entity *se = &p->se;
	int i;
	printk(KERN_EMERG "[YUN Debug]: dequeue_task_mycfs\n");
	//find the task
	for (i=0; i < end; i++){
		if (schedule_list[i]==se)
			break;
	}
	// move tasks in the list
	if(i<end){
		for (;i<end;i++){
			schedule_list[i]=schedule_list[i+1];
		}
		schedule_list[i]=0;
		end--;
	}
	else{
		printk(KERN_EMERG "[YUN Debug]: dequeue_task_mycfs : cannot find task in the list\n");
		BUG();		
	}
	dec_nr_running(rq);
}


/*
 * sched_yield() is very simple
 *
 * The magic of dealing with the ->skip buddy is in pick_next_entity.
 */
static void yield_task_mycfs(struct rq *rq)
{
	printk(KERN_EMERG "[YUN Debug]: yield_task_mycfs\n");
}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup_mycfs(struct rq *rq, struct task_struct *p, int wake_flags)
{
	printk(KERN_EMERG "[YUN Debug]: check_preempt_wakeup_mycfs\n");
	struct task_struct *curr = rq->curr;
	struct sched_entity *cse = &curr->se, *pse = &p->se;
	if (cse == pse) return;
	resched_task_mycfs(curr);
}
/*
 * Idle tasks are unconditionally rescheduled:
 */
static void check_preempt_curr_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
	printk(KERN_EMERG "[YUN Debug]: check_preempt_curr_mycfs\n");
	struct task_struct *curr = rq->curr;
	struct sched_entity *se = &curr->se, *pse = &p->se;
	
	if (se == pse) return;
	
	resched_task_mycfs(curr);
}
/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{}

static struct task_struct *pick_next_task_mycfs(struct rq *rq)
{
/*
	return cpu_rq(rq->cpu)->idle;
	printk(KERN_EMERG "[YUN Debug]: pick_next_task_mycfs\n");
	struct task_struct *p; 
	struct task_struct *curr_task = rq->curr;
	if (end > 0 && end < LIST_SIZE){
		if (pick_point==-1 && schedule_list[0]!=0){
			pick_point =0;
			p = task_of(schedule_list[0]);
			return p;
		}
		else{
			//BUG();
			if (end-1 != pick_point){
				if (schedule_list[pick_point+1]!=0){
					pick_point++;
					p = task_of(schedule_list[pick_point]);
					return p;
				}else{
					return NULL;
					//BUG();
				}				
			}else{ //end-1 == pick_point
				if (schedule_list[0]!=0){
					pick_point =0;
					p = task_of(schedule_list[0]);
					return p;
				}else{
					BUG();
				}
			}
		}

	}
	else{
		BUG();
	}
*/

	struct task_struct *p; 
      struct task_struct *curr_task = rq->curr;
      int i=0;
  
        if (end >= 0 && end < LIST_SIZE){
          
            if (pick_point == -1){
                // first time pick the first task in the list
                
                for (i=0; i< LIST_SIZE; i++){
                    // returning the first task in the list
                    if (schedule_list[i] != 0) {
                        pick_point = i;
                        p = task_of(schedule_list[i]);
                        printk(KERN_EMERG "[YUN Debug] picked task (first) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
                            ,p->comm, i, (int) p->pid, cpu_of(rq),pick_point);
                        return p;
                    }            
                }
                
            }
            else{
                // pick other tasks. 
                for (i=(pick_point+1); i< LIST_SIZE; i++){
                    // returning the first task in the list after head
                    if (schedule_list[i] != 0) {
                        pick_point = i;
                        p = task_of(schedule_list[i]);
                        printk(KERN_EMERG "[YUN Debug] picked task (head-start-other) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
                            ,p->comm, i, (int) p->pid, cpu_of(rq),pick_point);
                        return p;
                    }            
                }
                
                for (i=0; i< pick_point; i++){
                    // returning the first task in the list after head
                    if (schedule_list[i] != 0) {
                        pick_point = i;
                        p = task_of(schedule_list[i]);
                        printk(KERN_EMERG "[YUN Debug] picked task (head-start-other) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
                            ,p->comm, i, (int) p->pid, cpu_of(rq),pick_point);
                        return p;
                    }            
                }
                
                
                // no other tasks, return same task if valid
                if (schedule_list[pick_point] != 0){
                    p = task_of(schedule_list[pick_point]);
                    printk(KERN_EMERG "[YUN Debug] picked task (no-other) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
                            ,p->comm, i, (int) p->pid, cpu_of(rq),pick_point);
                    return p;
                }
                
                
            }
            return NULL;
          
          }
        else{
            printk(KERN_EMERG "[YUN Debug] end at(%d) is invalid\n");
            BUG();
          }

}

/* Account for a task changing its policy or group.
 *
 * This routine is mostly called to set cfs_rq->curr field when a task
 * migrates between groups/classes.
 */
static void set_curr_task_mycfs(struct rq *rq)
{
	printk(KERN_EMERG "[YUN Debug]: set_curr_task_mycfs\n");

}
static bool yield_to_task_mycfs(struct rq *rq, struct task_struct *p, bool preempt)
{
	printk(KERN_EMERG "[YUN Debug]: yield_to_task_mycfs\n");
	return false;
}

/*
 * Account for a descheduled task:
 */
static void put_prev_task_mycfs(struct rq *rq, struct task_struct *prev)
{
	printk(KERN_EMERG "[YUN Debug]: put_prev_task_mycfs\n");
}

/*
 * sched_balance_self: balance the current task (running on cpu) in domains
 * that have the 'flag' flag set. In practice, this is SD_BALANCE_FORK and
 * SD_BALANCE_EXEC.
 *
 * Balance, ie. select the least loaded group.
 *
 * Returns the target CPU number, or the same CPU if no balancing is needed.
 *
 * preempt must be disabled.
 */
static int
select_task_rq_mycfs(struct task_struct *p, int sd_flag, int wake_flags)
{
	printk(KERN_EMERG "[YUN Debug]: select_task_rq_mycfs\n");
	return 0;
}

/*
 * Called immediately before a task is migrated to a new cpu; task_cpu(p) and
 * cfs_rq_of(p) references at time of call are still valid and identify the
 * previous cpu.  However, the caller only guarantees p->pi_lock is held; no
 * other assumptions, including the state of rq->lock, should be made.
 */
static void
migrate_task_rq_mycfs(struct task_struct *p, int next_cpu)
{
	printk(KERN_EMERG "[YUN Debug]: migrate_task_rq_mycfs\n");
}


static void rq_online_mycfs(struct rq *rq)
{
	printk(KERN_EMERG "[YUN Debug]: rq_online_mycfs\n");
}
static void rq_offline_mycfs(struct rq *rq)
{
	printk(KERN_EMERG "[YUN Debug]: rq_offline_mycfs\n");
}
static void task_waking_mycfs(struct task_struct *p)
{
	printk(KERN_EMERG "[YUN Debug]: task_waking_mycfs\n");
}
/*
 * scheduler tick hitting a task of our scheduling class:
 */
static void task_tick_mycfs(struct rq *rq, struct task_struct *curr, int queued)
{
	printk(KERN_EMERG "[YUN Debug]: task_tick_mycfs\n");
	if (rq->curr == curr)
		resched_task_mycfs(curr);
	else resched_task_mycfs(rq->curr);

}
/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_mycfs(struct task_struct *p)
{
	printk(KERN_EMERG "[YUN Debug]: task_fork_mycfs\n");
}

/*
 * Priority of the task has changed. Check to see if we preempt
 * the current task.
 */
static void
prio_changed_mycfs(struct rq *rq, struct task_struct *p, int oldprio)
{
	printk(KERN_EMERG "[YUN Debug]: prio_changed_mycfs\n");
}

static void switched_from_mycfs(struct rq *rq, struct task_struct *p)
{
	printk(KERN_EMERG "[YUN Debug]: switched_from_mycfs\n");
}
/*
 * We switched to the sched_fair class.
 */
static void switched_to_mycfs(struct rq *rq, struct task_struct *p)
{
	printk(KERN_EMERG "[YUN Debug]: switched_to_mycfs\n");
	if (!p->se.on_rq)
		return;

	/*
	 * We were most likely switched from sched_rt, so
	 * kick off the schedule if running, otherwise just see
	 * if we can still preempt the current task.
	 */
	if (rq->curr == p){
		printk(KERN_EMERG "handling task %s , reschedule task\n",p->comm);
		resched_task(rq->curr);
	}
	else{
		printk(KERN_EMERG "going to check preempt task %s\n",p->comm);
		check_preempt_curr_mycfs(rq, p, 0);
	}
}

static unsigned int get_rr_interval_mycfs(struct rq *rq, struct task_struct *task)
{
	printk(KERN_EMERG "[YUN Debug]: get_rr_interval_mycfs\n");
	return 0;
}

static void task_move_group_mycfs(struct task_struct *p, int on_rq)
{
	printk(KERN_EMERG "[YUN Debug]: task_move_group_mycfs\n");
}

/*
 * init. for mycfs policy, called in core.c [Naif & Yun]
 */
void init_mycfs_rq(struct mycfs_rq *mycfs_rq)
{
	printk(KERN_EMERG "[YUN Debug]: init_mycfs_rq\n");
	memset(schedule_list,0,sizeof(schedule_list));
        //printk(KERN_EMERG"[Naif Debug]: INSIDE init_mycfs_rq\n");
        // temp begin: to be filled
        mycfs_rq->tasks_timeline = RB_ROOT;
        mycfs_rq->min_vruntime = (u64)(-(1LL << 20));
#ifndef CONFIG_64BIT
        mycfs_rq->min_vruntime_copy = mycfs_rq->min_vruntime;
#endif
#if defined(CONFIG_FAIR_GROUP_SCHED) && defined(CONFIG_SMP)
        atomic64_set(&cfs_rq->decay_counter, 1);
        atomic64_set(&cfs_rq->removed_load, 0);
#endif
                // temp end
}


/*
 * All the scheduling class methods:
 */
const struct sched_class mycfs_sched_class = {
       .next			= &idle_sched_class, 
	.enqueue_task		= enqueue_task_mycfs,
	.dequeue_task		= dequeue_task_mycfs,
	.yield_task		= yield_task_mycfs,
	.yield_to_task		= yield_to_task_mycfs,

	.check_preempt_curr	= check_preempt_wakeup,

	.pick_next_task		= pick_next_task_mycfs,
	.put_prev_task		= put_prev_task_mycfs,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_mycfs,
#ifdef CONFIG_FAIR_GROUP_SCHED
	.migrate_task_rq	= migrate_task_rq_mycfs,
#endif
	.rq_online		= rq_online_mycfs,
	.rq_offline		= rq_offline_mycfs,

	.task_waking		= task_waking_mycfs,
#endif

	.set_curr_task          = set_curr_task_mycfs,
	.task_tick		= task_tick_mycfs,
	.task_fork		= task_fork_mycfs,

	.prio_changed		= prio_changed_mycfs,
	.switched_from		= switched_from_mycfs,
	.switched_to		= switched_to_mycfs,

	.get_rr_interval	= get_rr_interval_mycfs,

#ifdef CONFIG_FAIR_GROUP_SCHED
	.task_move_group	= task_move_group_mycfs,
#endif
};

