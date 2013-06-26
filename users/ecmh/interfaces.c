#include "ecmh.h"

void int_add(struct intnode *intn)
{
	char ll[60];
	listnode_add(g_conf->ints, intn);

	inet_ntop(AF_INET6, &intn->linklocal, ll, sizeof(ll));

	dolog(LOG_DEBUG, "Added %s, link %d, hw %s/%d with an MTU of %d, linklocal: %s\n",
		intn->name, intn->ifindex,
		(intn->hwaddr.sa_family == ARPHRD_ETHER ? "ether" : 
		 (intn->hwaddr.sa_family == ARPHRD_SIT ? "sit" : "???")),
		intn->hwaddr.sa_family,
		intn->mtu,
		ll);
}

struct intnode *int_create(int ifindex)
{
	struct intnode	*intn = malloc(sizeof(struct intnode));
	struct ifreq	ifreq;
	int		i;

	if (!intn) return NULL;
	memset(intn, 0, sizeof(*intn));
	
	intn->ifindex = ifindex;
	intn->removeme = false;

	// Get the interface name (eth0/sit0/...)
	// Will be used for reports etc
	memset(&ifreq, 0, sizeof(ifreq));
	ifreq.ifr_ifindex = ifindex;
	if (ioctl(g_conf->rawsocket, SIOCGIFNAME, &ifreq) != 0)
	{
		dolog(LOG_ERR, "Couldn't determine interfacename of link %d : %s\n", intn->ifindex, strerror(errno));
		int_destroy(intn);
		return NULL;
	}
	memcpy(&intn->name, &ifreq.ifr_name, sizeof(intn->name));

	// Get the MTU size of this interface
	// We will use that for fragmentation
	if (ioctl(g_conf->rawsocket, SIOCGIFMTU, &ifreq) != 0)
	{
		dolog(LOG_ERR, "Couldn't determine MTU size for %s, link %d : %s\n", intn->name, intn->ifindex, strerror(errno));
		perror("Error");
		int_destroy(intn);
		return NULL;
	}
	intn->mtu = ifreq.ifr_mtu;

	// Get hardware address
	if (ioctl(g_conf->rawsocket, SIOCGIFHWADDR, &ifreq) != 0)
	{
		dolog(LOG_ERR, "Couldn't determine hardware address for %s, link %d : %s\n", intn->name, intn->ifindex, strerror(errno));
		perror("Error");
		int_destroy(intn);
		return NULL;
	}
	memcpy(&intn->hwaddr, &ifreq.ifr_hwaddr, sizeof(intn->hwaddr));

	// All okay	
	return intn;
}

void int_destroy(struct intnode *intn)
{
	if (!intn) return;

	free(intn);
	return;
}

struct intnode *int_find(int ifindex)
{
	struct intnode	*intn;
	struct listnode	*ln;

	LIST_LOOP(g_conf->ints, intn, ln)
	{
		if (ifindex == intn->ifindex) return intn;
	}
	return NULL;
}
