/* insert.c - insert */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  insert  -  Insert a process into a queue in descending key order
 *------------------------------------------------------------------------
 */
status	insert(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key		/* Key for the inserted process	*/
	)
{
	int16	curr;			/* Runs through items in a queue*/
	int16	prev;			/* Holds previous node index	*/
	struct procent *prptr;
	pid32  user_head;

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}
	prptr = &proctab[pid];
	if(prptr->is_user_process == 0){
		curr = firstid(q);
		while (queuetab[curr].qkey >= key) {
		curr = queuetab[curr].qnext;
		}

	/* Insert process between curr node and previous node */

		prev = queuetab[curr].qprev;	/* Get index of previous node	*/
		queuetab[pid].qnext = curr;
		queuetab[pid].qprev = prev;
		queuetab[pid].qkey = key;
		queuetab[prev].qnext = pid;
		queuetab[curr].qprev = pid;
		return OK;
	}
	else {
		curr = firstid(q);
		while(proctab[curr].is_user_process == 0){
			if(curr == 0)
				break;

			curr = queuetab[curr].qnext;
		}

		if(curr == 0){

			prev = queuetab[curr].qprev;	/* Get index of previous node	*/
			queuetab[pid].qnext = curr;
			queuetab[pid].qprev = prev;
			queuetab[pid].qkey = key;
			queuetab[prev].qnext = pid;
			queuetab[curr].qprev = pid;
			return OK;
		}
		else{
			while(proctab[curr].tickets >= proctab[pid].tickets){
				if(curr == 0)
					break;
				curr = queuetab[curr].qnext;
			}
			
			prev = queuetab[curr].qprev;	/* Get index of previous node	*/
			queuetab[pid].qnext = curr;
			queuetab[pid].qprev = prev;
			queuetab[pid].qkey = key;
			queuetab[prev].qnext = pid;
			queuetab[curr].qprev = pid;
			return OK;
		}
	}
}
