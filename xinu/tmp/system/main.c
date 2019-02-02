/*  main.c  - main */

#include <xinu.h>

void priohog_func();
void victim_func();

process	main(void)
{
	print_ready_list();
	kprintf("Making the case for starvation\n\n");
	resume(create(victim_func,8192,20,"Priority victim function",0));	
	resume(create(priohog_func,8192,100,"Priority hogging function",0));
	
	/* Run the Xinu shell */

	recvclr();
	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));


	/* Wait for shell to exit and recreate it */

	//print_ready_list();
	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
	return OK;
    
}

void priohog_func(){
	int i;
	while(TRUE){
		kprintf("Hog\n");
		for(i=0;i<10000;i++);
	}
}

void victim_func(){
	
	int i;
	while(TRUE){
		kprintf("Victim\n");
		for(i=0;i<10000;i++);
	}
}
