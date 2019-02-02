#include <xinu.h>
#include <stdio.h>

int main() {
	pid32 prA, prB, prC, prD;

    prA = create_user_proc(timed_execution, 1024, 20, "timed_execution", 3, 1, 20, 0);
    prB = create_user_proc(timed_execution, 1024, 20, "timed_execution", 3, 1, 20, 0);
	set_tickets(prA, 50);
	set_tickets(prB, 50);
    resume(prA);
    resume(prB);
    sleepms(30);
    prC = create_user_proc(timed_execution, 1024, 20, "timed_execution", 3, 1, 20, 0);
    prD = create_user_proc(timed_execution, 1024, 20, "timed_execution", 3, 1, 20, 0);
	set_tickets(prC, 50);
	set_tickets(prD, 50);
    resume(prC);
    resume(prD);
    sleepms(100);

    return OK;
}

