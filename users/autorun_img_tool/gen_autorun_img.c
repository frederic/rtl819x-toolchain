#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../linux-2.6.30/drivers/usb/gadget_cathy/usb_ulinker.h"

void gen_setup_ini(void)
{
	FILE *fp;
	int eth_vid=ULINKER_ETHER_VID, eth_pid=ULINKER_ETHER_PID;
	int fsg_vid=ULINKER_STORAGE_VID, fsg_pid=ULINKER_STORAGE_PID;

	fp = fopen("./Setup.ini", "w");
	fprintf(fp,
			"[Setup]\n"
			"\n"
			"GUID=\"%s\"\n"
			"DisplayName=\"%s\"\n"
			"Contact=\"%s\"\n"
			"DisplayVersion=\"%s\"\n"
			"HelpLink=\"%s\"\n"
			"Publisher=\"%s\"\n"
			"\n"
			"TargetDir=\"%s\"\n"
			"DriverPath=\"%s\"\n"
			"Installcmd=\"/i /8188cu /F rtrndis.inf /HWID \"USB\\VID_%04X&PID_%04X\"\"\n"
			"UnInstallcmd=\"/u /8188cu /F rtrndis.inf /HWID \"USB\\VID_%04X&PID_%04X\"\"\n"
			"\n"
			"ServiceName = RtkDevSvc\n"
			"ServiceDescription = \"Realtek Device Service\"\n"
			"AppName = RtkDevSvc.exe\n"
			"AppParam = *\n"
			"StorageVID=\"%04X\"\n"
			"StoragePID=\"%04X\"\n"
			"\n"
			"RNdisVID=\"%04X\"\n"
			"RNdisPID=\"%04X\"\n",
			ULINKER_WINTOOLS_GUID,
			ULINKER_WINTOOLS_DISPLAY_NAME,
			ULINKER_WINTOOLS_CONTACT,
			ULINKER_WINTOOLS_DISPLAY_VERSION,
			ULINKER_WINTOOLS_HELP_LINK,
			ULINKER_WINTOOLS_PUBLISHER,
			ULINKER_WINTOOLS_TARGET_DIR,
			ULINKER_WINTOOLS_DRIVER_PATH,
			eth_vid, eth_pid,
			eth_vid, eth_pid,
			fsg_vid,
			fsg_pid,
			eth_vid,
			eth_pid);
	fclose(fp);
}

int main(int argc, char *argv[])
{
	int ret;
	gen_setup_ini();

	ret = system("rm -fr files > /dev/null 2>&1");
	ret = system("rm -fr mount_img > /dev/null 2>&1");
	ret = system("mkdir files");
	ret = system("mkdir mount_img");
	//ret = system("cp -fr autorun_files/* files/");
	ret = system("rsync -r --exclude=.svn  autorun_files/ files/");
	ret = system("sudo mount -o loop autorun.img mount_img");
	ret = system("sudo rm -fr mount_img/*");
	ret = system("sudo mv Setup.ini files/");

#if defined(CONFIG_RTL_ULINKER_CUSTOMIZATION)
	/* remove realtek ico when customiztion enable */
	if (ULINKER_ETHER_VID != 0x0BDA)
		ret = system("sudo rm -f files/Setup.ico");
#endif

	ret = system("sudo cp -fr files/* mount_img/");
	ret = system("sudo umount mount_img");

	return 0;
}
