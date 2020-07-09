// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../rkguide.h"
#include "VMM.h"
#include "../data/DirectionalSampleData.h"

#include "VMMFactory.h"

namespace rkguide
{

template<int VecSize, int maxComponents>
struct WeightedEMVonMisesFisherFactory: public VonMisesFisherFactory< VecSize, maxComponents>
{
    public:
    typedef std::integral_constant<size_t, (maxComponents + (VecSize -1)) / VecSize> NumVectors;

    struct Configuration
    {
        size_t maK {maxComponents};
        size_t maxEMIterrations {100};

        float maxKappa {10000.0f};
        float maxMeanCosine { KappaToMeanCosine<float>(10000.0f)};
        float convergenceThreshold {0.1f};

        // MAP prior parameters
        // weight prior
        float weightPrior{0.1f};

        // concentration/meanCosine prior
        float meanCosinePriorStrength {0.1f};
        float meanCosinePrior {0.0f};

        void init();

        std::string toString() const;

    };

    struct SufficientStatisitcs
    {
        embree::Vec3< vfloat<VecSize> > sumOfWeightedDirections[NumVectors::value];
        vfloat<VecSize> sumOfWeightedStats[NumVectors::value];

        float sumWeights {0.f};
        float numSamples {0};
        size_t numComponents {maxComponents};

        //SufficientStatisitcs operator+(const SufficientStatisitcs &stats);
        SufficientStatisitcs& operator+=(const SufficientStatisitcs &stats);


        void clear(size_t _numComponents);

        void clearAll();

        void normalize( const float &_numSamples );

        std::string toString() const;

    };

public:

    typedef VonMisesFisherMixture<VecSize, maxComponents> VMM;

    WeightedEMVonMisesFisherFactory();

    void fitMixture(VMM &vmm, size_t numComponents, SufficientStatisitcs &stats, const DirectionalSampleData* samples, const size_t numSamples, const Configuration &cfg) const;

    void updateMixture(VMM &vmm, SufficientStatisitcs &previousStats, const DirectionalSampleData* samples, const size_t numSamples, const Configuration &cfg) const;

private:

    float weightedExpectationStep(VMM &vmm, SufficientStatisitcs &stats, const DirectionalSampleData* samples, const size_t numSamples) const;

    void weightedMaximumAPosteriorStep(VMM &vmm, SufficientStatisitcs &previousStats,
        SufficientStatisitcs &currentStats,
        const Configuration &cfg) const;

    void estimateMAPWeights( VMM &vmm, SufficientStatisitcs &currentStats, SufficientStatisitcs &previousStats, const float &_weightPrior ) const;

    void estimateMAPMeanDirectionAndConcentration( VMM &vmm, SufficientStatisitcs &currentStats, SufficientStatisitcs &previousStats, const Configuration &cfg) const;


};


template<int VecSize, int maxComponents>
WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::WeightedEMVonMisesFisherFactory()
{
    typename VonMisesFisherFactory<VecSize, maxComponents>::VonMisesFisherFactory( );
}


template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::fitMixture(VMM &vmm, size_t numComponents, SufficientStatisitcs &stats, const DirectionalSampleData* samples, const size_t numSamples, const Configuration &cfg) const
{
    //VonMisesFisherFactory< VecSize, maxComponents>::InitUniformVMM( vmm, numComponents, 5.0f);
    this->InitUniformVMM( vmm, numComponents, 5.0f);
    //SufficientStatisitcs stats;
    stats.clear(numComponents);
    //stats.clearAll();
    updateMixture(vmm, stats, samples, numSamples, cfg);

}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::updateMixture(VMM &vmm, SufficientStatisitcs &previousStats, const DirectionalSampleData* samples, const size_t numSamples, const Configuration &cfg) const
{
    SufficientStatisitcs currentStats;
    //stats.clear();
    size_t currentEMIteration = 0;
    bool converged = false;
    float logLikelihood;
    while ( !converged  && currentEMIteration < cfg.maxEMIterrations )
    {
        logLikelihood = weightedExpectationStep( vmm, currentStats, samples, numSamples);
        weightedMaximumAPosteriorStep( vmm, currentStats, previousStats, cfg);
        currentEMIteration++;
        // TODO: Add convergence check
    }
    previousStats += currentStats;
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::Configuration::init()
{
    maxMeanCosine  = KappaToMeanCosine<float>(maxKappa);
}

template<int VecSize, int maxComponents>
std::string WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::Configuration::toString() const
{
    std::stringstream ss;
    ss << "Configuration:" << std::endl;
    ss << "\tmaxComponents:" << maxComponents << std::endl;
    ss << "\tmaxEMIterrations:" << maxEMIterrations << std::endl;
    ss << "\tmaxKappa:" << maxKappa << std::endl;
    ss << "\tmaxMeanCosine:" << maxMeanCosine << std::endl;
    ss << "\tconvergenceThreshold:" << convergenceThreshold << std::endl;
    ss << "\tweightPrior:" << weightPrior << std::endl;
    ss << "\tmeanCosinePriorStrength:" << meanCosinePriorStrength << std::endl;
    ss << "\tmeanCosinePrior:" << meanCosinePrior << std::endl;
    return ss.str();
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs::clear(size_t _numComponents)
{
    const embree::Vec3< vfloat<VecSize> > vecZeros(0.0f);
    const vfloat<VecSize> zeros(0.0f);

    numComponents = _numComponents;
    const int cnt = (numComponents+VecSize-1) / VecSize;

    for(int k = 0; k < cnt;k++)
    {
        sumOfWeightedDirections[k] = vecZeros;
        sumOfWeightedStats[k] = zeros;
    }

    //sumWeights = 0.0f;
    numSamples = 0;
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs::clearAll()
{
     clear(maxComponents);
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs::normalize( const float &_numSamples )
{
    const int cnt = (numComponents+VecSize-1) / VecSize;
    numSamples = _numSamples;
    vfloat<VecSize> sumWeightedStatsVec(0.0f);

    for(int k = 0; k < cnt;k++)
    {
        sumWeightedStatsVec += sumOfWeightedStats[k];
    }
    float sumWeights = reduce_add(sumWeightedStatsVec);
    vfloat<VecSize> norm ( _numSamples / sumWeights );

    for(int k = 0; k < cnt;k++)
    {
        sumOfWeightedDirections[k] *= norm;
        sumOfWeightedStats[k] *= norm;
    }
}

template<int VecSize, int maxComponents>
typename WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs& WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs::operator+=(const WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs &stats)
{
    const int cnt = (numComponents+VecSize-1) / VecSize;

    this->numSamples += stats.numSamples;
    for(int k = 0; k < cnt;k++)
    {
        this->sumOfWeightedDirections[k] += stats.sumOfWeightedDirections[k];
        this->sumOfWeightedStats[k] += stats.sumOfWeightedStats[k];
    }

    return *this;
}

template<int VecSize, int maxComponents>
std::string WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::SufficientStatisitcs::toString() const
{
    std::stringstream ss;
    ss << "SufficientStatisitcs:" << std::endl;
    //ss << "\tsumWeights:" << sumWeights << std::endl;
    ss << "\tnumSamples:" << numSamples << std::endl;
    ss << "\tnumComponents:" << numComponents << std::endl;
    for (size_t k = 0; k < numComponents ; k++)
    {
        int i = k / VecSize;
        int j = k % VecSize;
        ss  << "\tstat["<< k <<"]:" << "\tsumWeightedStats: " << sumOfWeightedStats[i][j]
            << "\tsumWeightedDirections: [" << sumOfWeightedDirections[i].x[j] << ",\t"
            << sumOfWeightedDirections[i].y[j] << ",\t" << sumOfWeightedDirections[i].z[j] << "]"
            << std::endl;
    }
    return ss.str();
}

template<int VecSize, int maxComponents>
float WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::weightedExpectationStep(VMM &vmm,
        SufficientStatisitcs &stats,
        const DirectionalSampleData* samples,
        const size_t numSamples) const
{
    stats.clear(vmm._numComponents);
    //stats.clearAll();
    stats.numComponents = vmm._numComponents;
    stats.numSamples = numSamples;

    const int cnt = (stats.numComponents+VecSize-1) / VecSize;

    float summedLogLikelihood {0.f};

    typename VMM::SoftAssignment softAssign;

    for (size_t n = 0; n < numSamples; n++ )
    {
        const DirectionalSampleData sampleData = samples[n];
        const vfloat<VecSize> sampleWeight = sampleData.weight;
        const embree::Vec3< vfloat<VecSize> > sampleDirection( sampleData.direction[0], sampleData.direction[1], sampleData.direction[2] );

        if ( !vmm.softAssignment( sampleData.direction, softAssign) )
        {
            std::cout << "continue" << std::endl;
            continue;
        }

        summedLogLikelihood += embree::log( softAssign.pdf );

        for (size_t k =0; k < cnt; k++)
        {
            stats.sumOfWeightedDirections[k] += sampleDirection * softAssign.assignments[k] * sampleWeight;
            stats.sumOfWeightedStats[k] += softAssign.assignments[k] * sampleWeight;
        }

    }

    stats.normalize(numSamples);
    return summedLogLikelihood;
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::estimateMAPWeights( VMM &vmm,
        SufficientStatisitcs &currentStats,
        SufficientStatisitcs &previousStats,
        const float &_weightPrior ) const
{
    const int cnt = (vmm._numComponents+VecSize-1) / VecSize;

    const size_t numComponents = vmm._numComponents;

    const vfloat<VecSize> weightPrior(_weightPrior);

    const vfloat<VecSize> numSamples = currentStats.numSamples + previousStats.numSamples;

    for ( size_t k = 0; k < cnt; k ++ )
    {
        //_sumWeights += currentStats.sumOfWeightedStats[k];
        vfloat<VecSize>  weight = ( currentStats.sumOfWeightedStats[k] + previousStats.sumOfWeightedStats[k] ) ;
        weight = ( weightPrior + ( weight ) ) / (( weightPrior * numComponents ) + numSamples );
        //vfloat<VecSize>  weight = ( currentStats.sumOfWeightedStats[k]/* + previousStats.sumOfWeightedStats[k]*/ ) / ( sumWeights );
        //weight = ( weightPrior + ( weight * numSamples ) ) / (( weightPrior * numComponents ) + numSamples );
        vmm._weights[k] = weight;
    }

    // TODO: find better more efficient way
    if ( vmm._numComponents % VecSize > 0 )
    {
            for (size_t i = vmm._numComponents % VecSize; i < VecSize; i++ )
            {
                vmm._weights[cnt-1][i] = 0.0f;
            }
    }
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::estimateMAPMeanDirectionAndConcentration( VMM &vmm,
        SufficientStatisitcs &currentStats,
        SufficientStatisitcs &previousStats ,
        const Configuration &cfg) const
{
    const vfloat<VecSize> currentNumSamples = currentStats.numSamples;
    const vfloat<VecSize> previousNumSamples = previousStats.numSamples;
    const vfloat<VecSize> numSamples = currentNumSamples + previousNumSamples;

    const vfloat<VecSize> currentEstimationWeight = currentNumSamples / numSamples;
    const vfloat<VecSize> previousEstimationWeight = 1.0f - currentEstimationWeight;

    const vfloat<VecSize> meanCosinePrior = cfg.meanCosinePrior;
    const vfloat<VecSize> meanCosinePriorStrength = cfg.meanCosinePriorStrength;
    const vfloat<VecSize> maxMeanCosine = cfg.maxMeanCosine;
    const int cnt = (vmm._numComponents+VecSize-1) / VecSize;
    const int rem = vmm._numComponents % VecSize;

    for (size_t k = 0; k < cnt; k ++)
    {
        const vfloat<VecSize> partialNumSamples = vmm._weights[k] * numSamples;
        embree::Vec3< vfloat<VecSize> > currentMeanDirection;
        currentMeanDirection.x = select(currentStats.sumOfWeightedStats[k] > 0.0f, currentStats.sumOfWeightedDirections[k].x / currentStats.sumOfWeightedStats[k], 0.0f);
        currentMeanDirection.y = select(currentStats.sumOfWeightedStats[k] > 0.0f, currentStats.sumOfWeightedDirections[k].y / currentStats.sumOfWeightedStats[k], 0.0f);
        currentMeanDirection.z = select(currentStats.sumOfWeightedStats[k] > 0.0f, currentStats.sumOfWeightedDirections[k].z / currentStats.sumOfWeightedStats[k], 0.0f);

        // TODO: find a better design to precompute the previousMeanDirection
        embree::Vec3< vfloat<VecSize> > previousMeanDirection;
        previousMeanDirection.x = select(previousStats.sumOfWeightedStats[k] > 0.0f, previousStats.sumOfWeightedDirections[k].x / previousStats.sumOfWeightedStats[k], 0.0f);
        previousMeanDirection.y = select(previousStats.sumOfWeightedStats[k] > 0.0f, previousStats.sumOfWeightedDirections[k].y / previousStats.sumOfWeightedStats[k], 0.0f);
        previousMeanDirection.z = select(previousStats.sumOfWeightedStats[k] > 0.0f, previousStats.sumOfWeightedDirections[k].z / previousStats.sumOfWeightedStats[k], 0.0f);

        embree::Vec3< vfloat<VecSize> > meanDirection =  currentMeanDirection * currentEstimationWeight
            + previousMeanDirection * previousEstimationWeight;

        vfloat<VecSize> meanCosine = length(meanDirection);

        vmm._meanDirections[k].x = select(meanCosine > 0.0f, meanDirection.x / meanCosine, vmm._meanDirections[k].x);
        vmm._meanDirections[k].y = select(meanCosine > 0.0f, meanDirection.y / meanCosine, vmm._meanDirections[k].y);
        vmm._meanDirections[k].z = select(meanCosine > 0.0f, meanDirection.z / meanCosine, vmm._meanDirections[k].z);

        meanCosine = ( meanCosinePrior * meanCosinePriorStrength + meanCosine * partialNumSamples ) / ( meanCosinePriorStrength + partialNumSamples );
        meanCosine = embree::min( maxMeanCosine, meanCosine );
        vmm._meanCosines[k] = meanCosine;
        vmm._kappas[k] = MeanCosineToKappa< vfloat<VecSize> >( meanCosine );
    }

    // TODO: find better more efficient way
    if ( rem > 0 )
    {
        for ( size_t i = rem; i < VecSize; i++)
        {
            vmm._meanDirections[cnt-1].x[i] = 0.0f;
            vmm._meanDirections[cnt-1].y[i] = 0.0f;
            vmm._meanDirections[cnt-1].z[i] = 1.0f;

            vmm._meanCosines[cnt-1][i] = 0.0f;
            vmm._kappas[cnt-1][i] = 0.0;
        }
    }

    vmm._calculateNormalization();
}

template<int VecSize, int maxComponents>
void WeightedEMVonMisesFisherFactory< VecSize, maxComponents>::weightedMaximumAPosteriorStep(VMM &vmm,
        SufficientStatisitcs &currentStats,
        SufficientStatisitcs &previousStats,
        const Configuration &cfg) const
{
    // Estimating components weights
    estimateMAPWeights( vmm, currentStats, previousStats, cfg.weightPrior );

    // Estimating mean and concentration
    estimateMAPMeanDirectionAndConcentration( vmm, currentStats, previousStats, cfg);
}



}