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

#include <opengm/graphicalmodel/graphicalmodel.hxx>
#include <opengm/graphicalmodel/space/simplediscretespace.hxx>
#include <opengm/operations/adder.hxx>
#include <opengm/operations/minimizer.hxx>
#include <opengm/functions/potts.hxx>
#include <opengm/inference/messagepassing/messagepassing.hxx>





#include "readCSV.h"





#include "boost/program_options.hpp"

struct options {
    std::string filename;
    char del;
    double p;
};


options parseOptions(int argc, char* argv[]) {

    options Opt;
    
    namespace po = boost::program_options;
    
    po::options_description desc("Options");
    desc.add_options()
    ("help", "This help message.")
    ("file", po::value<std::string>(), "data filename (csv)")
    ("p", po::value<double>()->default_value(0.9), "parameter p: default 0.9")
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

    
    return Opt;
}












int main ( int argc, char **argv )
{
    
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
                                   opengm::Adder,
                                   OPENGM_TYPELIST_2(opengm::ExplicitFunction<double>,
                                                     opengm::PottsFunction<double>  ),
                                   Space> Model;
    Space space(dataSize, nClass);
    Model gm(space);
    //
    //
    //
    
    
    //
    // construct gm
    //
    {
        // singleton
        const size_t shape[] = {nClass};

        for (size_t n = 0; n < dataSize; n++) {
            // function
            opengm::ExplicitFunction<double> f1(shape, shape + 1);
            
            for (int c = 0; c < nClass; c++)
                f1(c) = -std::log(data[n][c]);
            
            // factor per function
            typename Model::FunctionIdentifier fid1 = gm.addFunction(f1);
            size_t variableIndices1[] = {n};
            gm.addFactor(fid1, variableIndices1, variableIndices1 + 1);
        }
        
        // pairwise
        double same = -std::log(Opt.p); // p: parameter
        double diff = -std::log((1.0-Opt.p) / 2.0);
        
        opengm::PottsFunction<double> f2(nClass, nClass,
                                         same, // same labels
                                         diff); // different labels
        typename Model::FunctionIdentifier fid2 = gm.addFunction(f2);


        for (size_t n = 0; n < dataSize-1; n++) {
            size_t variableIndices2[] = {n, n+1};
            gm.addFactor(fid2, variableIndices2, variableIndices2 + 2);
        }
        

    }
    //
    //
    //
    
    
    
    //
    // infer
    //
    typedef opengm::BeliefPropagationUpdateRules<Model, opengm::Minimizer> UpdateRules;
    typedef opengm::MessagePassing<Model, opengm::Minimizer, UpdateRules, opengm::MaxDistance> BeliefPropagation;
    const size_t maxNumberOfIterations = 40;
    const double convergenceBound = 1e-7;
    const double damping = 0.5;
    BeliefPropagation::Parameter parameter(maxNumberOfIterations, convergenceBound, damping);
    parameter.useNormalization_ = false;
    BeliefPropagation bp(gm, parameter);
    bp.infer();
    
    std::vector<size_t> labeling(dataSize);
    bp.arg(labeling);
    
    
    typedef Model::IndependentFactorType IndependentFactor;
    for(size_t n = 0; n < dataSize; n++) {
        IndependentFactor ift;
        bp.marginal(n, ift);
        double sum = 0;
        for (size_t c = 0; c < nClass; c++) {
            sum += std::exp(-ift(c));
        }
        for (size_t c = 0; c < nClass; c++) {
            std::cout << std::exp(-ift(c))/sum << " ";
        }
        std::cout << std::endl;
    }
    //
    //
    //
    
    
    
    //
    // output to cout
    //
//    std::copy(labeling.begin(), labeling.end(),
//              std::ostream_iterator<float>(std::cout, "\n"));
    //
    //
    //
    
    
    
    
    
    
    
    
    return 0;
}
