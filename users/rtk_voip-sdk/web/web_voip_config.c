#include <stdio.h>
#include <time.h>
#include "web_voip.h"


#define SECOND_DAY 86400
#define SECOND_HOUR 3600

char time_slot[4][20]={"AM 00:00~05:59","AM 06:00~11:59","PM 00:00~05:59","PM 06:00~11:59"};
char Mode[4][5]={"Off","TFTP","FTP","HTTP"};

#if CONFIG_RTK_VOIP_PACKAGE_865X || CONFIG_RTK_VOIP_PACKAGE_867X
#define ERR_MSG(msg)	{ \
    websWrite(wp, T("<html>\n")); \
    websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
    websWrite(wp, T("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>")); \
    websWrite(wp, T("</html>\n")); \
}
#else
#include "apmib.h"
#include "apform.h"
#endif

void asp_voip_ConfigSave(webs_t wp, char_t *path, char_t *query);
void asp_voip_ConfigLoad(webs_t wp, char_t *path, char_t *query);
void asp_voip_FwupdateSet(webs_t wp, char_t *path, char_t *query);
void CaculateNextTime(voipCfgParam_t *pVoIPCfg);

#ifdef CONFIG_APP_BOA
int asp_voip_ConfigGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_ConfigGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	voipCfgParam_t *pVoIPCfg;

	if (web_flash_get(&pVoIPCfg) != 0)
		return -1;

	if (strcmp(argv[0], "mode_http")==0)
	{
		websWrite(wp, pVoIPCfg->auto_cfg_mode ? "checked" : "");
	}
/*+++++added by Jack for auto provision for tftp and ftp+++++*/
	else if (strcmp(argv[0], "mode_tftp")==0)
	{
		websWrite(wp, (pVoIPCfg->auto_cfg_mode == 2) ? "checked" : "");
	}
	else if (strcmp(argv[0], "mode_ftp")==0)
	{
		websWrite(wp, (pVoIPCfg->auto_cfg_mode == 3) ? "checked" : "");
	}
/*-----end-----*/
	else if (strcmp(argv[0], "mode_disable")==0)
	{
		websWrite(wp, pVoIPCfg->auto_cfg_mode ? "" : "checked");
	}
	else if (strcmp(argv[0], "http_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_http_addr);
	}
	else if (strcmp(argv[0], "http_port")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->auto_cfg_http_port);
	}
/*+++++added by Jack for auto provision for tftp and ftp+++++*/
	else if (strcmp(argv[0], "tftp_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_tftp_addr);
	}
	else if (strcmp(argv[0], "ftp_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_ftp_addr);
	}
	else if (strcmp(argv[0], "ftp_user")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_ftp_user);
	}
	else if (strcmp(argv[0], "ftp_passwd")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_ftp_passwd);
	}
	else if (strcmp(argv[0], "autoconfig_version")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->auto_cfg_ver);
	}
/*-----end-----*/
	else if (strcmp(argv[0], "file_path")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_file_path);
	}
	else if (strcmp(argv[0], "expire")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->auto_cfg_expire);
	}
	else
	{
		return -1;
	}

	return 0;
}

void asp_voip_ConfigSet_done( voipCfgParam_t *pVoIPCfg )
{
	system("killall autocfg.sh");
	system("killall sleep");
#ifdef CONFIG_RTK_VOIP_8186_OVER_8671
	system("/bin/autocfg.sh ATA8972-8m &");
#else
	system("/bin/autocfg.sh&");
#endif
	web_flash_set(pVoIPCfg);
#ifndef REBOOT_CHECK
	web_restart_solar();
#endif
}

void asp_voip_ConfigSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;

	if (websGetVar(wp, T("setting_load"), T(""))[0] != 0)
	{
		asp_voip_ConfigLoad(wp, path, query);
		return;
	}

	if (websGetVar(wp, T("setting_save"), T(""))[0] != 0)
	{
		asp_voip_ConfigSave(wp, path, query);
		return;
	}

	if (websGetVar(wp, T("fw_apply"), T(""))[0] != 0)
	{
		asp_voip_FwupdateSet(wp, path, query);
		return;
	}

	if (web_flash_get(&pVoIPCfg) != 0)
		return;

	if (strcmp(websGetVar(wp, T("setting_reset"), T("")), "Reset") == 0)
	{
		voipCfgParam_t *pDefVoIPCfg;

		if (voip_flash_get_default(&pDefVoIPCfg) == 0)
		{
			memcpy(pVoIPCfg, pDefVoIPCfg, sizeof(*pVoIPCfg));
		}
		else
		{
			flash_voip_default(pVoIPCfg);
		}

		goto asp_voip_ConfigSet_done;
	}
	
	pVoIPCfg->auto_cfg_mode = atoi(websGetVar(wp, T("mode"), T("0")));
	if(pVoIPCfg->auto_cfg_mode != 0)
	{
		if(pVoIPCfg->auto_cfg_mode == 1)
		{
			strcpy(pVoIPCfg->auto_cfg_http_addr, websGetVar(wp, T("http_addr"), T("")));
			pVoIPCfg->auto_cfg_http_port = atoi(websGetVar(wp, T("http_port"), T("80")));
		}
		/*+++++added by Jack for auto provision for tftp and ftp+++++*/
		else if(pVoIPCfg->auto_cfg_mode == 2)
			strcpy(pVoIPCfg->auto_cfg_tftp_addr, websGetVar(wp, T("tftp_addr"), T("")));
		else if(pVoIPCfg->auto_cfg_mode == 3){
			strcpy(pVoIPCfg->auto_cfg_ftp_addr, websGetVar(wp, T("ftp_addr"), T("")));
			strcpy(pVoIPCfg->auto_cfg_ftp_user, websGetVar(wp, T("ftp_user"), T("")));
			strcpy(pVoIPCfg->auto_cfg_ftp_passwd, websGetVar(wp, T("ftp_passwd"), T("")));
		}
		/*-----end-----*/
		strcpy(pVoIPCfg->auto_cfg_file_path, websGetVar(wp, T("file_path"), T("")));
		pVoIPCfg->auto_cfg_expire = atoi(websGetVar(wp, T("expire"), T("0")));
	}
asp_voip_ConfigSet_done:

	asp_voip_ConfigSet_done( pVoIPCfg );

#ifdef REBOOT_CHECK
	OK_MSG("/voip_config.asp");
#else
	websRedirect(wp, T("/voip_config.asp"));
#endif
}

void asp_voip_ConfigSave(webs_t wp, char_t *path, char_t *query)
{
#if defined(CONFIG_APP_BOA)
	// TODO
#else
	voipCfgAll_t cfg_all;
	voipCfgParam_t *pVoIPCfg;
	char *buf;
	int buf_len;

	if (web_flash_get(&pVoIPCfg) != 0)
		return;

	memset(&cfg_all, 0, sizeof(cfg_all));
	memcpy(&cfg_all.current_setting, pVoIPCfg, sizeof(voipCfgParam_t));
	cfg_all.mode |= VOIP_CURRENT_SETTING;
	if (flash_voip_export(&cfg_all, &buf, &buf_len, 1) != 0)
	{
		ERR_MSG("export failed\n");
		return;
	}

	websWrite(wp, "HTTP/1.0 200 OK\n");
	websWrite(wp, "Content-Type: application/octet-stream;\n");
	websWrite(wp, "Content-Disposition: attachment;filename=\"config_voip.dat\" \n");
	websWrite(wp, "Pragma: no-cache\n");
	websWrite(wp, "Cache-Control: no-cache\n");
	websWrite(wp, "\n");
	websWriteBlock(wp, buf, buf_len);
	free(buf);
#endif
}

#if defined(CONFIG_APP_BOA) || CONFIG_RTK_VOIP_PACKAGE_867X
void asp_voip_ConfigLoad(webs_t wp, char_t *path, char_t *query)
{
#if defined(CONFIG_APP_BOA)
	// TODO
	websRedirect(wp, T("/voip_config.asp"));
	return;
#else
#define CTYPE_MULTIPART	"multipart/form-data"
#define BOUNDARY_KWD	"boundary="
	voipCfgParam_t *pVoIPCfg;
	struct stat statbuf;
	char *buf;
	char *p;
	char boundary[40];
	char *part, *end_part;
	int endOfHeader;
	int lenContent;
	voipCfgAll_t	*voipall;


	if (web_flash_get(&pVoIPCfg) != 0)
	{
		return;
	}

	fstat(wp->post_data_fd, &statbuf);
	lseek(wp->post_data_fd, SEEK_SET, 0);
	fprintf(stderr, "config size=%d\n", statbuf.st_size);
	
	// ------------------------------------------
	// parse multipart form for file upload 
	// ------------------------------------------
	if (strncmp(wp->content_type, CTYPE_MULTIPART, strlen(CTYPE_MULTIPART)) != 0)
	{
		char err_msg[256];

		sprintf(err_msg, "import failed: not multipart form (content type = %s)\n", wp->content_type);
		ERR_MSG(err_msg);
		return;
	}

	p = strstr(wp->content_type, BOUNDARY_KWD);
	if (p == NULL)
	{
		ERR_MSG("import failed: boundary not found\n");
		return;
	}

	/* skip over the 'boundary=' part */
	p += strlen(BOUNDARY_KWD);
	snprintf(boundary, sizeof(boundary), "--%s", p);

	buf = (char *) malloc(statbuf.st_size + 1); // +1 for null terminated
	if (buf == NULL)
	{
		ERR_MSG("import failed (OOM)\n");
		return;
	}
	buf[statbuf.st_size + 1] = 0;

	read(wp->post_data_fd, buf, statbuf.st_size);

	part = strstr(buf, boundary);
	if (part == NULL)
	{
		ERR_MSG("import failed: part not found\n");
	free(buf);
		return;
	}

	part = strchr(part, '\n');
	if (part == NULL)
	{
		ERR_MSG("import failed: part is end part\n");
		free(buf);
		return;
}
	part++;

	// TODO: check Content-Disposition ?

	endOfHeader = 0;
	while (!endOfHeader)
	{
		part = strchr(part, '\n');
		if (part == NULL)
		{
			ERR_MSG("import failed: part is end part2\n");
			free(buf);
			return;
		}

		part++;
		if ((*part) == '\n') {
			part += 1;
			endOfHeader = 1;
		}
		else if (((*part) == '\r') && ((*(part+1)) == '\n')) {
			part += 2;
			endOfHeader = 1;
		}
	}

	if (!endOfHeader)
	{
		ERR_MSG("import failed: end header not found\n");
		free(buf);
		return;		
	}

	end_part = (char *) memmem(part, statbuf.st_size - ((int) part - (int) buf), 
		boundary, strlen(boundary));

	if (end_part == NULL)
	{
		ERR_MSG("import failed: end_part not found\n");
		free(buf);
		return;		
	}

	lenContent = (int) end_part - (int) part - 2; // -2 for \r\n

	// bug fixed, voip config can't upgrade.
	voipall = malloc(sizeof(voipCfgAll_t));
	if (!voipall)
	{
		ERR_MSG("out of memory !!\r\n");
		free(buf);
		return;
	}
	memset (voipall,0 ,sizeof(voipCfgAll_t));
	memcpy(&voipall->current_setting, pVoIPCfg , sizeof (voipCfgParam_t));
	voipall->mode = VOIP_CURRENT_SETTING;
	if (flash_voip_import(voipall, part, lenContent) != 0)
	{
		ERR_MSG("import failed\n");
		free(buf);
		return;
	}
	free(buf);

#if 0
	free(wp->post_file_name);
	close(wp->post_data_fd);
	wp->post_file_name = NULL;
	wp->post_data_fd = NULL;
#endif

	web_flash_set(&voipall->current_setting);
	free(voipall);
#ifdef REBOOT_CHECK
	OK_MSG("/voip_config.asp");
#else
	web_restart_solar();
	websRedirect(wp, T("/voip_config.asp"));
#endif
#endif 
}

#else

void asp_voip_ConfigLoad(webs_t wp, char_t *path, char_t *query)
{
	voipCfgAll_t cfg_all;
	voipCfgParam_t *pVoIPCfg;

	memset(&cfg_all, 0, sizeof(cfg_all));

	if (web_flash_get(&pVoIPCfg) == 0)
	{
		memcpy(&cfg_all.current_setting, pVoIPCfg, sizeof(voipCfgParam_t));
		cfg_all.mode |= VOIP_CURRENT_SETTING;
	}
	
	if (voip_flash_get_default(&pVoIPCfg) == 0)
	{
		memcpy(&cfg_all.default_setting, pVoIPCfg, sizeof(voipCfgParam_t));
		cfg_all.mode |= VOIP_DEFAULT_SETTING;
	}
	
	if (flash_voip_import(&cfg_all, wp->postData, wp->lenPostData) != 0)
	{
		ERR_MSG("import failed\n");
		return;
	}

	voip_flash_write(&cfg_all);
#ifdef REBOOT_CHECK
	OK_MSG("/voip_config.asp");
#else
	web_restart_solar();
	websRedirect(wp, T("/voip_config.asp"));
#endif
}

#endif

#ifdef CONFIG_APP_BOA
int asp_voip_FwupdateGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_FwupdateGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{	
	voipCfgParam_t *pVoIPCfg;
	time_t next_time;
	struct tm *tmNow;

	int index;
	
	if (web_flash_get(&pVoIPCfg) != 0)
		return -1;

	if (strcmp(argv[0], "fw_mode_tftp")==0)
	{
		websWrite(wp, (pVoIPCfg->fw_update_mode==1) ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_mode_ftp")==0)
	{
		websWrite(wp, (pVoIPCfg->fw_update_mode==2) ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_mode_http")==0)
	{ 
		websWrite(wp, (pVoIPCfg->fw_update_mode==3) ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_mode_off")==0)
	{
		websWrite(wp, (pVoIPCfg->fw_update_mode==0) ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_tftp_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_tftp_addr);
	}
	else if (strcmp(argv[0], "fw_http_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_http_addr);
	}
	else if (strcmp(argv[0], "fw_http_port")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->fw_update_http_port);
	}
	else if (strcmp(argv[0], "fw_ftp_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_ftp_addr);
	}
	else if (strcmp(argv[0], "fw_ftp_user")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_ftp_user);
	}
	else if (strcmp(argv[0], "fw_ftp_passwd")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_ftp_passwd);
	}
	else if (strcmp(argv[0], "fw_file_path")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_file_path);
	}
	else if (strcmp(argv[0], "fw_power_on")==0)
	{
		websWrite(wp, (pVoIPCfg->fw_update_power_on == 0) ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_scheduling")==0)
	{
		websWrite(wp, (pVoIPCfg->fw_update_power_on == 1 )? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_both")==0)
	{
		websWrite(wp, (pVoIPCfg->fw_update_power_on == 2 ) ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_day")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->fw_update_scheduling_day);
	}
	else if (strcmp(argv[0], "fw_time")==0)
	{
		for (index=0; index < 4 ;index++)
		{
			if (index == pVoIPCfg->fw_update_scheduling_time)
				websWrite(wp, "<option selected>%s</option>", time_slot[index]);
			else
				websWrite(wp, "<option>%s</option>", time_slot[index]);
		}
	}
	else if (strcmp(argv[0], "fw_notify")==0)
	{
		websWrite(wp, pVoIPCfg->fw_update_auto ? "" : "checked");
	}
	else if (strcmp(argv[0], "fw_auto")==0)
	{
		websWrite(wp, pVoIPCfg->fw_update_auto ? "checked" : "");
	}
	else if (strcmp(argv[0], "fw_file_prefix")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_file_prefix);
	}

	else if (strcmp(argv[0], "fw_next_time")==0)
	{
		
        	next_time = pVoIPCfg->fw_update_next_time;
        	if(pVoIPCfg->fw_update_mode == 0){
        		websWrite(wp, Mode[0]);
        	}
               	else if(pVoIPCfg ->fw_update_power_on == 0){
        		tmNow = localtime( &next_time );
			websWrite(wp, "Rebooting" );
		}
		else if(pVoIPCfg ->fw_update_power_on == 1){
        		tmNow = localtime( &next_time );
			websWrite(wp, "%s: %s", Mode[pVoIPCfg->fw_update_mode], asctime(tmNow) );
		}
		else if(pVoIPCfg ->fw_update_power_on == 2){
        		tmNow = localtime( &next_time );
			websWrite(wp, "%s: %s or Rebooting", Mode[pVoIPCfg->fw_update_mode], asctime(tmNow) );
		}
	}
	else if (strcmp(argv[0], "fw_version")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->fw_update_fw_version);
	}
	else
	{
		return -1;
	}

	return 0;

}
void asp_voip_FwupdateSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;
	int i;
	char *ptr;
	//time_t now;

	if (web_flash_get(&pVoIPCfg) != 0)
		return;
	
	pVoIPCfg->fw_update_mode = atoi(websGetVar(wp, T("fw_mode"), T("0")));

	if(pVoIPCfg->fw_update_mode !=0){

		if(pVoIPCfg->fw_update_mode ==1)
			strcpy(pVoIPCfg->fw_update_tftp_addr, websGetVar(wp, T("fw_tftp_addr"), T("")));

		else if (pVoIPCfg->fw_update_mode==3)
		{
			strcpy(pVoIPCfg->fw_update_http_addr, websGetVar(wp, T("fw_http_addr"), T("")));
			pVoIPCfg->fw_update_http_port = atoi(websGetVar(wp, T("fw_http_port"), T("")));
		}
		else if(pVoIPCfg->fw_update_mode==2)
		{
			strcpy(pVoIPCfg->fw_update_ftp_addr, websGetVar(wp, T("fw_ftp_addr"), T("")));
			strcpy(pVoIPCfg->fw_update_ftp_user, websGetVar(wp, T("fw_ftp_user"), T("")));
			strcpy(pVoIPCfg->fw_update_ftp_passwd, websGetVar(wp, T("fw_ftp_passwd"), T("")));
		}

		strcpy(pVoIPCfg->fw_update_file_path, websGetVar(wp, T("fw_file_path"), T("")));
		pVoIPCfg->fw_update_power_on = atoi(websGetVar(wp, T("fw_power_on"), T("0")));
		pVoIPCfg->fw_update_scheduling_day = atoi(websGetVar(wp, T("fw_day"), T("0")));
		pVoIPCfg->fw_update_auto = atoi(websGetVar(wp, T("fw_auto_mode"), T("0")));
		strcpy(pVoIPCfg->fw_update_file_prefix, websGetVar(wp, T("fw_file_prefix"), T("")));

		/* time slot */
		ptr = websGetVar(wp, T("fw_time"), T(""));
		for(i=0; i < 4; i++)
		{
			if (!gstrcmp(ptr, time_slot[i]))
				break;
		}
		pVoIPCfg->fw_update_scheduling_time = i;
	}

	CaculateNextTime(pVoIPCfg);
	
	web_flash_set(pVoIPCfg);
#ifdef REBOOT_CHECK	
	OK_MSG("/voip_config.asp");
#else
	voip_flash_server_update();
	system("echo 1 > /var/run/fwupdate.fifo");
	websRedirect(wp, T("/voip_config.asp"));
#endif

}
// form WEB, To re-caculate next time
void asp_voip_FwSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;

	/* Set VoIP part */
	if (web_flash_get(&pVoIPCfg) != 0)
		return;
	
	CaculateNextTime(pVoIPCfg);
	
	web_flash_set(pVoIPCfg);
	voip_flash_server_update();
	system("echo 1 > /var/run/fwupdate.fifo");
	websRedirect(wp, T("/voip_ivr_req.asp"));
}

void CaculateNextTime(voipCfgParam_t *pVoIPCfg)
{
	
	time_t now;
	unsigned int day_now;
	
	time_t next_update_time;

	int mode;
	int scheduling_time;
	int scheduling_day;
	
	
	mode = pVoIPCfg->fw_update_mode;
	scheduling_time = pVoIPCfg->fw_update_scheduling_time;
	scheduling_day = pVoIPCfg->fw_update_scheduling_day;

	if(mode==0){
		next_update_time = 0;
		goto CaculateNextTimeEnd;
	}
	
	
	time( &now );
	localtime( &now);//to get timezone
	
	now -= timezone; //fix time zone problem
	day_now = now / SECOND_DAY;
	
	next_update_time = (day_now + scheduling_day)* SECOND_DAY; 	
	
	//random: (scheduling_day % 6)
	next_update_time += (scheduling_time*6 + scheduling_day %(24/4))* SECOND_HOUR;
	next_update_time += timezone; //fix time zone problem
	
CaculateNextTimeEnd:

	pVoIPCfg->fw_update_next_time = next_update_time;
	
}

