/** \file mpi_hal.h
 * mpi_hal.h
 *
 * Header file for the VP-API-II c files requiring MPI interface.
 *
 * Copyright (c) 2006, Legerity Inc.
 * All rights reserved
 *
 * This software is the property of Legerity , Inc. Please refer to the
 * Non Disclosure Agreement (NDA) that you have signed for more information
 * on legal obligations in using, modifying or distributing this file.
 */
#ifndef MPI_UVB_HAL_H
#define MPI_UVB_HAL_H
#include "vp_api_types.h"
/*
 * The API header is needed to define the Device Types used by the API to know
 * how to implement VpMpiReset
 */
#include "vp_api_dev_term.h"
EXTERN void
VpMpiCmd(
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 cmd,
    uint8 cmdLen,
    uint8 *dataPtr);
#endif
