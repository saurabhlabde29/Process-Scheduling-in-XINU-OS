/* resched.c - resched, resched_cntl , reshuffle, priority boost*/

#include <xinu.h>


struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	pid32 last_pid = currpid;
	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}
	ptold = &proctab[currpid];
	
/***************************************************************************************************************************************/

	/* Call priority boost to boost priority of all processes if boost period has elapsed */
	if(time_to_boost==0){
		priority_boost(currpid);  // function defined in create user proc
		
	}
	
	rearrange_queues(currpid); // function defined in create user proc
	
	
/************************************************** IF NON-NULL PROCESS IN READY LIST ****************************************************/
	if(firstid(readylist)!=0){
		/* Point to process table entry for the current (old) process */
		if(ptold->prstate == PR_CURR ) {  /*Check if ptold is in current state. If yes, schedule it */
			if (ptold->prprio > firstkey(readylist)) {
				return;
			}
			/* Old process will no longer remain current */
			if(ptold->is_user_process == 0){
				ptold->prstate = PR_READY;
				insert(currpid, readylist, ptold->prprio);
				
			}
		}		
		/* Force context switch to highest priority ready process */
		currpid = dequeue(readylist);
		if(last_pid == currpid){
			dequeue(readylist);
			proctab[currpid].prstate = PR_CURR;
			preempt = TIME_SLICE;
		}
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = TIME_SLICE;		/* Reset time slice for process	*/
#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif	
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	}
/****************************************************** IF NO SYSTEM PROCESS IS AVAILABLE ***************************************************/

	else if(nonempty(user_list_high)){

		if(ptold->is_user_process == 0 && ptold->prstate == PR_CURR)
			 return ;
		

		if(currpid==firstid(user_list_high)){
			ptold-> prstate = PR_CURR;
			dequeue(user_list_high);
			preempt = TIME_SLICE;
			return;
		}
		currpid = dequeue(user_list_high);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		
		preempt = TIME_SLICE;		//* Reset time slice for process	*/
#ifdef CTXSW
			kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif	
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	}

	else if(nonempty(user_list_med)){
		
		if(ptold->is_user_process == 0 && ptold->prstate == PR_CURR)
			 return; 
		
		if(currpid==firstid(user_list_med)){
			ptold-> prstate = PR_CURR;
			dequeue(user_list_med);
			preempt = 2*TIME_SLICE;
			return;
		}
		currpid = dequeue(user_list_med);
			
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		
		preempt = 2*TIME_SLICE;		//* Reset time slice for process	*/
		
#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif	
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	}

	else if(nonempty(user_list_low)){
		
		if(ptold->is_user_process == 0 && ptold->prstate == PR_CURR)
			 return ;
		
		if(currpid==firstid(user_list_low)){
			ptold-> prstate = PR_CURR;
			dequeue(user_list_low);
			preempt = 4*TIME_SLICE;
			return;
		}
		currpid = dequeue(user_list_low);
			
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		
		preempt = 4*TIME_SLICE;		//* Reset time slice for process	*/
		
#ifdef CTXSW
		kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif	
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	}
	else{
		if(ptold->prstate == PR_CURR) {  /* Process remains eligible */
			if (ptold->prprio > firstkey(readylist)) {
				return;
			}
			/* Old process will no longer remain current */
			if(ptold->is_user_process == 0){
				ptold->prstate = PR_READY;
				insert(currpid, readylist, ptold->prprio);
				//kprintf("Inserting %d in ready list from null case\n",currpid);
			}
		}		
			/* Force context switch to highest priority ready process */
			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = TIME_SLICE;		/* Reset time slice for process	*/
#ifdef CTXSW
			kprintf("ctxsw::%d-%d\n",last_pid,currpid);
#endif	
			ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	}
	/* Old process returns here when resumed */
	
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
