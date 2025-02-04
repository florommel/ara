/**
 ******************************************************************************
 * @addtogroup UAVObjects OpenPilot UAVObjects
 * @{
 * @addtogroup AirspeedState AirspeedState
 * @brief UAVO for true airspeed and calibrated airspeed state estimation.
 *
 * Autogenerated files and functions for AirspeedState Object
 *
 * @{
 *
 * @file       airspeedstate.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2013.
 * @brief      Implementation of the AirspeedState object. This file has been
 *             automatically generated by the UAVObjectGenerator.
 *
 * @note       Object definition file: airspeedstate.xml.
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

#ifndef AIRSPEEDSTATE_H
#define AIRSPEEDSTATE_H
#include <stdbool.h>

/* Object constants */
#define AIRSPEEDSTATE_OBJID 0xC7009F28
#define AIRSPEEDSTATE_ISSINGLEINST 1
#define AIRSPEEDSTATE_ISSETTINGS 0
#define AIRSPEEDSTATE_ISPRIORITY 0
#define AIRSPEEDSTATE_NUMBYTES sizeof(AirspeedStateData)

/* Generic interface functions */
int32_t AirspeedStateInitialize();
UAVObjHandle AirspeedStateHandle();
void AirspeedStateSetDefaults(UAVObjHandle obj, uint16_t instId);

/* Field CalibratedAirspeed information */

/* Field TrueAirspeed information */




/*
 * Packed Object data (unaligned).
 * Should only be used where 4 byte alignment can be guaranteed
 * (eg a single instance on the heap)
 */
typedef struct {
        float CalibratedAirspeed;
    float TrueAirspeed;

} __attribute__((packed)) AirspeedStateDataPacked;

/*
 * Packed Object data.
 * Alignment is forced to 4 bytes so as to avoid the potential for CPU usage faults
 * on Cortex M4F during load/store of float UAVO fields
 */
typedef AirspeedStateDataPacked __attribute__((aligned(4))) AirspeedStateData;

void AirspeedStateDataOverrideDefaults(AirspeedStateData * data);

/* Typesafe Object access functions */
static inline int32_t AirspeedStateGet(AirspeedStateData * dataOut) {
    return UAVObjGetData(AirspeedStateHandle(), dataOut);
}
static inline int32_t AirspeedStateSet(const AirspeedStateData * dataIn) {
    return UAVObjSetData(AirspeedStateHandle(), dataIn);
}
static inline int32_t AirspeedStateInstGet(uint16_t instId, AirspeedStateData * dataOut) {
    return UAVObjGetInstanceData(AirspeedStateHandle(), instId, dataOut);
}
static inline int32_t AirspeedStateInstSet(uint16_t instId, const AirspeedStateData * dataIn) {
    return UAVObjSetInstanceData(AirspeedStateHandle(), instId, dataIn);
}
static inline int32_t AirspeedStateConnectQueue(xQueueHandle queue) {
    return UAVObjConnectQueue(AirspeedStateHandle(), queue, EV_MASK_ALL_UPDATES);
}
static inline int32_t AirspeedStateConnectCallback(UAVObjEventCallback cb) {
    return UAVObjConnectCallback(AirspeedStateHandle(), cb, EV_MASK_ALL_UPDATES, false);
}
static inline int32_t AirspeedStateConnectFastCallback(UAVObjEventCallback cb) {
    return UAVObjConnectCallback(AirspeedStateHandle(), cb, EV_MASK_ALL_UPDATES, true);
}
static inline uint16_t AirspeedStateCreateInstance() {
    return UAVObjCreateInstance(AirspeedStateHandle());
}
static inline void AirspeedStateRequestUpdate() {
    UAVObjRequestUpdate(AirspeedStateHandle());
}
static inline void AirspeedStateRequestInstUpdate(uint16_t instId) {
    UAVObjRequestInstanceUpdate(AirspeedStateHandle(), instId);
}
static inline void AirspeedStateUpdated() {
    UAVObjUpdated(AirspeedStateHandle());
}
static inline void AirspeedStateInstUpdated(uint16_t instId) {
    UAVObjInstanceUpdated(AirspeedStateHandle(), instId);
}
static inline void AirspeedStateLogging() {
    UAVObjLogging(AirspeedStateHandle());
}
static inline void AirspeedStateInstLogging(uint16_t instId) {
    UAVObjInstanceLogging(AirspeedStateHandle(), instId);
}
static inline int32_t AirspeedStateGetMetadata(UAVObjMetadata * dataOut) {
    return UAVObjGetMetadata(AirspeedStateHandle(), dataOut);
}
static inline int32_t AirspeedStateSetMetadata(const UAVObjMetadata * dataIn) {
    return UAVObjSetMetadata(AirspeedStateHandle(), dataIn);
}
static inline int8_t AirspeedStateReadOnly() {
    return UAVObjReadOnly(AirspeedStateHandle());
}

/* Set/Get functions */
extern void AirspeedStateCalibratedAirspeedSet(float *NewCalibratedAirspeed);
extern void AirspeedStateCalibratedAirspeedGet(float *NewCalibratedAirspeed);
extern void AirspeedStateTrueAirspeedSet(float *NewTrueAirspeed);
extern void AirspeedStateTrueAirspeedGet(float *NewTrueAirspeed);


#endif // AIRSPEEDSTATE_H

/**
 * @}
 * @}
 */
