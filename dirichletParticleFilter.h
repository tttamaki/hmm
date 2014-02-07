//
//  dirichletParticleFilter.h
//  hmm
//
//  Created by tamaki on 2014/02/04.
//  Copyright (c) 2014 tamaki. All rights reserved.
//

#ifndef __hmm__dirichletParticleFilter__
#define __hmm__dirichletParticleFilter__

#include <iostream>
#include <vector>
#include "dirichlet.h"


template <class VALUETYPE>
class partifleFilter {
    
protected:
    typedef std::vector< VALUETYPE > aPARTICLE;
    typedef std::vector< aPARTICLE > PARTICLES;
    PARTICLES particles;
    
public:
    partifleFilter(const size_t nParticles)
    {
        assert(nParticles > 0);
        particles.resize(nParticles);
    };
    ~partifleFilter(){};
    
    void init(const std::vector<VALUETYPE> &init){};
    void update(const std::vector<VALUETYPE> &obserbed){};
    
    aPARTICLE average() const {
        aPARTICLE average(particles[0].size(), VALUETYPE(0));
        
        for (size_t d = 0; d < particles[0].size(); d++) {
            for (size_t n = 0; n < N(); n++) {
                average[d] += particles[n][d];
            }
            average[d] /= N();
        }
        return average;
    };
    
    size_t N() const { return particles.size(); };
    
    const PARTICLES & getParticles() const {
        return particles;
    };
};



template <class VALUETYPE>
class dirichletParticleFilter : public partifleFilter<VALUETYPE> {
   
protected:
    typedef typename partifleFilter<VALUETYPE>::aPARTICLE aPARTICLE;
    typedef typename partifleFilter<VALUETYPE>::PARTICLES PARTICLES;
    typedef std::vector<VALUETYPE> PARAMETER;
    
    dirichlet<VALUETYPE> d;
    discrete<VALUETYPE> disc;

    std::vector<VALUETYPE>
    scalarProd(const std::vector<VALUETYPE> &input,
               const VALUETYPE scalar,
               const VALUETYPE adder = 0) const {
        std::vector<VALUETYPE> output(input.size());
        for (size_t i = 0; i < input.size(); i++)
            output[i] = input[i] * scalar + adder;
        return output;
    };
    
public:
    dirichletParticleFilter(const size_t nParticles)
    : partifleFilter<VALUETYPE>(nParticles),
      theta1(100), theta2(5) {};
    ~dirichletParticleFilter(){};

    VALUETYPE theta1; // parameter 1 between 10 to 1000
    VALUETYPE theta2; // parameter 2 between 3 to 10

    void init(const std::vector<VALUETYPE> &initval){
        assert(this->particles[0].size() == 0); // not yet initialized

        PARAMETER alpha = scalarProd(initval, theta2, 1);
        d.setAlpha(alpha);

        for (size_t i = 0; i < this->N(); i++)
            d.sampling(this->particles[i]);
    };
    
    void update(const std::vector<VALUETYPE> &obserbed){
        assert(obserbed.size() == this->particles[0].size());
        PARAMETER alpha;
        
        // diffusion
        for (size_t i = 0; i < this->N(); i++){
            alpha = scalarProd(this->particles[i], theta1);
            d.setAlpha(alpha);
            d.sampling(this->particles[i]);
        }
        // weights
        std::vector<VALUETYPE> weights(this->N());
        alpha = scalarProd(obserbed, theta2, 1);
        d.setAlpha(alpha);
        for (size_t i = 0; i < this->N(); i++)
            weights[i] = d.pdf(this->particles[i]);

        // resampling
        disc.setProb(weights);
        PARTICLES particlesNext(this->N());
        for (size_t i = 0; i < this->N(); i++)
            particlesNext[i] = this->particles[disc.sampling()];
        this->particles = particlesNext;
    };


};







#endif /* defined(__hmm__dirichletParticleFilter__) */
