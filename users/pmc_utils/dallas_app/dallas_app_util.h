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
# $RCSfile: dallas_app_util.h,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.3 $
#-------------------------------------------------------------------------------
# 
#-------------------------------------------------------------------------------
*/

#ifndef _DALLAS_APP_UTIL_H
#define _DALLAS_APP_UTIL_H

#include "pmc_typs.h"

void dallas_show_request(struct pbrc_cmd * pbrc);
void dallas_show_reply(struct pbrc_cmd * pbrc);
unsigned long dallas_convert_endian(unsigned long value);
int dallas_get_nvdb_file(dallas_nvdb *p_nvdb, FILE *p_fd);

#endif /* end _DALLAS_APP_UTIL_H */

// end file
