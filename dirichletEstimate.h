//
//  dirichletEstimate.h
//  hmm
//
//  Created by tamaki on 2014/02/07.
//  Copyright (c) 2014年 tamaki. All rights reserved.
//

#ifndef hmm_dirichletEstimate_h
#define hmm_dirichletEstimate_h

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

#include "ASA266/asa266.hpp"


template <class VAL>
class dirichletEstimate {

protected:
    std::vector< VAL > alpha_;
    bool computed;

public:
    dirichletEstimate() : computed(false){};
    dirichletEstimate(const std::vector< std::vector< VAL > > &particles)
    : computed(false){ estimate(particles); };
    ~dirichletEstimate(){};

    const std::vector< VAL > & getAlpha(){
        return alpha_;
    }
    
    std::vector< VAL >
    estimate(const std::vector< std::vector< VAL > > &particles)
    {
        int k = particles[0].size();
        assert(k >= 2);
        
        int n = particles.size();
        assert(k < n);
        
        int init = 1; // use the method of moments
        int ix = n; // leading dimension of the array X; n<= ix
        
        // input
        double *x = new double[ix*k];
        for (size_t i = 0; i < n; i++) {
            assert(particles[i].size() == k);
            for (size_t j = 0; j < k; j++) {
                x[i+j*ix] = particles[i][j]; // storing order in x is of FORTRAN!
            }
        }
        
        // output
        double *alpha = new double[k];
        double rlogl;
        double *v = new double[k*k];
        double *g = new double[k];
        int niter;
        double s;
        double eps;
        int ifault = 0;
        
        asa266::dirichlet_estimate (k, n, x, ix, init,
                                    alpha, rlogl, v, g, niter,
                                    s, eps, ifault );
        assert(ifault == 0);

        alpha_.resize(k);
        for (size_t j = 0; j < k; j++)
            alpha_[j] = alpha[j];
        
        delete [] alpha;
        delete [] x;
        delete [] v;
        delete [] g;
        
        
        computed = true;
        return alpha_;
    }
    
    std::vector< VAL >
    mean(){
        assert(computed == true);
        int k = alpha_.size();
        VAL sum = std::accumulate(alpha_.begin(), alpha_.end(), VAL(0));
        std::vector< VAL > meanvec(alpha_.size());
        for (size_t j = 0; j < k; j++) {
            meanvec[j] = alpha_[j] / sum;
        }
        return meanvec;
    }

    std::vector< VAL >
    mode(){
        assert(computed == true);
        int k = alpha_.size();
        VAL sum = std::accumulate(alpha_.begin(), alpha_.end(), VAL(0));
        std::vector< VAL > modevec(alpha_.size());
        for (size_t j = 0; j < k; j++) {
            modevec[j] = (alpha_[j]-1) / (sum-k);
        }
        return modevec;
    }
    
};

#endif
