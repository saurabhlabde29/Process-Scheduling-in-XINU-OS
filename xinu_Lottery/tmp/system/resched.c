/* resched.c - resched, resched_cntl */

#include <xinu.h>
#include <stdlib.h>
struct	defer	Defer;
local syscall print_ready_list(void);
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	pid32  last_pid;
	pid32 tempid;
	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	//print_ready_list();
	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
	last_pid = currpid;

	//kprintf("*****\n");
	//kprintf("Current pid : %d \t Currstate : %d \t nextpid : %d\n",last_pid,ptold->prstate,firstid(readylist));
	if(firstid(readylist)!=0 && proctab[firstid(readylist)].is_user_process == 0){
		
		if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
			if (ptold->prprio > firstkey(readylist)) 
				return;
			

			/* Old process will no longer remain current */

			ptold->prstate = PR_READY;
			if(ptold->is_user_process == 0){
				insert(currpid, readylist, ptold->prprio);
				tempid = firstid(readylist);
				//kprintf("*** INSIDE FUNCTION ***\n");
				//while(tempid != queuetail(readylist)){
				//	kprintf("%d\n",tempid);
				//	tempid = queuetab[tempid].qnext;
				//}			
			}	
		}
	

	/* Force context switch to highest priority ready process */

		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	}

	else if(firstid(readylist)!=0 && proctab[firstid(readylist)].is_user_process == 1){
		if(ptold->prstate == PR_CURR && ptold->is_user_process == 0)
			return;
		else if(ptold->prstate == PR_CURR && ptold->is_user_process == 1 && ptold->elapsed_time >= ptold->run_time )
			kill(last_pid);

		tempid = firstid(readylist);
				//kprintf("*** INSIDE USER LOOP FUNCTION ***\n");
				//while(tempid != queuetail(readylist)){
				//	kprintf("%d\n",tempid);
				//	tempid = queuetab[tempid].qnext;
				//}			

		int ticket_counter = 0;
		//kprintf("Total Number of Tickets = %d\n",ticketsTotal);
		int winner = rand() % ticketsTotal;
		//kprintf("*** And the winner is %d\n",winner);
		int winner_location = firstid(readylist);

		while(winner_location != queuetail(readylist)){
			ticket_counter += proctab[winner_location].tickets;
			if(ticket_counter > winner)
				break;
			winner_location = queuetab[winner_location].qnext;
		}
		if(currpid == winner_location && ptold->prstate == PR_CURR)
			return;
		if(ptold->is_user_process == 0 && ptold->prstate == PR_CURR){
			ptold->prstate = PR_READY;
			insert(currpid,readylist,ptold->prprio);
		}
		currpid = winner_location;
		if(ptold->is_user_process == 1 && ptold->prstate == PR_CURR)
			ptold->prstate = PR_READY;

		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;

#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	}

	else{
		if(ptold->prstate == PR_CURR){
			if(ptold->prprio > firstkey(readylist))
				return;

			ptold->prstate = PR_READY;
			if(ptold->is_user_process == 0)
				insert(currpid, readylist, ptold->prprio);
		}

		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt == QUANTUM;

#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif
		
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	}

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}

syscall print_ready_list(void){

	qid16 curr = firstid(readylist);
	if(isbadqid(readylist) || isempty(readylist))return SYSERR;	
	
	
	printf("Printing the contents of the ready list\n");
	printf("PID\tKEY\n");
	printf("-----------------------\n");
	while(curr != EMPTY ){
		printf("%u\t%u\n",curr,queuetab[curr].qkey);
		curr = queuetab[curr].qnext;
	}
	
	return OK;
}
