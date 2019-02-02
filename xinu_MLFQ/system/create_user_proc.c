/* create_user_proc.c - create_user_proc, newpid, burst_execution */

#include <xinu.h>
#include <stdarg.h>

local	int newpid();

/*------------------------------------------------------------------------
 *  create_user_proc  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32 create_user_proc(
	  void		*funcaddr,	/* Address of the function	*/
	  uint32	ssize,		/* Stack size in bytes		*/
	  uint32	run_time,	/* Process runtime > 0		*/
	  char		*name,		/* Name (for debugging)		*/
	  uint32	nargs,		/* Number of args that follow	*/
	  ...
	)
{
	uint32		savsp, *pushsp;
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/

	mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (run_time < 1) || ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];

	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended						*/
	prptr->prprio = 7;	/* Initially, all process have priority less than that of the system processes 		*/
	prptr->run_time = run_time;	/* Runtime parameter as passed from the argument */
	
	va_list argm;
	va_start(argm, nargs);
	prptr->number_of_bursts = va_arg(argm ,uint32);	/* Getting the arguments from the burst execution function		*/
	prptr->burst_duration = va_arg(argm, uint32);
	prptr->sleep_duration = va_arg(argm ,uint32);  
	va_end(argm);
	
	prptr->TA = TIME_ALLOTMENT;
	prptr->is_user_process = 1; /* Indicates a user process */
	prptr->burst_duration_copy = prptr->burst_duration;
	prptr->q_num = 1 ;/* All the user processes go to the highest priority queue by default */
	prptr->burst_time = 0;
	prptr->burst_done = 0;
	

	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	/* Initialize stack as if the process was called		*/

	*saddr = STACKMAGIC;
	savsp = (uint32)saddr;

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
		*--saddr = *a--;	/* onto created process's stack	*/
	*--saddr = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	*--saddr = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

	*--saddr = 0;			/* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
	restore(mask);
	return pid;
}

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------

 */

local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 5;	/* Position in table to try or	*/ //FIXME - according to piazza post. Not sure
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */

		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}


/*-----------------------------------------------------------------------
 * burst_execution - To mimic interactive processes
 *-----------------------------------------------------------------------
 */
void burst_execution(uint32 number_bursts,uint32 burst_duration, uint32 sleep_duration)
{
	while(1);
	return;
}

/*------------------------------------------------------------------------------------------------
 *  priority_boost  -  Priority boost all user processes after the given boost period
 *------------------------------------------------------------------------------------------------
 */
void priority_boost(pid32 boost_pid){
	struct procent *prptr;
	prptr = &proctab[boost_pid];
	pid32 iterator;

	if(prptr->prstate==PR_CURR ){	
		if(prptr->is_user_process==0)
		{	
			insert(boost_pid,readylist,prptr->prprio);
			prptr->prstate = PR_READY;
		}
		else if(prptr->is_user_process==1){

			switch(prptr->q_num){
			case 1: enqueue(boost_pid,user_list_high);
				prptr->prstate = PR_READY;
				break;
			case 2: enqueue(boost_pid,user_list_med);
				prptr->prstate = PR_READY;
				break;
			case 3: enqueue(boost_pid,user_list_low);
				prptr->prstate = PR_READY;
				break;
			}
			

		}
	}
			
		
	while(nonempty(user_list_med)){				/* Dequeue processes from mid queue */
		iterator = dequeue(user_list_med);
		enqueue(iterator,user_list_high);	
		proctab[iterator].q_num = 1;
	}
	while(nonempty(user_list_low)){				/* Dequeue processes from lowest queue */
		iterator = dequeue(user_list_low);
		enqueue(iterator,user_list_high);
		proctab[iterator].q_num = 1;
	}

	iterator = firstid(user_list_high);
	if(nonempty(user_list_high)){
		while(iterator!=queuetail(user_list_high)){
			proctab[iterator].TA = TIME_ALLOTMENT;	/* Reset time allotment of all processes in topmost queue */
			iterator = queuetab[iterator].qnext;
		}
	}

	iterator = firstid(sleepq);
	if(nonempty(sleepq)){
		while(iterator!=queuetail(sleepq)){
			if(proctab[iterator].is_user_process==1){
				proctab[iterator].q_num = 1;	/* Change priority of process in sleep queue to highest priority queue */
				proctab[iterator].TA = TIME_ALLOTMENT;
			}
			iterator = queuetab[iterator].qnext;
		}
	}
	time_to_boost = PRIORITY_BOOST_PERIOD;
	return;
}


/*------------------------------------------------------------------------------------------------
 *  reshuffle  -  Reshuffle process according to priority and enqueue them in appropriate queues
 *------------------------------------------------------------------------------------------------
 */
void rearrange_queues(pid32 pid){
	struct procent *prptr;
	prptr = &proctab[pid];
	
	if(prptr->is_user_process==1){
		if(prptr->TA == 0){		/* Test whether the alloted time for a process in the queue has been completed */
			switch(prptr->q_num){
			case 1: if(prptr->prstate==PR_CURR){
					enqueue(pid,user_list_med);
					prptr->prstate = PR_READY;	 /* Current processes are made ready */	
				}
				prptr->q_num = 2;
				prptr->TA = TIME_ALLOTMENT;
				break;
			case 2: if(prptr->prstate==PR_CURR){
					enqueue(pid,user_list_low);
					prptr->prstate = PR_READY;	 /* Current processes are made ready */	
				}
				prptr->q_num == 3;
				prptr->TA = TIME_ALLOTMENT;
				break;
			case 3: if(prptr->prstate==PR_CURR){					
					enqueue(pid,user_list_low);
					prptr->prstate = PR_READY;	 /* Current processes are made ready */	
				}
				prptr->q_num == 3;
				prptr->TA = TIME_ALLOTMENT;
			}

	
		
		}
		else if(prptr->TA > 0 && prptr->prstate==PR_CURR ){ /* Test if the process is ellible to stay in the queue */
			if(prptr->q_num == 1){
				enqueue(pid,user_list_high);
				prptr->prstate = PR_READY;
			}
			else if(prptr->q_num == 2){
				enqueue(pid,user_list_med);
				prptr->prstate = PR_READY;
			}
			else if(prptr->q_num == 3){
				enqueue(pid,user_list_low);
				prptr->prstate = PR_READY;
			}
						
		}
	}
	return;
}

