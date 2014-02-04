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
        rng_type = gsl_rng_default;
        rng = gsl_rng_alloc (rng_type);
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
        gsl_ran_dirichlet_pdf(K(), alpha_, theta);
    };
    VALUETYPE lnpdf(const std::vector<VALUETYPE> &theta){
        gsl_ran_dirichlet_lnpdf(K(), alpha_, theta);
    };
    void sampling(std::vector<VALUETYPE> &theta) const {
        gsl_ran_dirichlet(this->rng, K(), alpha_.data(), theta.data());
    };
    
    void setAlpha(const std::vector<VALUETYPE> &alpha){
        alpha_ = alpha;
    };
    size_t K(){ alpha_.size();};

protected:
    std::vector<VALUETYPE> alpha_;
};

#endif /* defined(__hmm__dirichlet__) */
