#include "ecmh.h"

// Create a groupnode
struct groupnode *group_create(const struct in6_addr *mca)
{
	struct groupnode *groupn = malloc(sizeof(struct groupnode));

	if (!groupn) return NULL;

	// Fill her in
	memcpy(&groupn->mca, mca, sizeof(*mca));

	// Setup the list
	groupn->interfaces = list_new();
	groupn->interfaces->del = (void(*)(void *))grpint_destroy;

	// All okay
	return groupn;
}

void group_destroy(struct groupnode *groupn)
{
	if (!groupn) return;
	// Free the node
	free(groupn);
}

struct groupnode *group_find(const struct in6_addr *mca)
{
	struct groupnode	*groupn;
	struct listnode		*ln;

	LIST_LOOP(g_conf->groups, groupn, ln)
	{
		if (COMPARE_IPV6_ADDRESS((*mca), groupn->mca)) return groupn;
	}
	return NULL;
}

// Find the groupint or create it
// mca		= The IPv6 address of the Multicast group
// interface	= the interface we received it on
struct grpintnode *groupint_get(const struct in6_addr *mca, struct intnode *interface)
{
	struct groupnode	*groupn;
	struct grpintnode	*grpintn;
	struct intnode		*intn;
	struct listnode		*ln;
	bool			forward = false;

	// Find our beloved group
	groupn = group_find(mca);

	if (!groupn)
	{
		// Create the group node
		groupn = group_create(mca);

		// Add the group to the list
		if (groupn) listnode_add(g_conf->groups, (void *)groupn);
		
		forward = true;
	}

	// Forward it if we haven't done so for quite some time
	else if ((time(NULL) - groupn->lastforward) > ECMH_SUBSCRIPTION_TIMEOUT)
	{
		D(dolog(LOG_DEBUG, "Last update was %d seconds ago -> resending\n", (int)(time(NULL) - groupn->lastforward));)
		forward = true;
	}

	if (forward)
	{
		// Broadcast that we want this new group
		LIST_LOOP(g_conf->ints, intn, ln)
		{
			// Skip the interface it came from
			if (interface->ifindex == intn->ifindex) continue;

			// Send the MLDv1 Report
			mld_send_report(intn, mca);

			// If it gets marked for being removed, do so
			if (intn->removeme) listnode_delete(g_conf->ints, ln);
		}
		groupn->lastforward = time(NULL);
	}

	if (!groupn) return false;

	// Find the interface in this group
	grpintn = grpint_find(groupn->interfaces, interface);

	if (!grpintn)
	{
		// Create the groupinterface node
		grpintn = grpint_create(interface);

		// Add the group to the list
		if (grpintn) listnode_add(groupn->interfaces, (void *)grpintn);
	}
	return grpintn;
}
