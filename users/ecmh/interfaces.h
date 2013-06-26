// The list of interfaces we do multicast on
// These are discovered on the fly, very handy ;)
#include<linux/if.h>
struct intnode
{
	int		ifindex;		// Interface Index
	char		name[IFNAMSIZ];		// Name of the interface
	int		groupcount;		// Number of groups this interface joined
	int		mtu;			// The MTU of this interface
	bool		removeme;		// Is this device to be removed?

	struct sockaddr	hwaddr;			// Hardware bytes
	struct in6_addr	linklocal;		// Link local address
};

/* Node functions */
void int_add(struct intnode *intn);
struct intnode *int_create(int ifindex);
void int_destroy(struct intnode *intn);

/* List functions */
struct intnode *int_find(int ifindex);
