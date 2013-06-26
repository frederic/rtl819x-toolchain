#ifndef PORT_H_
#define PORT_H_

#define char_t	char
#define webs_t	request *
#define	T(str)	str
#define websRedirect	send_redirect_perm
#define websWrite	req_format_write
#define websGetVar	req_get_cstream_var
#define websError(...)	fprintf(stderr, "websError\n")
#define websDone(...)	do {} while(0)
#define websHeader(req)	req_write(req, "<html>")
#define websFooter(req)	req_write(req, "</html>")

#endif
