/* print ready list - Implements a function that prints the contents of the ready list */

#include<xinu.h>

syscall print_ready_list(void);

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
