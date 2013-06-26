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
# $RCSfile: dallas_app_cmd.h,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.4 $
#-------------------------------------------------------------------------------
# 
#-------------------------------------------------------------------------------
*/

#ifndef _DALLAS_APP_CMD_H
#define _DALLAS_APP_CMD_H

#include "pmc_typs.h"

int pmc_pbrc_non_slave(int fd, int mode);
int pmc_pbrc_slave(int fd);
int pmc_pbrc_reset_debug(int fd);
int pmc_pbrc_get_debug(int fd, unsigned long address, unsigned long length);
int pmc_pbrc_set_debug(int fd, unsigned long address, unsigned long length, unsigned long * value);
int pmc_pbrc_set_field_debug(int fd, unsigned long address, unsigned long mask, unsigned long * value);
int pmc_pbrc_branch_debug(int fd, unsigned long address);
int pmc_pbrc_init_debug(int fd, int mode);

int pmc_pbrc_read_uni_mac_debug(int fd, unsigned long address);
int pmc_pbrc_write_uni_mac_debug(int fd, unsigned long address, unsigned long * value);
int pmc_pbrc_read_lag_debug(int fd, unsigned long address);
int pmc_pbrc_write_lag_debug(int fd, unsigned long address, unsigned long * value);
int pmc_pbrc_dallas_irq(int fd);

#endif /* end _DALLAS_APP_CMD_H */

// end file

