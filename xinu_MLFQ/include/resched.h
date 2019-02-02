/* resched.h */

/* Constants and variables related to deferred rescheduling */

#define	DEFER_START	1	/* Start deferred rescehduling		*/
#define	DEFER_STOP	2	/* Stop  deferred rescehduling		*/
#define TIME_SLICE 2		/* Time slice of the highest priority queue and ready queue*/
#define TIME_ALLOTMENT 400	/* Time allotment for each queue*/
#define PRIORITY_BOOST_PERIOD 2200 /*Boost period after priority */

/* Structure that collects items related to deferred rescheduling	*/

struct	defer	{
	int32	ndefers;	/* Number of outstanding defers 	*/
	bool8	attempt;	/* Was resched called during the	*/
				/*   deferral period?			*/
};

extern	struct	defer	Defer;
extern uint32 time_to_boost;


