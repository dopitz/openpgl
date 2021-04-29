// Copyright 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../openpgl.h"
#include "Region.h"

namespace openpgl
{
namespace cpp
{

/**
 * @brief 
 * 
 */

struct VolumeSamplingDistribution
{
    VolumeSamplingDistribution();
    ~VolumeSamplingDistribution();

    VolumeSamplingDistribution(const VolumeSamplingDistribution&) = delete;

    /**
     * @brief 
     * 
     * @param sample2D 
     * @return pgl_vec3f 
     */
    pgl_vec3f Sample(const pgl_point2f& sample2D)const;

    /**
     * @brief 
     * 
     * @param direction 
     * @return float 
     */
    float PDF(const pgl_vec3f& direction) const;

    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool IsValid() const;

    /**
     * @brief 
     * 
     */
    void Clear();

    /**
     * @brief 
     * 
     * @param region 
     * @param pos 
     * @param useParallaxCompensation 
     */
    void Init(const Region& region, const pgl_point3f& pos, const bool useParallaxCompensation = true);

    private:
        PGLVolumeSamplingDistribution m_volumeSamplingDistributionHandle{nullptr};
};

VolumeSamplingDistribution::VolumeSamplingDistribution()
{
    m_volumeSamplingDistributionHandle = pglNewVolumeSamplingDistribution();
}

VolumeSamplingDistribution::~VolumeSamplingDistribution()
{
    OPENPGL_ASSERT(m_volumeSamplingDistributionHandle);
    if(m_volumeSamplingDistributionHandle)
        pglReleaseVolumeSamplingDistribution(m_volumeSamplingDistributionHandle);
    m_volumeSamplingDistributionHandle = nullptr;
}

pgl_vec3f VolumeSamplingDistribution::Sample(const pgl_point2f& sample2D)const
{
    OPENPGL_ASSERT(m_volumeSamplingDistributionHandle);
    return pglVolumeSamplingDistributionSample(m_volumeSamplingDistributionHandle, sample2D);
}

float VolumeSamplingDistribution::PDF(const pgl_vec3f& direction) const
{
    OPENPGL_ASSERT(m_volumeSamplingDistributionHandle);
    return pglVolumeSamplingDistributionPDF(m_volumeSamplingDistributionHandle, direction);
}

bool VolumeSamplingDistribution::IsValid() const
{
    OPENPGL_ASSERT(m_volumeSamplingDistributionHandle);
    return pglVolumeSamplingDistributionIsValid(m_volumeSamplingDistributionHandle);
}

void VolumeSamplingDistribution::Clear()
{
    OPENPGL_ASSERT(m_volumeSamplingDistributionHandle);
    return pglVolumeSamplingDistributionClear(m_volumeSamplingDistributionHandle);
}

void VolumeSamplingDistribution::Init(const Region& region, const pgl_point3f& pos, const bool useParallaxCompensation)
{
    OPENPGL_ASSERT(m_volumeSamplingDistributionHandle);
    pglVolumeSamplingDistributionInit(m_volumeSamplingDistributionHandle, region.m_regionHandle, pos, useParallaxCompensation);
}

}
}