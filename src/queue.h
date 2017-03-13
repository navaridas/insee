/**
* @file
* @brief	Definition of the queues used in FSIN routers. Both transit & injection queues are defined here.
*/
#ifndef _queue
#define _queue

#include "phit.h"
#include "misc.h"

/**
* This structure defines a transit queue.
*/
typedef struct queue {
    long head;  ///< Points to the item just before the head
    long tail;  ///< Points to the last item inserted
    phit * pos; ///< size = MAX_QUEUE_LEN
} queue;

/**
* This structure defines an injection queue.
*/
typedef struct inj_queue {
    long head;  ///< Points to the item just before the head
    long tail;  ///< Points to the last item inserted
    phit * pos; ///< size = MAX_INJ_QUEUE_LEN
} inj_queue;

// some declarations in queue.c.
void init_queue (queue *q);
long queue_len (queue *q);
long queue_space (queue *q);
phit * head_queue (queue *q);
void ins_queue (queue *q, phit *i);
void ins_mult_queue (queue *q, phit *i, long copies);
void rem_queue (queue *q, phit *i);
void rem_head_queue (queue *q);

// some declarations in queue_inj.c.
void inj_init_queue (inj_queue *q);
long inj_queue_len (inj_queue *q);
long inj_queue_space (inj_queue *q);
void inj_ins_queue (inj_queue *q, phit *i);
void inj_ins_mult_queue (inj_queue *q, phit *i, long copies);
void inj_rem_queue (inj_queue *q, phit *i);

#endif /* _queue */



