#ifndef __ASP_PAGE_H
#define __ASP_PAGE_H
//#include "globals.h"
#ifdef SUPPORT_ASP
#define ERROR -1
#define FAILED -1
#define SUCCESS 0
#define TRUE 1
#define FALSE 0

#define MAX_QUERY_TEMP_VAL_SIZE 4096
#define MAX_BOUNDRY_LEN 64

typedef int (*asp_funcptr)(request * req,  int argc, char **argv);
typedef void (*form_funcptr)(request * req, char* path, char* query);

typedef struct asp_name_s
{
	char *name;
	asp_funcptr function;
} asp_name_t;

typedef struct form_name_s
{
	char *name;
	form_funcptr function;
} form_name_t;

typedef struct temp_mem_s
{
	char *str;
	struct temp_mem_s *next;
} temp_mem_t;

int req_format_write(request *req, char *format, ...);
char *req_get_cstream_var(request *req, char *var, char *defaultGetValue);
int update_content_length(request *req);

#endif
#endif // __ASP_PAGE_H
