#include "ecmh.h"

// Subscription Node
struct subscrnode *subscr_create(const struct in6_addr *ipv6)
{
	struct subscrnode *subscrn = malloc(sizeof(*subscrn));

	if (!subscrn) return NULL;

	// Fill her in
	memcpy(&subscrn->ipv6, ipv6, sizeof(*ipv6));
	subscrn->refreshtime = time(NULL);

	// All okay
	return subscrn;
}

void subscr_destroy(struct subscrnode *subscrn)
{
	if (!subscrn) return;
	// Free the node
	free(subscrn);
}

struct subscrnode *subscr_find(const struct list *list, const struct in6_addr *ipv6)
{
	struct subscrnode	*subscrn;
	struct listnode		*ln;

	LIST_LOOP(list, subscrn, ln)
	{
		if (COMPARE_IPV6_ADDRESS((*ipv6), subscrn->ipv6)) return subscrn;
	}
	return NULL;
}

bool subscr_unsub(struct list *list, const struct in6_addr *ipv6)
{
	struct subscrnode	*subscrn;
	struct listnode		*ln;

	LIST_LOOP(list, subscrn, ln)
	{
		if (COMPARE_IPV6_ADDRESS((*ipv6), subscrn->ipv6))
		{
			listnode_delete(list, ln);
			return true;
		}
	}
	return false;
}
