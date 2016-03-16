#include <linux/printk.h>
#include "sched.h"
#include <linux/syscalls.h>


/*
 * The enqueue_task method is called before nr_running is
 * increased. Here we update the fair scheduling stats and
 * then put the task into the rbtree:
 */

static void
enqueue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{

}

/*
 * The dequeue_task method is called before nr_running is
 * decreased. We remove the task from the rbtree and
 * update the fair scheduling stats:
 */

static void dequeue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{

}


/*
 * sched_yield() is very simple
 *
 * The magic of dealing with the ->skip buddy is in pick_next_entity.
 */
static void yield_task_mycfs(struct rq *rq)
{

}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{

}

static struct task_struct *pick_next_task_mycfs(struct rq *rq)
{

}

/* Account for a task changing its policy or group.
 *
 * This routine is mostly called to set cfs_rq->curr field when a task
 * migrates between groups/classes.
 */
static void set_curr_task_mycfs(struct rq *rq)
{

}
static bool yield_to_task_mycfs(struct rq *rq, struct task_struct *p, bool preempt)
{
}

/*
 * Account for a descheduled task:
 */
static void put_prev_task_mycfs(struct rq *rq, struct task_struct *prev)
{
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
}


static void rq_online_mycfs(struct rq *rq)
{
}
static void rq_offline_mycfs(struct rq *rq)
{
}
static void task_waking_mycfs(struct task_struct *p)
{
}
/*
 * scheduler tick hitting a task of our scheduling class:
 */
static void task_tick_mycfs(struct rq *rq, struct task_struct *curr, int queued)
{}
/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_mycfs(struct task_struct *p)
{}

/*
 * Priority of the task has changed. Check to see if we preempt
 * the current task.
 */
static void
prio_changed_mycfs(struct rq *rq, struct task_struct *p, int oldprio)
{}

static void switched_from_mycfs(struct rq *rq, struct task_struct *p)
{}
/*
 * We switched to the sched_fair class.
 */
static void switched_to_mycfs(struct rq *rq, struct task_struct *p)
{}

static unsigned int get_rr_interval_mycfs(struct rq *rq, struct task_struct *task)
{}

static void task_move_group_mycfs(struct task_struct *p, int on_rq)
{}

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

