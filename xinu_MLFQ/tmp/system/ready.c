/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16   user_list_high;
qid16   user_list_med;
qid16   user_list_low;
/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	


	if(prptr->q_num==0){
		insert(pid, readylist, prptr->prprio);
		
	}
	
	else if(prptr->q_num == 1){
		enqueue(pid,user_list_high);
			
		}
	else if(prptr->q_num == 2){
		enqueue(pid,user_list_med);
			
		}
	else if(prptr->q_num == 3){
		enqueue(pid,user_list_low);
			
		}

	
	return OK;
}
