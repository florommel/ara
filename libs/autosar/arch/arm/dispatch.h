/**
 * @file
 * @ingroup i386
 * @brief i386 task dispatching
 */

#ifndef __DISPATCH_H__
#define __DISPATCH_H__

#include "os/util/inline.h"
#include "os/util/encoded.h"
#include "os/scheduler/task.h"
#include "os/hooks.h"
#include "gic.h"
#include "machine.h"
#include "syscall.h"
#include "tcb.h"


namespace arch {

extern volatile void* startup_sp;

// next task to dispatch (used by dispatch interrupt)
#ifdef CONFIG_DEPENDABILITY_ENCODED
extern os::redundant::MergedDMR<uint32_t> save_sp;
extern volatile Encoded_Static<A0, 42> dispatch_task;
#else
extern os::redundant::Plain<uint32_t> save_sp;
extern volatile uint16_t dispatch_task;
#endif

class Dispatcher {

#ifdef CONFIG_ARCH_IDLE_HALT
	/** \brief The idle loop
	 *
	 * Must be run in ring 0 to allow halting the machine
	 */
	static noinline void idle_loop() {
		/* Call the idle loop callback (does end with
		   Machine::enable_interrupts())*/
		CALL_HOOK(PreIdleHook);

		// allow all interrupts
		GIC::set_task_prio(IRQ_PRIO_LOWEST);

		/* enable interrupts and go to sleep */
		while (true) Machine::goto_sleep();

		Machine::unreachable(); // should never come here
	}
#endif

	static forceinline void dispatch_syscall(const os::scheduler::Task& task) {
		// set task to dispatch
#ifdef CONFIG_DEPENDABILITY_ENCODED
		dispatch_task.encode(task.id);
#else
		dispatch_task = task.id;
#endif
        CALL_HOOK(PreTaskHook);

        //kout << "dispatch to " << (int) task.id << endl;

		// request dispatcher AST
		GIC::trigger(IRQ_DISPATCH);

		// unblock ISR2s by lowering GIC task priority
		GIC::set_task_prio(IRQ_PRIO_LOWEST);

        // wait for interrupt
        Machine::halt();
        Machine::unreachable();
	}

public:

	static forceinline void Prepare(const os::scheduler::Task& task) {
        task.tcb.reset();
	}

	static forceinline void Destroy(const os::scheduler::Task& task) {
        task.tcb.reset();
	}

	static forceinline void Dispatch(const os::scheduler::Task& task,
									 const os::scheduler::Task *last = 0) {
		(void) last;
		dispatch_syscall(task);
	}

	static forceinline void ResumeToTask(const os::scheduler::Task& task,
										 const os::scheduler::Task *last = 0 ) {
		(void) last;
		dispatch_syscall(task);
	}

	static forceinline void StartToTask(const os::scheduler::Task& task,
										const os::scheduler::Task *last = 0) {
		(void) last;
		dispatch_syscall(task);
	}

	/** \brief Syscall to start idle loop (in ring 0) */
	static forceinline void Idle(void) {
#ifdef CONFIG_ARCH_IDLE_HALT
		syscall(&idle_loop);
#else
		/* Call the idle loop callback (ends with
		   Machine::enable_interrupts())*/
		CALL_HOOK(PreIdleHook);

		// allow all interrupts
		GIC::set_task_prio(IRQ_PRIO_LOWEST);

		// do nothing forever
		while(true) Machine::nop();

		Machine::unreachable(); // should never come here
#endif // CONFIG_IDLE_HALT

	}
};

} // namespace arch

#endif // __DISPATCH_H__
