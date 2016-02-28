/*
 *  linux/arch/arm/mm/fault.c
 *
 *  Copyright (C) 1995  Linus Torvalds
 *  Modifications for ARM processor (c) 1995-2004 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/perf_event.h>

#include <asm/exception.h>
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <asm/system_info.h>
#include <asm/tlbflush.h>
#include <asm/io.h>
#include "fault.h"
#include <asm/bug.h>
#include <linux/slab.h>


#ifdef CONFIG_MMU

#ifdef CONFIG_KPROBES
static inline int notify_page_fault(struct pt_regs *regs, unsigned int fsr)
{
	int ret = 0;

	if (!user_mode(regs)) {
		/* kprobe_running() needs smp_processor_id() */
		preempt_disable();
		if (kprobe_running() && kprobe_fault_handler(regs, fsr))
			ret = 1;
		preempt_enable();
	}

	return ret;
}
#else
static inline int notify_page_fault(struct pt_regs *regs, unsigned int fsr)
{
	return 0;
}
#endif


pte_t *yun_show_pte(struct mm_struct *mm, unsigned long addr){
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	if (!mm)
		mm = &init_mm;

	//printk(KERN_ALERT "pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
	//printk(KERN_ALERT "[%08lx] *pgd=%08llx",
	//		addr, (long long)pgd_val(*pgd));

//	do {
		//pud_t *pud;
		//pmd_t *pmd;
		//pte_t *pte=NULL;

		if (pgd_none(*pgd))
			return NULL;
			//break;

		if (pgd_bad(*pgd)) {
			printk("(bad)");
			return NULL;
			//break;
		}

		pud = pud_offset(pgd, addr);
		if (PTRS_PER_PUD != 1)
			;
			//printk(", *pud=%08llx", (long long)pud_val(*pud));

		if (pud_none(*pud))
			return NULL;
			//break;

		if (pud_bad(*pud)) {
			printk("(bad)");
			return NULL;
			//break;
		}

		pmd = pmd_offset(pud, addr);
		if (PTRS_PER_PMD != 1)
			;
			//printk(", *pmd=%08llx", (long long)pmd_val(*pmd));

		if (pmd_none(*pmd))
			return NULL;
			//break;

		if (pmd_bad(*pmd)) {
			printk("(bad)");
			return NULL;
			//break;
		}

		/* We must not map this if we have highmem enabled */
		if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
			return NULL;
			//break;

		pte = pte_offset_map(pmd, addr);
		if(!pte) return NULL;

		// check if it is  presented
		if (!pte_present(*pte) ) 
           		return NULL;
           	//if (pte_val(*pte) == 0x00000000)
           	//	return NULL;
		//printk(", *pte=%08llx", (long long)pte_val(*pte));
		
#ifndef CONFIG_ARM_LPAE
		printk(", *ppte=%08llx",
		       (long long)pte_val(pte[PTE_HWTABLE_PTRS]));
#endif
		//pte_unmap(pte);
//	} while(0);

	//printk("\n");
	return pte;
}

/*
 * This is useful to dump out the page tables associated with
 * 'addr' in mm 'mm'.
 */
void show_pte(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;

	if (!mm)
		mm = &init_mm;

	printk(KERN_ALERT "pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
	printk(KERN_ALERT "[%08lx] *pgd=%08llx",
			addr, (long long)pgd_val(*pgd));

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none(*pgd))
			break;

		if (pgd_bad(*pgd)) {
			printk("(bad)");
			break;
		}

		pud = pud_offset(pgd, addr);
		if (PTRS_PER_PUD != 1)
			printk(", *pud=%08llx", (long long)pud_val(*pud));

		if (pud_none(*pud))
			break;

		if (pud_bad(*pud)) {
			printk("(bad)");
			break;
		}

		pmd = pmd_offset(pud, addr);
		if (PTRS_PER_PMD != 1)
			printk(", *pmd=%08llx", (long long)pmd_val(*pmd));

		if (pmd_none(*pmd))
			break;

		if (pmd_bad(*pmd)) {
			printk("(bad)");
			break;
		}

		/* We must not map this if we have highmem enabled */
		if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
			break;

		pte = pte_offset_map(pmd, addr);
		printk(", *pte=%08llx", (long long)pte_val(*pte));
#ifndef CONFIG_ARM_LPAE
		printk(", *ppte=%08llx", (long long)pte_val(pte[PTE_HWTABLE_PTRS]));
#endif
		pte_unmap(pte);
	} while(0);

	printk("\n");
}
#else					/* CONFIG_MMU */
void show_pte(struct mm_struct *mm, unsigned long addr)
{ }
#endif					/* CONFIG_MMU */

/*
 * Oops.  The kernel tried to access some page that wasn't present.
 */
static void
__do_kernel_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr,
		  struct pt_regs *regs)
{
	/*
	 * Are we prepared to handle this kernel fault?
	 */
	if (fixup_exception(regs))
		return;

	/*
	 * No handler, we'll have to terminate things with extreme prejudice.
	 */
	bust_spinlocks(1);
	printk(KERN_ALERT
		"Unable to handle kernel %s at virtual address %08lx\n",
		(addr < PAGE_SIZE) ? "NULL pointer dereference" :
		"paging request", addr);

	show_pte(mm, addr);
	die("Oops", regs, fsr);
	bust_spinlocks(0);
	do_exit(SIGKILL);
}

/*
 * Something tried to access memory that isn't in our memory map..
 * User mode accesses just cause a SIGSEGV
 */
static void
__do_user_fault(struct task_struct *tsk, unsigned long addr,
		unsigned int fsr, unsigned int sig, int code,
		struct pt_regs *regs)
{
	struct siginfo si;

#ifdef CONFIG_DEBUG_USER
	if (((user_debug & UDBG_SEGV) && (sig == SIGSEGV)) ||
	    ((user_debug & UDBG_BUS)  && (sig == SIGBUS))) {
		printk(KERN_DEBUG "%s: unhandled page fault (%d) at 0x%08lx, code 0x%03x\n",
		       tsk->comm, sig, addr, fsr);
		show_pte(tsk->mm, addr);
		show_regs(regs);
	}
#endif

	tsk->thread.address = addr;
	tsk->thread.error_code = fsr;
	tsk->thread.trap_no = 14;
	si.si_signo = sig;
	si.si_errno = 0;
	si.si_code = code;
	si.si_addr = (void __user *)addr;
	force_sig_info(sig, &si, tsk);
}

void do_bad_area(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	struct task_struct *tsk = current;
	struct mm_struct *mm = tsk->active_mm;

	/*
	 * If we are in kernel mode at this point, we
	 * have no context to handle this fault with.
	 */
	if (user_mode(regs))
		__do_user_fault(tsk, addr, fsr, SIGSEGV, SEGV_MAPERR, regs);
	else
		__do_kernel_fault(mm, addr, fsr, regs);
}

#ifdef CONFIG_MMU
#define VM_FAULT_BADMAP		0x010000
#define VM_FAULT_BADACCESS	0x020000

/*
 * Check that the permissions on the VMA allow for the fault which occurred.
 * If we encountered a write fault, we must have write permission, otherwise
 * we allow any permission.
 */
static inline bool access_error(unsigned int fsr, struct vm_area_struct *vma)
{
	unsigned int mask = VM_READ | VM_WRITE | VM_EXEC;

	if (fsr & FSR_WRITE)
		mask = VM_WRITE;
	if (fsr & FSR_LNX_PF)
		mask = VM_EXEC;

	return vma->vm_flags & mask ? false : true;
}

static int __kprobes
__do_page_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr,
		unsigned int flags, struct task_struct *tsk)
{
	struct vm_area_struct *vma;
	int fault;

	vma = find_vma(mm, addr);
	fault = VM_FAULT_BADMAP;
	if (unlikely(!vma))
		goto out;
	if (unlikely(vma->vm_start > addr))
		goto check_stack;

	/*
	 * Ok, we have a good vm_area for this
	 * memory access, so we can handle it.
	 */
good_area:
	if (access_error(fsr, vma)) {
		fault = VM_FAULT_BADACCESS;
		goto out;
	}

	return handle_mm_fault(mm, vma, addr & PAGE_MASK, flags);

check_stack:
	/* Don't allow expansion below FIRST_USER_ADDRESS */
	if (vma->vm_flags & VM_GROWSDOWN &&
	    addr >= FIRST_USER_ADDRESS && !expand_stack(vma, addr))
		goto good_area;
out:
	return fault;
}

static int __kprobes
do_page_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	int fault, sig, code;
	unsigned int flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
	
	int found_flag=-1;
	int cnt_temp=0;
	int found_duplicated=0;
	struct reference_counter * refcnt_print = NULL;
	struct reference_counter * refcnt_new = NULL;
	struct reference_counter * refcnt = NULL;

	if (notify_page_fault(regs, fsr))
		return 0;

	tsk = current;
	mm  = tsk->mm;

	/* Enable interrupts if they were enabled in the parent context. */
	if (interrupts_enabled(regs))
		local_irq_enable();

	/*
	 * If we're in an interrupt or have no user
	 * context, we must not take the fault..
	 */
	if (in_atomic() || !mm)
		goto no_context;

	if (user_mode(regs))
		flags |= FAULT_FLAG_USER;
	if (fsr & FSR_WRITE)
		flags |= FAULT_FLAG_WRITE;

	/*
	 * As per x86, we may deadlock here.  However, since the kernel only
	 * validly references user space from well defined areas of the code,
	 * we can bug out early if this is from code which shouldn't.
	 */
	if (!down_read_trylock(&mm->mmap_sem)) {
		if (!user_mode(regs) && !search_exception_tables(regs->ARM_pc))
			goto no_context;
retry:
		down_read(&mm->mmap_sem);
	} else {
		/*
		 * The above down_read_trylock() might have succeeded in
		 * which case, we'll have missed the might_sleep() from
		 * down_read()
		 */
		might_sleep();
#ifdef CONFIG_DEBUG_VM
		if (!user_mode(regs) &&
		    !search_exception_tables(regs->ARM_pc))
			goto no_context;
#endif
	}

	fault = __do_page_fault(mm, addr, fsr, flags, tsk);

#if 0
	if(strncmp(current->comm,"test",TASK_COMM_LEN)==0){
		if (fsr==0x817){
			//TODO : check NULL for each. might use yun_show_pte
			pgd_t *pgd;
			pud_t *pud;
			pmd_t *pmd;
			pte_t *pte, Npte;
			pgd = pgd_offset(mm,addr);
			pud = pud_offset(pgd,addr);
			pmd = pmd_offset(pud,addr);
			pte = pte_offset_map(pmd,addr);
			Npte = *pte + 2048;
			// add reference_counter for the pte
			//refcnt = tsk->ref_head;
								if (tsk->ref_head==NULL){
									// there is no refcnt. assign it for the first time
									refcnt = (struct reference_counter *)kzalloc(sizeof(struct reference_counter), GFP_KERNEL);
									if (refcnt==NULL){
										printk(KERN_EMERG "[Yun DEBUG] kzalloc() failed in show_map_vma\n");
											BUG();
									}
									refcnt->cnt = 5;
									refcnt->pc = 0;
									refcnt->instruction = 0;
									refcnt->pte = pte;
									refcnt->hpte = pte[PTE_HWTABLE_PTRS];
									refcnt->next=NULL;
									tsk->ref_head = refcnt;
									tsk->ref_tail = refcnt;
									//pte_val(t_pte[PTE_HWTABLE_PTRS]);// = pte_val(t_pte[PTE_HWTABLE_PTRS]) | (0x00000003);
									//pte_val(*t_pte);// = pte_val(*t_pte) | (0x00000003);
									pte_val(pte[PTE_HWTABLE_PTRS]) = pte_val(pte[PTE_HWTABLE_PTRS])  & (0xfffffffc);
									pte_val(*pte) = pte_val(*pte)  & (0xfffffffc);
									printk(KERN_EMERG "[Yun DEBUG] assigned refcnt in do_page_fault2, %d, %lx, %lx\n", cnt_temp, addr, pte_val(*pte));
									//printk(KERN_EMERG "[Yun DEBUG] assigned refcnt in show_map_vma 1,%lx\n", start_address);
								}
								else{
#if 1
									// find if there is refcnt for this pte
									cnt_temp=0;
									found_duplicated=0;
									//printk(KERN_EMERG "[Yun DEBUG] here0\n");
									refcnt = tsk->ref_head;
									//printk(KERN_EMERG "[Yun DEBUG] here1\n");
									while(refcnt != NULL){
										cnt_temp+=1;
										//printk(KERN_EMERG "[Yun DEBUG] here2\n");
										if (pte_val(*(refcnt->pte)) == pte_val(*pte)){
											//TODO :  what to do??
											//printk(KERN_EMERG "[Yun DEBUG] Found duplicated pte in do_page_fault\n");
											found_duplicated=1;
											break;
										}
										refcnt = refcnt->next;
									}
									//printk(KERN_EMERG "[Yun DEBUG] here3\n");
									//found end. add reference cnt
									if (found_duplicated !=1){
										//printk(KERN_EMERG "[Yun DEBUG] here4\n");
										refcnt_new = (struct reference_counter *)kzalloc(sizeof(struct reference_counter), GFP_KERNEL);
										if (refcnt_new==NULL){
											printk(KERN_EMERG "[Yun DEBUG] kzalloc() failed in show_map_vma\n");
												BUG();
										}
										refcnt_new->cnt = 5;
										refcnt_new->pc = 0;
										refcnt_new->instruction = 0;
										refcnt_new->pte = pte;
										//printk(KERN_EMERG "[Yun DEBUG] here5\n");
										refcnt_new->hpte = pte[PTE_HWTABLE_PTRS];
										//printk(KERN_EMERG "[Yun DEBUG] here6\n");
										refcnt_new->next=NULL;
										tsk->ref_tail->next = refcnt_new;
										//printk(KERN_EMERG "[Yun DEBUG] here7\n");
										tsk->ref_tail = refcnt_new;
										//pte_val(t_pte[PTE_HWTABLE_PTRS]);// = pte_val(t_pte[PTE_HWTABLE_PTRS]) | (0x00000003);
										//pte_val(*t_pte);// = pte_val(*t_pte) | (0x00000003);
										pte_val(pte[PTE_HWTABLE_PTRS]) = pte_val(pte[PTE_HWTABLE_PTRS])  & (0xfffffffc);
										pte_val(*pte) = pte_val(*pte)  & (0xfffffffc);
										printk(KERN_EMERG "[Yun DEBUG] assigned refcnt in do_page_fault2, %d, %lx, %lx\n", cnt_temp, addr, pte_val(*pte));
										//printk(KERN_EMERG "[Yun DEBUG] assigned refcnt in show_map_vma 2, %d, %lx\n", cnt_temp, start_address);
									}
#endif
								}

		}
	}
	/* If we need to retry but a fatal signal is pending, handle the
	 * signal first. We do not need to release the mmap_sem because
	 * it would already be released in __lock_page_or_retry in
	 * mm/filemap.c. */
#endif

	if ((fault & VM_FAULT_RETRY) && fatal_signal_pending(current))
		return 0;

	/*
	 * Major/minor page fault accounting is only done on the
	 * initial attempt. If we go through a retry, it is extremely
	 * likely that the page will be found in page cache at that point.
	 */

	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, regs, addr);
	if (!(fault & VM_FAULT_ERROR) && flags & FAULT_FLAG_ALLOW_RETRY) {
		if (fault & VM_FAULT_MAJOR) {
			tsk->maj_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1,
					regs, addr);
		} else {
			tsk->min_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1,
					regs, addr);
		}
		if (fault & VM_FAULT_RETRY) {
			/* Clear FAULT_FLAG_ALLOW_RETRY to avoid any risk
			* of starvation. */
			flags &= ~FAULT_FLAG_ALLOW_RETRY;
			flags |= FAULT_FLAG_TRIED;
			goto retry;
		}
	}

	up_read(&mm->mmap_sem);

	/*
	 * Handle the "normal" case first - VM_FAULT_MAJOR / VM_FAULT_MINOR
	 */
	if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP | VM_FAULT_BADACCESS))))
		return 0;

	/*
	 * If we are in kernel mode at this point, we
	 * have no context to handle this fault with.
	 */
	if (!user_mode(regs))
		goto no_context;

	if (fault & VM_FAULT_OOM) {
		/*
		 * We ran out of memory, call the OOM killer, and return to
		 * userspace (which will retry the fault, or kill us if we
		 * got oom-killed)
		 */
		pagefault_out_of_memory();
		return 0;
	}

	if (fault & VM_FAULT_SIGBUS) {
		/*
		 * We had some memory, but were unable to
		 * successfully fix up this page fault.
		 */
		sig = SIGBUS;
		code = BUS_ADRERR;
	} else {
		/*
		 * Something tried to access memory that
		 * isn't in our memory map..
		 */
		sig = SIGSEGV;
		code = fault == VM_FAULT_BADACCESS ?
			SEGV_ACCERR : SEGV_MAPERR;
	}

	__do_user_fault(tsk, addr, fsr, sig, code, regs);
	return 0;

no_context:
	__do_kernel_fault(mm, addr, fsr, regs);
	return 0;
}
#else					/* CONFIG_MMU */
static int
do_page_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	return 0;
}
#endif					/* CONFIG_MMU */

/*
 * First Level Translation Fault Handler
 *
 * We enter here because the first level page table doesn't contain
 * a valid entry for the address.
 *
 * If the address is in kernel space (>= TASK_SIZE), then we are
 * probably faulting in the vmalloc() area.
 *
 * If the init_task's first level page tables contains the relevant
 * entry, we copy the it to this task.  If not, we send the process
 * a signal, fixup the exception, or oops the kernel.
 *
 * NOTE! We MUST NOT take any locks for this case. We may be in an
 * interrupt or a critical region, and should only copy the information
 * from the master page table, nothing more.
 */
#ifdef CONFIG_MMU
static int __kprobes
do_translation_fault(unsigned long addr, unsigned int fsr,
		     struct pt_regs *regs)
{
	unsigned int index;
	pgd_t *pgd, *pgd_k;
	pud_t *pud, *pud_k;
	pmd_t *pmd, *pmd_k;

	if (addr < TASK_SIZE)
		return do_page_fault(addr, fsr, regs);

	if (user_mode(regs))
		goto bad_area;

	index = pgd_index(addr);

	pgd = cpu_get_pgd() + index;
	pgd_k = init_mm.pgd + index;

	if (pgd_none(*pgd_k))
		goto bad_area;
	if (!pgd_present(*pgd))
		set_pgd(pgd, *pgd_k);

	pud = pud_offset(pgd, addr);
	pud_k = pud_offset(pgd_k, addr);

	if (pud_none(*pud_k))
		goto bad_area;
	if (!pud_present(*pud)) {
		set_pud(pud, *pud_k);
		/*
		 * There is a small window during free_pgtables() where the
		 * user *pud entry is 0 but the TLB has not been invalidated
		 * and we get a level 2 (pmd) translation fault caused by the
		 * intermediate TLB caching of the old level 1 (pud) entry.
		 */
		flush_tlb_kernel_page(addr);
	}

	pmd = pmd_offset(pud, addr);
	pmd_k = pmd_offset(pud_k, addr);

#ifdef CONFIG_ARM_LPAE
	/*
	 * Only one hardware entry per PMD with LPAE.
	 */
	index = 0;
#else
	/*
	 * On ARM one Linux PGD entry contains two hardware entries (see page
	 * tables layout in pgtable.h). We normally guarantee that we always
	 * fill both L1 entries. But create_mapping() doesn't follow the rule.
	 * It can create inidividual L1 entries, so here we have to call
	 * pmd_none() check for the entry really corresponded to address, not
	 * for the first of pair.
	 */
	index = (addr >> SECTION_SHIFT) & 1;
#endif
	if (pmd_none(pmd_k[index]))
		goto bad_area;
	if (!pmd_present(pmd[index]))
		copy_pmd(pmd, pmd_k);

	return 0;

bad_area:
	do_bad_area(addr, fsr, regs);
	return 0;
}
#else					/* CONFIG_MMU */
static int
do_translation_fault(unsigned long addr, unsigned int fsr,
		     struct pt_regs *regs)
{
	return 0;
}
#endif					/* CONFIG_MMU */

/*
 * Some section permission faults need to be handled gracefully.
 * They can happen due to a __{get,put}_user during an oops.
 */
static int
do_sect_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	do_bad_area(addr, fsr, regs);
	return 0;
}

/*
 * This abort handler always returns "fault".
 */
static int
do_bad(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	return 1;
}

struct fsr_info {
	int	(*fn)(unsigned long addr, unsigned int fsr, struct pt_regs *regs);
	int	sig;
	int	code;
	const char *name;
};

/* FSR definition */
#ifdef CONFIG_ARM_LPAE
#include "fsr-3level.c"
#else
#include "fsr-2level.c"
#endif

void __init
hook_fault_code(int nr, int (*fn)(unsigned long, unsigned int, struct pt_regs *),
		int sig, int code, const char *name)
{
	if (nr < 0 || nr >= ARRAY_SIZE(fsr_info))
		BUG();

	fsr_info[nr].fn   = fn;
	fsr_info[nr].sig  = sig;
	fsr_info[nr].code = code;
	fsr_info[nr].name = name;
}

/*
 * Restore an instruction
 */

/*
 * Patch an instruction
 */

// patch undefined instruction to get the kernel control again. so that we can make faulty instruction invalid again
// Idea borrowed from Renju
void patch_instruction(struct pt_regs *regs, struct mm_struct *mm, struct reference_counter* refcnt, unsigned long addr)
{
	unsigned long currentPC  = regs->ARM_pc;
	unsigned int currentInstruction;
	unsigned int undefinedInstruction= BUG_INSTR_VALUE; //e7f001f2
	//unsigned int undefinedInstruction=3778019102;
	int out;
	unsigned int inst_temp=0;
	pte_t * pte=NULL;
	unsigned long hpte;
	// make it writeable.
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : here1\n");
	pte = yun_show_pte(mm, currentPC);
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : here2\n");
	if (pte==NULL){printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : cannot get the pte for PC\n"); BUG();}		 
	//hpte = readl(hw_pte(pte));
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : here3\n");
	hpte = pte_val(pte[PTE_HWTABLE_PTRS]);
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : here4\n");
	set_pte_at(mm, addr, pte, pte_mkwrite(*pte));
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : here5\n");
	writel(hpte & (~PTE_EXT_APX), (void *)(const volatile void *)(((unsigned long)pte) + 2048));
	//writel(hpte & (~PTE_EXT_APX), pte[PTE_HWTABLE_PTRS]);
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : here6\n");
	//printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : ready to patch\n");

	// get current instruction	
	out = get_user(currentInstruction, (u32 __user *)currentPC);
	if (out != 0) {printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : cannot get_user 1\n"); BUG();}
       if (thumb_mode(regs)){
      		printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : thumb_mode\n");
       	if (!is_wide_instruction(currentInstruction)) {
       		printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : not wide\n");
             	currentPC += 2;
       	}
       	else {
       		currentPC += 4;
       	}
        }
        else {
        	currentPC += 4;
        }

	 // save it for checking later
        refcnt->pc = currentPC;
        //save it for restoring later      
        out = get_user(refcnt->instruction, (u32 __user *)currentPC);
        printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : patch instruction start %lx, %08x\n", currentPC, refcnt->instruction);
        if (out != 0) {printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : cannot get_user 2\n"); BUG();}
        out = put_user(undefinedInstruction, (u32 __user *)currentPC);
        if (out != 0) {printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : cannot put_user 1\n"); /*BUG();*/}
        get_user(inst_temp, (u32 __user *)currentPC);
	  flush_cache_all();
	  printk(KERN_EMERG "[Yun:DEBUG] patch_instruction : patch instruction done %lx, %08x\n", currentPC, inst_temp);
	  
        
}

/*
 * Dispatch a data abort to the relevant handler.
 */
asmlinkage void __exception
do_DataAbort(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	const struct fsr_info *inf = fsr_info + fsr_fs(fsr);
	struct siginfo info;
     pte_t *pte;
     unsigned long hpte;
     struct reference_counter *  refcnt=NULL;



#if 1
	if (strncmp(current->comm, "test", TASK_COMM_LEN) == 0) {
		//pte = yun_show_pte(current->active_mm, addr);
		//if (pte){
		//	printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort for test %lx\n", addr);
		//	//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort for test\n");
		//	hpte = readl((const volatile void *)(((unsigned long)pte) + 2048));
		//	if (hpte && (!(hpte & 0x3))) {
		//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort : Found invalid %lx\n", addr);
		//		return;
		//	}
		//}
		if (1){
		//if (fsr==0x817){
		struct task_struct *tsk;
		struct mm_struct *mm;
		tsk = current;
           	mm  = tsk->active_mm;
          	pgd_t *pgd;
           	pud_t *pud;
           	pmd_t *pmd;
           	pte_t *pte;
		pgd = pgd_offset(mm,addr);
		//yun_show_pte(struct mm_struct * mm, unsigned long addr)
		
		if (!pgd_none(*pgd) && !pgd_bad(*pgd)){
			//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort pgd : Found invalid %lx, %lx\n", addr, pgd_val(*pgd));
			pud = pud_offset(pgd,addr);
			if (!pud_none(*pud) && !pud_bad(*pud)){
				//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort pud : Found invalid %lx, %lx\n", addr, pud_present(*pud));
				pmd = pmd_offset(pud, addr);
				if (!pmd_none(*pmd) && !pmd_bad(*pmd)){
					//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort pmd : Found invalid %lx, %lx\n", addr, pmd_val(*pmd));
					pte = pte_offset_map(pmd, addr);
					//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort  : Found invalid 1 %lx, %lx, %lx\n", addr, pte_val(*pte), pte_val(pte[PTE_HWTABLE_PTRS]));
					//if (!pte_present(*pte) && pte_val(*pte) != 0x00000000){
					//if (pte_val(*pte) != 0x00000000){
					//hpte && (!(hpte & 0x3))
					if (pte_present(*pte) && !pte_present(pte[PTE_HWTABLE_PTRS])){
						if(tsk->ref_head==NULL){
							//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort : dont have refcnt header. why- not because of me?\n"); 
							goto normal_routine;
						}
						refcnt = tsk->ref_head;
						while(refcnt!=NULL){
							if (*(refcnt->pte)==*(pte)){
								refcnt->cnt+=1;
								break;
							}
							refcnt=refcnt->next;
						}
						if (refcnt==NULL){
							//printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort : cannot find matching pte. why - not because of me?\n"); 
							goto normal_routine;
						}
						printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort  : Found invalid 2 %08x, %08x, %lx, %lx, %lx\n", regs->ARM_pc, regs->ARM_lr, addr, pte_val(*pte), pte_val(pte[PTE_HWTABLE_PTRS]));
						//printk(KERN_EMERG" CATCHED!!, valid again...now returning!\n");
						pte_val(pte[PTE_HWTABLE_PTRS]) = pte_val(pte[PTE_HWTABLE_PTRS]) | (0x00000003);
						//pte_val(*pte) = pte_val(*pte) | (0x00000003);
						printk(KERN_EMERG "[Yun:DEBUG] do_DataAbort  : make it valid pte=0x%08llx, *ppte=%08llx \n",(long long)pte_val(*pte), (long long)pte_val(pte[PTE_HWTABLE_PTRS]));
						//patch next instruction in order to control for current pte again
						if (refcnt->cnt < 10){patch_instruction(regs, mm, refcnt,addr);}
						return;
					}
				}
			}
		}

	}
	}
#endif

	normal_routine:

	if (!inf->fn(addr, fsr & ~FSR_LNX_PF, regs))
		return;

	printk(KERN_ALERT "Unhandled fault: %s (0x%03x) at 0x%08lx\n",
		inf->name, fsr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm_notify_die("", regs, &info, fsr, 0);
}

void __init
hook_ifault_code(int nr, int (*fn)(unsigned long, unsigned int, struct pt_regs *),
		 int sig, int code, const char *name)
{
	if (nr < 0 || nr >= ARRAY_SIZE(ifsr_info))
		BUG();

	ifsr_info[nr].fn   = fn;
	ifsr_info[nr].sig  = sig;
	ifsr_info[nr].code = code;
	ifsr_info[nr].name = name;
}

asmlinkage void __exception
do_PrefetchAbort(unsigned long addr, unsigned int ifsr, struct pt_regs *regs)
{
	const struct fsr_info *inf = ifsr_info + fsr_fs(ifsr);
	struct siginfo info;

	if (!inf->fn(addr, ifsr | FSR_LNX_PF, regs))
		return;

	printk(KERN_ALERT "Unhandled prefetch abort: %s (0x%03x) at 0x%08lx\n",
		inf->name, ifsr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm_notify_die("", regs, &info, ifsr, 0);
}

#ifndef CONFIG_ARM_LPAE
static int __init exceptions_init(void)
{
	if (cpu_architecture() >= CPU_ARCH_ARMv6) {
		hook_fault_code(4, do_translation_fault, SIGSEGV, SEGV_MAPERR,
				"I-cache maintenance fault");
	}

	if (cpu_architecture() >= CPU_ARCH_ARMv7) {
		/*
		 * TODO: Access flag faults introduced in ARMv6K.
		 * Runtime check for 'K' extension is needed
		 */
		hook_fault_code(3, do_bad, SIGSEGV, SEGV_MAPERR,
				"section access flag fault");
		hook_fault_code(6, do_bad, SIGSEGV, SEGV_MAPERR,
				"section access flag fault");
	}

	return 0;
}

arch_initcall(exceptions_init);
#endif
