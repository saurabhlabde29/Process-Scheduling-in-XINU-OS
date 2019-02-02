/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

process	main(void)
{
	pid32 prA, prB , prC, prD;
	
	prA = create_user_proc(timed_execution, 1024, 10, "timed_execution", 1, 10);
	prB = create_user_proc(timed_execution, 1024, 10, "timed_execution", 1, 10);

	set_tickets(prA, 50);
	set_tickets(prB, 50);

 	resume(prA);
	resume(prB);
	
	sleepms(10);
	kill(prA);
	kill(prB);
	//prC = create_user_proc(timed_execution, 1024, 20, "timed_execution", 1, 20);
	//prD = create_user_proc(timed_execution, 1024, 20, "timed_execution", 1, 20);
	//set_tickets(prC, 50);
	//set_tickets(prD, 50);
	//resume(prC);
	//resume(prD);
	//sleepms(100);

   	return OK;

    
}
