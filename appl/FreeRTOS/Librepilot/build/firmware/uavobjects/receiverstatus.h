/**
 ******************************************************************************
 * @addtogroup UAVObjects OpenPilot UAVObjects
 * @{
 * @addtogroup ReceiverStatus ReceiverStatus
 * @brief Receiver status.
 *
 * Autogenerated files and functions for ReceiverStatus Object
 *
 * @{
 *
 * @file       receiverstatus.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2013.
 * @brief      Implementation of the ReceiverStatus object. This file has been
 *             automatically generated by the UAVObjectGenerator.
 *
 * @note       Object definition file: receiverstatus.xml.
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef RECEIVERSTATUS_H
#define RECEIVERSTATUS_H
#include <stdbool.h>

/* Object constants */
#define RECEIVERSTATUS_OBJID 0xFD24EDF2
#define RECEIVERSTATUS_ISSINGLEINST 1
#define RECEIVERSTATUS_ISSETTINGS 0
#define RECEIVERSTATUS_ISPRIORITY 1
#define RECEIVERSTATUS_NUMBYTES sizeof(ReceiverStatusData)

/* Generic interface functions */
int32_t ReceiverStatusInitialize();
UAVObjHandle ReceiverStatusHandle();
void ReceiverStatusSetDefaults(UAVObjHandle obj, uint16_t instId);

/* Field Quality information */




/*
 * Packed Object data (unaligned).
 * Should only be used where 4 byte alignment can be guaranteed
 * (eg a single instance on the heap)
 */
typedef struct {
        uint8_t Quality;

} __attribute__((packed)) ReceiverStatusDataPacked;

/*
 * Packed Object data.
 * Alignment is forced to 4 bytes so as to avoid the potential for CPU usage faults
 * on Cortex M4F during load/store of float UAVO fields
 */
typedef ReceiverStatusDataPacked __attribute__((aligned(4))) ReceiverStatusData;

void ReceiverStatusDataOverrideDefaults(ReceiverStatusData * data);

/* Typesafe Object access functions */
static inline int32_t ReceiverStatusGet(ReceiverStatusData * dataOut) {
    return UAVObjGetData(ReceiverStatusHandle(), dataOut);
}
static inline int32_t ReceiverStatusSet(const ReceiverStatusData * dataIn) {
    return UAVObjSetData(ReceiverStatusHandle(), dataIn);
}
static inline int32_t ReceiverStatusInstGet(uint16_t instId, ReceiverStatusData * dataOut) {
    return UAVObjGetInstanceData(ReceiverStatusHandle(), instId, dataOut);
}
static inline int32_t ReceiverStatusInstSet(uint16_t instId, const ReceiverStatusData * dataIn) {
    return UAVObjSetInstanceData(ReceiverStatusHandle(), instId, dataIn);
}
static inline int32_t ReceiverStatusConnectQueue(xQueueHandle queue) {
    return UAVObjConnectQueue(ReceiverStatusHandle(), queue, EV_MASK_ALL_UPDATES);
}
static inline int32_t ReceiverStatusConnectCallback(UAVObjEventCallback cb) {
    return UAVObjConnectCallback(ReceiverStatusHandle(), cb, EV_MASK_ALL_UPDATES, false);
}
static inline int32_t ReceiverStatusConnectFastCallback(UAVObjEventCallback cb) {
    return UAVObjConnectCallback(ReceiverStatusHandle(), cb, EV_MASK_ALL_UPDATES, true);
}
static inline uint16_t ReceiverStatusCreateInstance() {
    return UAVObjCreateInstance(ReceiverStatusHandle());
}
static inline void ReceiverStatusRequestUpdate() {
    UAVObjRequestUpdate(ReceiverStatusHandle());
}
static inline void ReceiverStatusRequestInstUpdate(uint16_t instId) {
    UAVObjRequestInstanceUpdate(ReceiverStatusHandle(), instId);
}
static inline void ReceiverStatusUpdated() {
    UAVObjUpdated(ReceiverStatusHandle());
}
static inline void ReceiverStatusInstUpdated(uint16_t instId) {
    UAVObjInstanceUpdated(ReceiverStatusHandle(), instId);
}
static inline void ReceiverStatusLogging() {
    UAVObjLogging(ReceiverStatusHandle());
}
static inline void ReceiverStatusInstLogging(uint16_t instId) {
    UAVObjInstanceLogging(ReceiverStatusHandle(), instId);
}
static inline int32_t ReceiverStatusGetMetadata(UAVObjMetadata * dataOut) {
    return UAVObjGetMetadata(ReceiverStatusHandle(), dataOut);
}
static inline int32_t ReceiverStatusSetMetadata(const UAVObjMetadata * dataIn) {
    return UAVObjSetMetadata(ReceiverStatusHandle(), dataIn);
}
static inline int8_t ReceiverStatusReadOnly() {
    return UAVObjReadOnly(ReceiverStatusHandle());
}

/* Set/Get functions */
extern void ReceiverStatusQualitySet(uint8_t *NewQuality);
extern void ReceiverStatusQualityGet(uint8_t *NewQuality);


#endif // RECEIVERSTATUS_H

/**
 * @}
 * @}
 */
