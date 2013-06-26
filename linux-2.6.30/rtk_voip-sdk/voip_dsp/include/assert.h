
#ifdef RTL865X_TEST
#define printk	printf
#endif /* RTL865X_TEST */

#ifndef RTL865X_DEBUG
#define assert(expr) do {} while (0)
#else
#define assert(expr) \
        if(!(expr)) {					\
        printk( "%s:%d: assert(%s)\n",	\
        __FILE__,__LINE__,#expr);		\
        }


#endif


