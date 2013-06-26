/*
#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: dallas_util.h,v $
#
# $Date: 2008-07-07 02:43:17 $
#
# $Revision: 1.1 $
#-------------------------------------------------------------------------------
# 
#-------------------------------------------------------------------------------
*/


#ifndef DALLAS_UTIL
#define DALLAS_UTIL

#define KTHREAD_RT_NAME_LEN     31

typedef struct {
	void *(*start_routine) (void *);
	void *arg;
	struct completion done;
	char name[KTHREAD_RT_NAME_LEN+1];
} kthread_rt_data_t;

int kernel_thread_rt(char *name, pid_t *tid, int prio, void *(*start_routine)(void*), void *arg);
int set_hw_addr (char buf[], char *str);

#endif
