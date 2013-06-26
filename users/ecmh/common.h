/*****************************************************
 ecmh - Easy Cast du Multi Hub - Common Functions
******************************************************
 $Author: fuzzel $
 $Id: common.h,v 1.1.1.1 2004/01/10 23:59:32 fuzzel Exp $
 $Date: 2004/01/10 23:59:32 $
*****************************************************/

void dolog(int level, char *fmt, ...);
int huprunning();
void savepid();
void cleanpid(int i);
