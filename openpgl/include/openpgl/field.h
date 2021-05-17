// Copyright 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#ifdef __cplusplus
#include <cstdint>
#include <cstdlib>
#else
#include <stdint.h>
#include <stdlib.h>
#endif

#include "common.h"
#include "config.h"
#include "sampler.h"
#include "samplestorage.h"
#include "region.h"
#include "surfacesamplingdistribution.h"
#include "volumesamplingdistribution.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
struct Field;
#else
typedef ManagedObject Field;
#endif


typedef Field *PGLField;

OPENPGL_CORE_INTERFACE PGLField pglNewField(PGLFieldArguments args);

OPENPGL_CORE_INTERFACE void pglReleaseField(PGLField field);

OPENPGL_CORE_INTERFACE size_t pglFieldGetIteration(PGLField field);

OPENPGL_CORE_INTERFACE void pglFieldSetSceneBounds(PGLField field, pgl_box3f bounds);

//OPENPGL_CORE_INTERFACE pgl_box3f pglFieldGetSceneBounds(PGLField field);

OPENPGL_CORE_INTERFACE void pglFieldUpdate(PGLField field, PGLSampleStorage sampleStorage, size_t numPerPixelSamples);

//OPENPGL_CORE_INTERFACE size_t pglGetTrainingIteration(PGLField field);

OPENPGL_CORE_INTERFACE size_t pglFieldGetTotalSPP(PGLField field);

//OPENPGL_CORE_INTERFACE PGLRegion pglFieldGetSurfaceRegion(PGLField field, pgl_point3f position, PGLSampler* sampler);

//OPENPGL_CORE_INTERFACE PGLRegion pglFieldGetVolumeRegion(PGLField field, pgl_point3f position, PGLSampler* sampler);

OPENPGL_CORE_INTERFACE PGLSurfaceSamplingDistribution pglFieldNewSurfaceSamplingDistribution(PGLField field);

OPENPGL_CORE_INTERFACE bool pglFieldInitSurfaceSamplingDistriubtion(PGLField field, PGLSurfaceSamplingDistribution surfaceSamplingDistriubtion, pgl_point3f position, const float sample1D, const bool useParallaxComp);

OPENPGL_CORE_INTERFACE PGLVolumeSamplingDistribution pglFieldNewVolumeSamplingDistribution(PGLField field);

OPENPGL_CORE_INTERFACE bool pglFieldInitVolumeSamplingDistriubtion(PGLField field, PGLVolumeSamplingDistribution volumeSamplingDistriubtion, pgl_point3f position, const float sample1D, const bool useParallaxComp);


#ifdef __cplusplus
}  // extern "C"
#endif
