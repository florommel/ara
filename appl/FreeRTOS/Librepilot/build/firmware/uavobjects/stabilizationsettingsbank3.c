/**
 ******************************************************************************
 * @addtogroup UAVObjects OpenPilot UAVObjects
 * @{ 
 * @addtogroup StabilizationSettingsBank3 StabilizationSettingsBank3
 * @brief Currently selected PID bank
 *
 * Autogenerated files and functions for StabilizationSettingsBank3 Object
 * @{ 
 *
 * @file       stabilizationsettingsbank3.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2013.
 * @brief      Implementation of the StabilizationSettingsBank3 object. This file has been 
 *             automatically generated by the UAVObjectGenerator.
 * 
 * @note       Object definition file: stabilizationsettingsbank3.xml. 
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

#include <openpilot.h>
#include "stabilizationsettingsbank3.h"

// Private variables
#if (defined(__MACH__) && defined(__APPLE__))
static UAVObjHandle handle __attribute__((section("__DATA,_uavo_handles")));
#else
static UAVObjHandle handle __attribute__((section("_uavo_handles")));
#endif

#if STABILIZATIONSETTINGSBANK3_ISSETTINGS
SETTINGS_INITCALL(StabilizationSettingsBank3Initialize);
#endif

/**
 * Initialize object.
 * \return 0 Success
 * \return -1 Failure to initialize or -2 for already initialized
 */
int32_t StabilizationSettingsBank3Initialize(void)
{
    // Compile time assertion that the StabilizationSettingsBank3DataPacked and StabilizationSettingsBank3Data structs
    // have the same size (though instances of StabilizationSettingsBank3Data
    // should be placed in memory by the linker/compiler on a 4 byte alignment).
    PIOS_STATIC_ASSERT(sizeof(StabilizationSettingsBank3DataPacked) == sizeof(StabilizationSettingsBank3Data));
    
    // Don't set the handle to null if already registered
    if (UAVObjGetByID(STABILIZATIONSETTINGSBANK3_OBJID)) {
        return -2;
    }

    static const UAVObjType objType = {
       .id = STABILIZATIONSETTINGSBANK3_OBJID,
       .instance_size = STABILIZATIONSETTINGSBANK3_NUMBYTES,
       .init_callback = &StabilizationSettingsBank3SetDefaults,
    };

    // Register object with the object manager
    handle = UAVObjRegister(&objType,
        STABILIZATIONSETTINGSBANK3_ISSINGLEINST, STABILIZATIONSETTINGSBANK3_ISSETTINGS, STABILIZATIONSETTINGSBANK3_ISPRIORITY);

    // Done
    return handle ? 0 : -1;
}

static inline void DataOverrideDefaults(__attribute__((unused)) StabilizationSettingsBank3Data * data) {}

void StabilizationSettingsBank3DataOverrideDefaults(StabilizationSettingsBank3Data * data) __attribute__((weak, alias("DataOverrideDefaults")));

/**
 * Initialize object fields and metadata with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
void StabilizationSettingsBank3SetDefaults(UAVObjHandle obj, uint16_t instId)
{
    StabilizationSettingsBank3Data data;

    // Initialize object fields to their default values
    UAVObjGetInstanceData(obj, instId, &data);
    memset(&data, 0, sizeof(StabilizationSettingsBank3Data));
    data.AttitudeFeedForward.Roll = 0.000000e+00f;
    data.AttitudeFeedForward.Pitch = 0.000000e+00f;
    data.AttitudeFeedForward.Yaw = 0.000000e+00f;
    data.RollRatePID.Kp = 3.000000e-03f;
    data.RollRatePID.Ki = 6.500000e-03f;
    data.RollRatePID.Kd = 3.300000e-05f;
    data.RollRatePID.ILimit = 3.000000e-01f;
    data.PitchRatePID.Kp = 3.000000e-03f;
    data.PitchRatePID.Ki = 6.500000e-03f;
    data.PitchRatePID.Kd = 3.300000e-05f;
    data.PitchRatePID.ILimit = 3.000000e-01f;
    data.YawRatePID.Kp = 6.200000e-03f;
    data.YawRatePID.Ki = 1.000000e-02f;
    data.YawRatePID.Kd = 5.000000e-05f;
    data.YawRatePID.ILimit = 3.000000e-01f;
    data.RollPI.Kp = 2.500000e+00f;
    data.RollPI.Ki = 0.000000e+00f;
    data.RollPI.ILimit = 5.000000e+01f;
    data.PitchPI.Kp = 2.500000e+00f;
    data.PitchPI.Ki = 0.000000e+00f;
    data.PitchPI.ILimit = 5.000000e+01f;
    data.YawPI.Kp = 2.500000e+00f;
    data.YawPI.Ki = 0.000000e+00f;
    data.YawPI.ILimit = 5.000000e+01f;
    data.ManualRate.Roll = 220;
    data.ManualRate.Pitch = 220;
    data.ManualRate.Yaw = 220;
    data.MaximumRate.Roll = 300;
    data.MaximumRate.Pitch = 300;
    data.MaximumRate.Yaw = 300;
    data.RollMax = 55;
    data.PitchMax = 55;
    data.YawMax = 35;
    data.StickExpo.Roll = 0;
    data.StickExpo.Pitch = 0;
    data.StickExpo.Yaw = 0;
    data.AcroInsanityFactor.Roll = 40;
    data.AcroInsanityFactor.Pitch = 40;
    data.AcroInsanityFactor.Yaw = 40;
    data.EnablePiroComp = 1;
    data.FpvCamTiltCompensation = 0;
    data.EnableThrustPIDScaling = 0;
    data.ThrustPIDScaleCurve[0] = 30;
    data.ThrustPIDScaleCurve[1] = 15;
    data.ThrustPIDScaleCurve[2] = 0;
    data.ThrustPIDScaleCurve[3] = -15;
    data.ThrustPIDScaleCurve[4] = -30;
    data.ThrustPIDScaleSource = 2;
    data.ThrustPIDScaleTarget = 0;
    data.ThrustPIDScaleAxes = 1;

    StabilizationSettingsBank3DataOverrideDefaults(&data);
    UAVObjSetInstanceData(obj, instId, &data);

    // Initialize object metadata to their default values
    if ( instId == 0 ) {
        UAVObjMetadata metadata;
        metadata.flags =
            ACCESS_READWRITE << UAVOBJ_ACCESS_SHIFT |
            ACCESS_READWRITE << UAVOBJ_GCS_ACCESS_SHIFT |
            1 << UAVOBJ_TELEMETRY_ACKED_SHIFT |
            1 << UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT |
            UPDATEMODE_ONCHANGE << UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT |
            UPDATEMODE_ONCHANGE << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT |
            UPDATEMODE_MANUAL << UAVOBJ_LOGGING_UPDATE_MODE_SHIFT;
        metadata.telemetryUpdatePeriod = 0;
        metadata.gcsTelemetryUpdatePeriod = 0;
        metadata.loggingUpdatePeriod = 0;
        UAVObjSetMetadata(obj, &metadata);
    }
}

/**
 * Get object handle
 */
UAVObjHandle StabilizationSettingsBank3Handle()
{
    return handle;
}

/**
 * Get/Set object Functions
 */
void StabilizationSettingsBank3AttitudeFeedForwardSet( StabilizationSettingsBank3AttitudeFeedForwardData *NewAttitudeFeedForward )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewAttitudeFeedForward, offsetof(StabilizationSettingsBank3Data, AttitudeFeedForward), 3*sizeof(float));
}
void StabilizationSettingsBank3AttitudeFeedForwardGet( StabilizationSettingsBank3AttitudeFeedForwardData *NewAttitudeFeedForward )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewAttitudeFeedForward, offsetof(StabilizationSettingsBank3Data, AttitudeFeedForward), 3*sizeof(float));
}
void StabilizationSettingsBank3AttitudeFeedForwardArraySet( float *NewAttitudeFeedForward )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewAttitudeFeedForward, offsetof(StabilizationSettingsBank3Data, AttitudeFeedForward), 3*sizeof(float));
}
void StabilizationSettingsBank3AttitudeFeedForwardArrayGet( float *NewAttitudeFeedForward )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewAttitudeFeedForward, offsetof(StabilizationSettingsBank3Data, AttitudeFeedForward), 3*sizeof(float));
}
void StabilizationSettingsBank3RollRatePIDSet( StabilizationSettingsBank3RollRatePIDData *NewRollRatePID )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollRatePID, offsetof(StabilizationSettingsBank3Data, RollRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3RollRatePIDGet( StabilizationSettingsBank3RollRatePIDData *NewRollRatePID )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollRatePID, offsetof(StabilizationSettingsBank3Data, RollRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3RollRatePIDArraySet( float *NewRollRatePID )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollRatePID, offsetof(StabilizationSettingsBank3Data, RollRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3RollRatePIDArrayGet( float *NewRollRatePID )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollRatePID, offsetof(StabilizationSettingsBank3Data, RollRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3PitchRatePIDSet( StabilizationSettingsBank3PitchRatePIDData *NewPitchRatePID )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchRatePID, offsetof(StabilizationSettingsBank3Data, PitchRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3PitchRatePIDGet( StabilizationSettingsBank3PitchRatePIDData *NewPitchRatePID )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchRatePID, offsetof(StabilizationSettingsBank3Data, PitchRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3PitchRatePIDArraySet( float *NewPitchRatePID )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchRatePID, offsetof(StabilizationSettingsBank3Data, PitchRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3PitchRatePIDArrayGet( float *NewPitchRatePID )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchRatePID, offsetof(StabilizationSettingsBank3Data, PitchRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3YawRatePIDSet( StabilizationSettingsBank3YawRatePIDData *NewYawRatePID )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawRatePID, offsetof(StabilizationSettingsBank3Data, YawRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3YawRatePIDGet( StabilizationSettingsBank3YawRatePIDData *NewYawRatePID )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawRatePID, offsetof(StabilizationSettingsBank3Data, YawRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3YawRatePIDArraySet( float *NewYawRatePID )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawRatePID, offsetof(StabilizationSettingsBank3Data, YawRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3YawRatePIDArrayGet( float *NewYawRatePID )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawRatePID, offsetof(StabilizationSettingsBank3Data, YawRatePID), 4*sizeof(float));
}
void StabilizationSettingsBank3RollPISet( StabilizationSettingsBank3RollPIData *NewRollPI )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollPI, offsetof(StabilizationSettingsBank3Data, RollPI), 3*sizeof(float));
}
void StabilizationSettingsBank3RollPIGet( StabilizationSettingsBank3RollPIData *NewRollPI )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollPI, offsetof(StabilizationSettingsBank3Data, RollPI), 3*sizeof(float));
}
void StabilizationSettingsBank3RollPIArraySet( float *NewRollPI )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollPI, offsetof(StabilizationSettingsBank3Data, RollPI), 3*sizeof(float));
}
void StabilizationSettingsBank3RollPIArrayGet( float *NewRollPI )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollPI, offsetof(StabilizationSettingsBank3Data, RollPI), 3*sizeof(float));
}
void StabilizationSettingsBank3PitchPISet( StabilizationSettingsBank3PitchPIData *NewPitchPI )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchPI, offsetof(StabilizationSettingsBank3Data, PitchPI), 3*sizeof(float));
}
void StabilizationSettingsBank3PitchPIGet( StabilizationSettingsBank3PitchPIData *NewPitchPI )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchPI, offsetof(StabilizationSettingsBank3Data, PitchPI), 3*sizeof(float));
}
void StabilizationSettingsBank3PitchPIArraySet( float *NewPitchPI )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchPI, offsetof(StabilizationSettingsBank3Data, PitchPI), 3*sizeof(float));
}
void StabilizationSettingsBank3PitchPIArrayGet( float *NewPitchPI )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchPI, offsetof(StabilizationSettingsBank3Data, PitchPI), 3*sizeof(float));
}
void StabilizationSettingsBank3YawPISet( StabilizationSettingsBank3YawPIData *NewYawPI )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawPI, offsetof(StabilizationSettingsBank3Data, YawPI), 3*sizeof(float));
}
void StabilizationSettingsBank3YawPIGet( StabilizationSettingsBank3YawPIData *NewYawPI )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawPI, offsetof(StabilizationSettingsBank3Data, YawPI), 3*sizeof(float));
}
void StabilizationSettingsBank3YawPIArraySet( float *NewYawPI )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawPI, offsetof(StabilizationSettingsBank3Data, YawPI), 3*sizeof(float));
}
void StabilizationSettingsBank3YawPIArrayGet( float *NewYawPI )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawPI, offsetof(StabilizationSettingsBank3Data, YawPI), 3*sizeof(float));
}
void StabilizationSettingsBank3ManualRateSet( StabilizationSettingsBank3ManualRateData *NewManualRate )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewManualRate, offsetof(StabilizationSettingsBank3Data, ManualRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3ManualRateGet( StabilizationSettingsBank3ManualRateData *NewManualRate )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewManualRate, offsetof(StabilizationSettingsBank3Data, ManualRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3ManualRateArraySet( uint16_t *NewManualRate )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewManualRate, offsetof(StabilizationSettingsBank3Data, ManualRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3ManualRateArrayGet( uint16_t *NewManualRate )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewManualRate, offsetof(StabilizationSettingsBank3Data, ManualRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3MaximumRateSet( StabilizationSettingsBank3MaximumRateData *NewMaximumRate )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewMaximumRate, offsetof(StabilizationSettingsBank3Data, MaximumRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3MaximumRateGet( StabilizationSettingsBank3MaximumRateData *NewMaximumRate )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewMaximumRate, offsetof(StabilizationSettingsBank3Data, MaximumRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3MaximumRateArraySet( uint16_t *NewMaximumRate )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewMaximumRate, offsetof(StabilizationSettingsBank3Data, MaximumRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3MaximumRateArrayGet( uint16_t *NewMaximumRate )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewMaximumRate, offsetof(StabilizationSettingsBank3Data, MaximumRate), 3*sizeof(uint16_t));
}
void StabilizationSettingsBank3RollMaxSet(uint8_t *NewRollMax)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollMax, offsetof(StabilizationSettingsBank3Data, RollMax), sizeof(uint8_t));
}
void StabilizationSettingsBank3RollMaxGet(uint8_t *NewRollMax)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewRollMax, offsetof(StabilizationSettingsBank3Data, RollMax), sizeof(uint8_t));
}
void StabilizationSettingsBank3PitchMaxSet(uint8_t *NewPitchMax)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchMax, offsetof(StabilizationSettingsBank3Data, PitchMax), sizeof(uint8_t));
}
void StabilizationSettingsBank3PitchMaxGet(uint8_t *NewPitchMax)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewPitchMax, offsetof(StabilizationSettingsBank3Data, PitchMax), sizeof(uint8_t));
}
void StabilizationSettingsBank3YawMaxSet(uint8_t *NewYawMax)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawMax, offsetof(StabilizationSettingsBank3Data, YawMax), sizeof(uint8_t));
}
void StabilizationSettingsBank3YawMaxGet(uint8_t *NewYawMax)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewYawMax, offsetof(StabilizationSettingsBank3Data, YawMax), sizeof(uint8_t));
}
void StabilizationSettingsBank3StickExpoSet( StabilizationSettingsBank3StickExpoData *NewStickExpo )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewStickExpo, offsetof(StabilizationSettingsBank3Data, StickExpo), 3*sizeof(int8_t));
}
void StabilizationSettingsBank3StickExpoGet( StabilizationSettingsBank3StickExpoData *NewStickExpo )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewStickExpo, offsetof(StabilizationSettingsBank3Data, StickExpo), 3*sizeof(int8_t));
}
void StabilizationSettingsBank3StickExpoArraySet( int8_t *NewStickExpo )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewStickExpo, offsetof(StabilizationSettingsBank3Data, StickExpo), 3*sizeof(int8_t));
}
void StabilizationSettingsBank3StickExpoArrayGet( int8_t *NewStickExpo )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewStickExpo, offsetof(StabilizationSettingsBank3Data, StickExpo), 3*sizeof(int8_t));
}
void StabilizationSettingsBank3AcroInsanityFactorSet( StabilizationSettingsBank3AcroInsanityFactorData *NewAcroInsanityFactor )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewAcroInsanityFactor, offsetof(StabilizationSettingsBank3Data, AcroInsanityFactor), 3*sizeof(uint8_t));
}
void StabilizationSettingsBank3AcroInsanityFactorGet( StabilizationSettingsBank3AcroInsanityFactorData *NewAcroInsanityFactor )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewAcroInsanityFactor, offsetof(StabilizationSettingsBank3Data, AcroInsanityFactor), 3*sizeof(uint8_t));
}
void StabilizationSettingsBank3AcroInsanityFactorArraySet( uint8_t *NewAcroInsanityFactor )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewAcroInsanityFactor, offsetof(StabilizationSettingsBank3Data, AcroInsanityFactor), 3*sizeof(uint8_t));
}
void StabilizationSettingsBank3AcroInsanityFactorArrayGet( uint8_t *NewAcroInsanityFactor )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewAcroInsanityFactor, offsetof(StabilizationSettingsBank3Data, AcroInsanityFactor), 3*sizeof(uint8_t));
}
void StabilizationSettingsBank3EnablePiroCompSet(StabilizationSettingsBank3EnablePiroCompOptions *NewEnablePiroComp)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewEnablePiroComp, offsetof(StabilizationSettingsBank3Data, EnablePiroComp), sizeof(StabilizationSettingsBank3EnablePiroCompOptions));
}
void StabilizationSettingsBank3EnablePiroCompGet(StabilizationSettingsBank3EnablePiroCompOptions *NewEnablePiroComp)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewEnablePiroComp, offsetof(StabilizationSettingsBank3Data, EnablePiroComp), sizeof(StabilizationSettingsBank3EnablePiroCompOptions));
}
void StabilizationSettingsBank3FpvCamTiltCompensationSet(uint8_t *NewFpvCamTiltCompensation)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewFpvCamTiltCompensation, offsetof(StabilizationSettingsBank3Data, FpvCamTiltCompensation), sizeof(uint8_t));
}
void StabilizationSettingsBank3FpvCamTiltCompensationGet(uint8_t *NewFpvCamTiltCompensation)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewFpvCamTiltCompensation, offsetof(StabilizationSettingsBank3Data, FpvCamTiltCompensation), sizeof(uint8_t));
}
void StabilizationSettingsBank3EnableThrustPIDScalingSet(StabilizationSettingsBank3EnableThrustPIDScalingOptions *NewEnableThrustPIDScaling)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewEnableThrustPIDScaling, offsetof(StabilizationSettingsBank3Data, EnableThrustPIDScaling), sizeof(StabilizationSettingsBank3EnableThrustPIDScalingOptions));
}
void StabilizationSettingsBank3EnableThrustPIDScalingGet(StabilizationSettingsBank3EnableThrustPIDScalingOptions *NewEnableThrustPIDScaling)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewEnableThrustPIDScaling, offsetof(StabilizationSettingsBank3Data, EnableThrustPIDScaling), sizeof(StabilizationSettingsBank3EnableThrustPIDScalingOptions));
}
void StabilizationSettingsBank3ThrustPIDScaleCurveSet( int8_t *NewThrustPIDScaleCurve )
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleCurve, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleCurve), 5*sizeof(int8_t));
}
void StabilizationSettingsBank3ThrustPIDScaleCurveGet( int8_t *NewThrustPIDScaleCurve )
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleCurve, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleCurve), 5*sizeof(int8_t));
}
void StabilizationSettingsBank3ThrustPIDScaleSourceSet(StabilizationSettingsBank3ThrustPIDScaleSourceOptions *NewThrustPIDScaleSource)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleSource, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleSource), sizeof(StabilizationSettingsBank3ThrustPIDScaleSourceOptions));
}
void StabilizationSettingsBank3ThrustPIDScaleSourceGet(StabilizationSettingsBank3ThrustPIDScaleSourceOptions *NewThrustPIDScaleSource)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleSource, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleSource), sizeof(StabilizationSettingsBank3ThrustPIDScaleSourceOptions));
}
void StabilizationSettingsBank3ThrustPIDScaleTargetSet(StabilizationSettingsBank3ThrustPIDScaleTargetOptions *NewThrustPIDScaleTarget)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleTarget, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleTarget), sizeof(StabilizationSettingsBank3ThrustPIDScaleTargetOptions));
}
void StabilizationSettingsBank3ThrustPIDScaleTargetGet(StabilizationSettingsBank3ThrustPIDScaleTargetOptions *NewThrustPIDScaleTarget)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleTarget, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleTarget), sizeof(StabilizationSettingsBank3ThrustPIDScaleTargetOptions));
}
void StabilizationSettingsBank3ThrustPIDScaleAxesSet(StabilizationSettingsBank3ThrustPIDScaleAxesOptions *NewThrustPIDScaleAxes)
{
    UAVObjSetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleAxes, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleAxes), sizeof(StabilizationSettingsBank3ThrustPIDScaleAxesOptions));
}
void StabilizationSettingsBank3ThrustPIDScaleAxesGet(StabilizationSettingsBank3ThrustPIDScaleAxesOptions *NewThrustPIDScaleAxes)
{
    UAVObjGetDataField(StabilizationSettingsBank3Handle(), (void *)NewThrustPIDScaleAxes, offsetof(StabilizationSettingsBank3Data, ThrustPIDScaleAxes), sizeof(StabilizationSettingsBank3ThrustPIDScaleAxesOptions));
}


/**
 * @}
 */
