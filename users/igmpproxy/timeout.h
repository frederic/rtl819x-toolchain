
#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_ 1


struct	callout {
    struct timeval	c_time;		/* time at which to call routine */
    void		*c_arg;		/* argument to routine */
    void		(*c_func) __P((void *)); /* routine */
    struct		callout *c_next;
};

void timeout(void (*func) __P((void *)), void *arg, int time, struct callout *handle);
void untimeout(struct callout *handle);

#endif
