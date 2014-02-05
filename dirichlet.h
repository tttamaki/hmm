//
//  dirichlet.h
//  hmm
//
//  Created by tamaki on 2014/02/04.
//  Copyright (c) 2014 tamaki. All rights reserved.
//

#ifndef __hmm__dirichlet__
#define __hmm__dirichlet__

#include <vector>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>


template<class VALUETYPE>
class randomDistribution {

public:
    randomDistribution(){
        gsl_rng_env_setup();
        rng = gsl_rng_alloc (gsl_rng_default);
        gsl_rng_set(rng, (unsigned long int)time(NULL));
    };
    ~randomDistribution(){
        gsl_rng_free(rng);
    };
    VALUETYPE pdf(const std::vector<VALUETYPE> &x){};
    VALUETYPE lnpdf(const std::vector<VALUETYPE> &x){};
    void sampling(std::vector<VALUETYPE> &theta) const {};
    
protected:
    gsl_rng *rng;
};

template<class VALUETYPE>
class dirichlet : public randomDistribution<VALUETYPE> {

public:
    dirichlet(): alpha_(std::vector<VALUETYPE>(0)){};
    ~dirichlet(){};
    VALUETYPE pdf(const std::vector<VALUETYPE> &x){
        return gsl_ran_dirichlet_pdf(K(), alpha_.data(), x.data());
    };
    VALUETYPE lnpdf(const std::vector<VALUETYPE> &x){
        return gsl_ran_dirichlet_lnpdf(K(), alpha_.data(), x.data());
    };
    void sampling(std::vector<VALUETYPE> &x) const {
        x.resize(K());
        gsl_ran_dirichlet(this->rng, K(), alpha_.data(), x.data());
    };
    
    void setAlpha(const std::vector<VALUETYPE> &alpha){
        alpha_ = alpha;
    };
    size_t K() const { return alpha_.size();};

protected:
    std::vector<VALUETYPE> alpha_;
};



template <class VALUETYPE>
class discrete : public randomDistribution<VALUETYPE> {
    gsl_ran_discrete_t *disc;
    
public:
    discrete(){};
    ~discrete(){};
    void setProb(const std::vector<VALUETYPE> &probability){
        disc = gsl_ran_discrete_preproc(probability.size(), probability.data());
    };
    VALUETYPE sampling(){
        return gsl_ran_discrete(this->rng, disc);
    };
};











#endif /* defined(__hmm__dirichlet__) */
