/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/
	ctr1000++;
	struct	procent	*prptr;	
	prptr=&proctab[currpid];
	

	if(prptr->is_user_process==1){
		if(time_to_boost > 0)	
			time_to_boost-- ;    /* decrease the time to boost if user process */
		
		
		if(prptr->prstate == PR_CURR){
			if(prptr->run_time > 0)   /* Decrease the runtime of the process if its currently running */
				prptr->run_time--;  
			if(prptr->TA > 0)
				prptr->TA--;
			if(prptr->burst_duration > 0) 
				prptr->burst_duration--;
			prptr->burst_time++;
		}
			/* burst execution logic is implemented here */
			if(prptr->burst_duration == 0 && prptr->number_of_bursts>0){
			prptr->number_of_bursts--;
			if(prptr->sleep_duration!=0)
				sleepms(prptr->sleep_duration);
			prptr->burst_duration = prptr->burst_duration_copy;
			}
			if(prptr->number_of_bursts==0)
				kill(currpid);
		
		

	}			


/***********************************************************************************************************************/

		/* burst execution logic is implemented here */
/*		if(prptr->burst_duration == 0 && prptr->number_of_bursts>=0){
			prptr->number_of_bursts--;
			if(prptr->sleep_duration!=0)
				sleepms(prptr->sleep_duration);
			prptr->burst_duration = prptr->burst_duration_copy;
		}
		if(prptr->number_of_bursts==0)
			kill(currpid);
		
	}
*/
/************************************************************************************************************************/

	/* Decrement the ms counter, and see if a second has passed */

	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		//if(time_to_boost==0)
		//priority_boost(currpid);  // function defined in create user proc
		
	
		if(prptr->q_num == 1 || prptr->q_num == 0)	
			preempt = TIME_SLICE;

		else if(prptr->q_num == 2)
			preempt = 2*TIME_SLICE;
		else if(prptr->q_num == 3)
			preempt = 4*TIME_SLICE;
		   
		resched();
	}
}
