#include "ecmh.h"

struct grpintnode *grpint_create(const struct intnode *interface)
{
	struct grpintnode *grpintn = malloc(sizeof(*grpintn));

	if (!grpintn) return NULL;

	// Fill her in
	grpintn->interface = (struct intnode *)interface;

	// Setup the list
	grpintn->subscriptions = list_new();
	grpintn->subscriptions->del = (void(*)(void *))grpint_destroy;

	// All okay
	return grpintn;
}

void grpint_destroy(struct grpintnode *grpintn)
{
	struct subscrnode	*subscrn;
	struct listnode		*ln;

	if (!grpintn) return;

	// Empty the subscriber list
	LIST_LOOP(grpintn->subscriptions, subscrn, ln)
	{
		listnode_delete(grpintn->subscriptions, subscrn);
	}

	// Free the node
	free(grpintn);
}

struct grpintnode *grpint_find(const struct list *list, const struct intnode *interface)
{
	struct grpintnode	*grpintn;
	struct listnode		*ln;

	LIST_LOOP(list, grpintn, ln)
	{
		if (grpintn->interface == interface) return grpintn;
	}
	return NULL;
}

// grpintn	= The GroupInterface node
// ipv6		= Source IPv6 address
//		  !ff3x::/96 : unspecified IPv6 address
//		  ff3x::/96  : The source IPv6 address that wants to receive this S<->G channel
bool grpint_refresh(struct grpintnode *grpintn, const struct in6_addr *ipv6)
{
	struct subscrnode *subscrn;

	// Find our beloved group
	subscrn = subscr_find(grpintn->subscriptions, ipv6);

	if (!subscrn)
	{
		// Create the subscr node
		subscrn = subscr_create(ipv6);

		// Add the group to the list
		if (subscrn)
		{
			listnode_add(grpintn->subscriptions, (void *)subscrn);
		}
	}

	if (!subscrn) return false;

	// Refresh it
	subscrn->refreshtime = time(NULL);

	// All Okay
	return true;
}
