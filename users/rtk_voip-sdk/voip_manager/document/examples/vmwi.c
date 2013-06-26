#include <unistd.h>
#include "voip_manager.h"

int main(void)
{
	char vmwi_on = 1, vmwi_off = 0;

	// VMWI test via FSK Type I
	rtk_Gen_FSK_VMWI(0, &vmwi_on, 0);
	sleep(2);
	rtk_Gen_FSK_VMWI(0, &vmwi_off, 0);

	return 0;
}
