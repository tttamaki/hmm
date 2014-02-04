//
//  Created by Toru Tamaki on 2014/2/3.
//  Copyright (c) 2014 tamaki. All rights reserved.
//


#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <numeric>
#include <iterator>
#include <fstream>

#include <opengm/graphicalmodel/graphicalmodel.hxx>
#include <opengm/graphicalmodel/space/simplediscretespace.hxx>
#include <opengm/operations/adder.hxx>
#include <opengm/operations/minimizer.hxx>
#include <opengm/functions/potts.hxx>
#include <opengm/inference/messagepassing/messagepassing.hxx>





#include "readCSV.h"

#include "dirichlet.h"
#include "dataplot.h" // gnuplot class



#include "boost/program_options.hpp"

struct options {
    std::string filename;
    char del;
    double p;
    
    std::string outfile_alpha_z_n;
    std::string outfile_p_z_n_X;
    std::string outfile_marginal;
    std::string outfile_labels;
};


options parseOptions(int argc, char* argv[]) {

    options Opt;
    
    namespace po = boost::program_options;
    
    po::options_description desc("Options");
    desc.add_options()
    ("help", "This help message.")
    ("file", po::value<std::string>(), "data filename (csv)")
    ("p", po::value<double>()->default_value(0.9), "parameter p: default 0.9")
  
    ("azn", po::value<std::string>()->default_value(""), "output filename for alpha(z_n)")
    ("pznx", po::value<std::string>()->default_value(""), "output filename for p(z_n|X)")
    ("marginal", po::value<std::string>()->default_value(""), "output filename for marginal by OpenGM")
    ("label", po::value<std::string>()->default_value(""), "output filename for labeling result")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        exit(0);
    }
    
    if (vm.count("file"))
        Opt.filename = vm["file"].as<std::string>();
    else{
        std::cout << "no filename is specified." << std::endl
        << desc << std::endl;
        exit(1);
    }
    
    if (vm.count("p"))
        Opt.p = vm["p"].as<double>();

    
    if (vm.count("azn"))
        Opt.outfile_alpha_z_n = vm["azn"].as<std::string>();
    if (vm.count("pznx"))
        Opt.outfile_p_z_n_X   = vm["pznx"].as<std::string>();
    if (vm.count("marginal"))
        Opt.outfile_marginal  = vm["marginal"].as<std::string>();
    if (vm.count("label"))
        Opt.outfile_labels  = vm["label"].as<std::string>();
    
    
    return Opt;
}












int main ( int argc, char **argv )
{
    
    {
        std::vector<double> alpha(3);
        alpha[0] = 2;
        alpha[1] = 2;
        alpha[2] = 6;
        dirichlet<double> d;
        d.setAlpha(alpha);

        std::vector<double> x,y;
        Gnuplot g1("dots");
        for (int i = 0; i < 10000; i++) {
            d.sampling(alpha);
            x.push_back(alpha[0]);
            y.push_back(alpha[1]);
        }
        g1.reset_plot();
        g1.plot_xy(x,y);

        
        exit(0);
    }
    
    
    
    
    
    
    
    
    
    options Opt = parseOptions(argc, argv);

    //
    // read data from file
    //
    std::vector< std::vector< float > > data;
    data.reserve(1000);
    
    readFile(Opt.filename, data);
    
    data.shrink_to_fit();
    size_t dataSize = data.size();
    size_t nClass = data[0].size();
    //
    //
    //
   
    
    //
    // set gm
    //
    typedef opengm::SimpleDiscreteSpace<size_t, size_t> Space;
    typedef opengm::GraphicalModel<double,
                                   opengm::Multiplier,
                                   OPENGM_TYPELIST_2(opengm::ExplicitFunction<double>,
                                                     opengm::PottsFunction<double>  ),
                                   Space> Model;
    Space space(dataSize, nClass);
    Model gm(space);
    //
    //
    //
    
    
    //
    // construct gm of HMM
    //
    {
        // singleton
        const size_t shape[] = {nClass};

        for (size_t n = 0; n < dataSize; n++) {
            // function per node
            opengm::ExplicitFunction<double> f1(shape, shape + 1);
            assert(data[n].size() == nClass);

            for (size_t c = 0; c < nClass; c++)
                f1(c) = data[n][c];
            
            // factor per function
            typename Model::FunctionIdentifier fid1 = gm.addFunction(f1);
            size_t variableIndices1[] = {n};
            gm.addFactor(fid1, variableIndices1, variableIndices1 + 1);
        }
        // now gm[0] to gm[dataSize-1] are singleton factors
        
        
        // pairwise
        double same = Opt.p; // p: parameter
        double diff = (1.0-Opt.p) / (nClass-1); //2.0;
        
        opengm::PottsFunction<double> f2(nClass, nClass,
                                         same,  // cost for same labels
                                         diff); // cost for different labels
        typename Model::FunctionIdentifier fid2 = gm.addFunction(f2);

        for (size_t n = 0; n < dataSize-1; n++) {
            size_t variableIndices2[] = {n, n+1};
            gm.addFactor(fid2, variableIndices2, variableIndices2 + 2);
        }
        // now gm[dataSize] to gm[2*dataSize-2] are pairwize factors
    }
    //
    //
    //
    

    //
    // forward-backward algorithm implementaiton
    //
    
    std::vector< std::vector<double> > alpha(dataSize);
    // n=0
    alpha[0].resize(nClass);
    for (size_t c = 0; c < nClass; c++) {
        size_t val[] = {c};
        alpha[0][c] = gm[0](val) / nClass;
    }
    // n>=1
    for (size_t n = 1; n < dataSize; n++) {
        alpha[n].resize(nClass);
        std::vector<double> sum(dataSize, 0.0);
        
        for (size_t i = 0; i < nClass; i++)
            for (size_t c = 0; c < nClass; c++) {
                size_t val[] = {i,c};
                sum[i] += alpha[n-1][c] * gm[dataSize+n-1](val);
            }
        
        for (size_t c = 0; c < nClass; c++) {
            size_t val[] = {c};
            alpha[n][c] = gm[n](val) * sum[c];
        }
    }
    
    std::vector< std::vector<double> > beta(dataSize);
    // n=N
    beta[dataSize-1].resize(nClass);
    for (size_t c = 0; c < nClass; c++) {
        beta[dataSize-1][c] = 1.0;
    }
    // n<N
    for (int n = dataSize-2; n >= 0; n--) {
        beta[n].resize(nClass);
        for (size_t i = 0; i < nClass; i++) {
            beta[n][i] = 0;
            for (size_t c = 0; c < nClass; c++) {
                size_t val1[] = {c};
                size_t val2[] = {i,c};
                beta[n][i] += beta[n+1][c] * gm[n+1](val1) * gm[dataSize+n](val2);
            }
        }
    }

    double partitionZ = 0; // p(X)
    for (size_t c = 0; c < nClass; c++) {
        partitionZ += alpha[dataSize-1][c];
    }



    
    // print alpha(z_n)
    if (! Opt.outfile_alpha_z_n.empty() ) {
        std::ofstream ofs(Opt.outfile_alpha_z_n);
        for(size_t n = 0; n < dataSize; n++) {
            double sum = 0;
            for (size_t c = 0; c < nClass; c++)
                sum += alpha[n][c];
            
            for (size_t c = 0; c < nClass; c++)
                ofs << alpha[n][c]/sum << (c < nClass-1 ? "," : "");
            
            ofs << std::endl;
        }
        ofs.close();
    }
    
    // print p(z_n|X)
    if (! Opt.outfile_p_z_n_X.empty() ) {
        std::ofstream ofs(Opt.outfile_p_z_n_X);
        for(size_t n = 0; n < dataSize; n++) {
            for (size_t c = 0; c < nClass; c++)
                ofs << alpha[n][c] * beta[n][c] / partitionZ << (c < nClass-1 ? "," : "");
            
            ofs << std::endl;
        }
        ofs.close();
    }
    //
    //
    //
    
    
    
    //
    // infer
    //
    typedef opengm::BeliefPropagationUpdateRules<Model, opengm::Maximizer> UpdateRules;
    typedef opengm::MessagePassing<Model, opengm::Maximizer, UpdateRules, opengm::MaxDistance> BeliefPropagation;
    BeliefPropagation bp(gm);
    bp.infer();
    
    std::vector<size_t> labeling(dataSize);
    bp.arg(labeling);
    //
    //
    //
    
    //
    // output to cout
    //
    if (! Opt.outfile_labels.empty() ) {
        std::ofstream ofs(Opt.outfile_labels);
        std::copy(labeling.begin(), labeling.end(),
                  std::ostream_iterator<float>(ofs, "\n"));
        ofs.close();
    }
    //
    //
    //
    
    
    // print
    // std::cout << "openGM margnial" << std::endl;
    if (! Opt.outfile_marginal.empty() ) {
        std::ofstream ofs(Opt.outfile_marginal);
        
        typedef Model::IndependentFactorType IndependentFactor;
        for(size_t n = 0; n < dataSize; n++) {
            IndependentFactor ift;
            bp.marginal(n, ift);
            double sum = 0;
            for (size_t c = 0; c < nClass; c++)
                sum += ift(c);
            
            for (size_t c = 0; c < nClass; c++)
                ofs << ift(c)/sum << (c < nClass-1 ? "," : "");
            
            ofs << std::endl;
        }
        ofs.close();
    }
    //
    //
    //
    
    
    
    
    
    return 0;
}
