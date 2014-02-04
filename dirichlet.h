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
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>


template<class VALUETYPE>
class randomDistribution {

public:
    randomDistribution(){
        gsl_rng_env_setup();
        rng = gsl_rng_alloc (gsl_rng_default);
    };
    ~randomDistribution(){
        gsl_rng_free(rng);
    };
    VALUETYPE pdf(const std::vector<VALUETYPE> &theta){};
    VALUETYPE lnpdf(const std::vector<VALUETYPE> &theta){};
    void sampling(std::vector<VALUETYPE> &theta) const {};
    
protected:
    gsl_rng_type *rng_type;
    gsl_rng *rng;
};

template<class VALUETYPE>
class dirichlet : public randomDistribution<VALUETYPE> {

public:
    dirichlet(): alpha_(std::vector<VALUETYPE>(0)){};
    ~dirichlet(){};
    VALUETYPE pdf(const std::vector<VALUETYPE> &theta){
        return gsl_ran_dirichlet_pdf(K(), alpha_.data(), theta.data());
    };
    VALUETYPE lnpdf(const std::vector<VALUETYPE> &theta){
        return gsl_ran_dirichlet_lnpdf(K(), alpha_.data(), theta.data());
    };
    void sampling(std::vector<VALUETYPE> &theta) const {
        theta.resize(K());
        gsl_ran_dirichlet(this->rng, K(), alpha_.data(), theta.data());
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
