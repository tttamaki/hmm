//
//  dirichletParticleFilter.h
//  hmm
//
//  Created by tamaki on 2014/02/04.
//  Copyright (c) 2014年 tamaki. All rights reserved.
//

#ifndef __hmm__dirichletParticleFilter__
#define __hmm__dirichletParticleFilter__

#include <iostream>
#include "dirichlet.h"


template <class VALUETYPE>
class partifleFilter {
protected:
    std::vector< std::vector<VALUETYPE> > particles;
    
public:
    partifleFilter(size_t nParticles)
    {
        particles.resize(nParticles);
    };
    ~partifleFilter(){};
    
    init(const std::vector<VALUETYPE> &init){};
    update(const std::vector<VALUETYPE> &obserbed){};
    VALUETYPE MAP(){};
    size_t N() const { particles.size(); };
};



template <class VALUETYPE>
class dirichletParticleFilter : public partifleFilter {
protected:
    dirichlet d;
    void scalarProd(const std::vector<VALUETYPE> &input,
                    VALUETYPE scalar,
                    VALUETYPE adder = 0){
        std::vector<VALUETYPE> output(input.size());
        for (size_t i = 0; i < input.size(); i++)
            output[i] = input[i] * scalar + adder;
        return output;
    };
    
public:
    dirichetPartifleFilter(size_t nParticles)
    : theta1(100), theta2(5)
    {};
    ~dirichletParticleFilter(){};

    VALUETYPE theta1; // parameter 1 between 10 to 1000
    VALUETYPE theta2; // parameter 2 between 3 to 10

    void init(const std::vector<VALUETYPE> &initval){
        std::vector<VALUETYPE> alpha = scalarProd(initval, theta2, 1);
        d.setAlpha(alpha);
        for (size_t i = 0; i < N(); i++)
            d.sampling(particles[i]);
    };
    update(const std::vector<VALUETYPE> &obserbed){
        // diffusion
        for (size_t i = 0; i < N(); i++){
            std::vector<VALUETYPE> alpha = scalarProd(particles[i], theta1);
            d.setAlpha(alpha);
            d.sampling(particles[i]);
        }
        // weights
        std::vector<VALUETYPE> weights(N());
        for (size_t i = 0; i < N(); i++){
            std::vector<VALUETYPE> alpha = scalarProd(obserbed, theta2, 1);
            weights[i] = d.pdf(alpha);
        }
        // resampling
        discrete disc;
        disc.setProb(weights);
        std::vector< std::vector<VALUETYPE> > particlesTmp;
        for (size_t i = 0; i < N(); i++){
            particlesTmp.push_back(particles[disc.sampling()]);
        }
        particles = particlesTmp;
        
    };
    VALUETYPE MAP(){};


};







#endif /* defined(__hmm__dirichletParticleFilter__) */
