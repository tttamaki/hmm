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







inline
double potts(const size_t label1,
             const size_t label2,
             const double same,  // cost for same labels
             const double diff) // cost for different labels
{
    return (label1 == label2) ? same : diff;
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

    // pairwise
    double same = Opt.p; // p: parameter
    double diff = (1.0-Opt.p) / (nClass-1); //2.0;
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
    
//    std::vector< std::vector<double> > alpha(dataSize);
//    // n=0
//    alpha[0].resize(nClass);
//    for (size_t c = 0; c < nClass; c++) {
//        size_t val[] = {c};
//        alpha[0][c] = gm[0](val) / nClass;
//    }
//    // n>=1
//    for (size_t n = 1; n < dataSize; n++) {
//        alpha[n].resize(nClass);
//        std::vector<double> sum(dataSize, 0.0);
//        
//        for (size_t i = 0; i < nClass; i++) {
//            for (size_t c = 0; c < nClass; c++) {
//                size_t val[] = {i,c};
//                sum[i] += alpha[n-1][c] * gm[dataSize+n-1](val);
//            }
//        }
//        for (size_t c = 0; c < nClass; c++) {
//            size_t val[] = {c};
//            alpha[n][c] = gm[n](val) * sum[c];
//        }
//    }
//    
    
    
    std::vector< std::vector<double> > alphaHat(dataSize);
    std::vector<double> cHat(dataSize);
    // n=0
    alphaHat[0].resize(nClass);
    for (size_t c = 0; c < nClass; c++) {
        alphaHat[0][c] = data[0][c] / nClass;
    }
    cHat[0] = std::accumulate(alphaHat[0].begin(), alphaHat[0].end(), 0.0);
    for (size_t c = 0; c < nClass; c++) {
        alphaHat[0][c] /= cHat[0];
    }
    // n>=1
    for (size_t n = 1; n < dataSize; n++) {
        alphaHat[n].resize(nClass);
        std::vector<double> sum(dataSize, 0.0);
        
        for (size_t i = 0; i < nClass; i++) {
            for (size_t c = 0; c < nClass; c++) {
                sum[i] += alphaHat[n-1][c] * potts(i,c, same, diff);
            }
        }
        for (size_t i = 0; i < nClass; i++) {
            alphaHat[n][i] = data[n][i] * sum[i];
        }

        cHat[n] = std::accumulate(alphaHat[n].begin(), alphaHat[n].end(), 0.0);
        for (size_t c = 0; c < nClass; c++) {
            alphaHat[n][c] /= cHat[n];
        }

    }
    
    
    
    
    
//    std::vector< std::vector<double> > logAlphaOnline(dataSize);
//    // n=0
//    logAlphaOnline[0].resize(nClass);
//    for (size_t c = 0; c < nClass; c++) {
//        logAlphaOnline[0][c] = std::log(data[0][c] / nClass);
//    }
//    // n>=1
//    for (size_t n = 1; n < dataSize; n++) {
//        logAlphaOnline[n].resize(nClass);
//        std::vector<double> logSum(dataSize, 0.0);
//        
//        for (size_t i = 0; i < nClass; i++) {
//            std::vector<double> whichIsMax(nClass);
//           
//            for (size_t c = 0; c < nClass; c++) {
//                whichIsMax[c] = (logAlphaOnline[n-1][c] + std::log(potts(i,c, same, diff)));
//            }
//            double maxVal = *std::max_element(whichIsMax.begin(), whichIsMax.end());
//            for (size_t c = 0; c < nClass; c++) { logSum[i] += exp(whichIsMax[c] - maxVal); }
//            logSum[i] = std::log(logSum[i]) + maxVal;
//        }
//        
//        for (size_t c = 0; c < nClass; c++) {
//            logAlphaOnline[n][c] = std::log(data[n][c]) + logSum[c];
//        }
//    }
//    
//    
//    for (size_t n = 0; n < dataSize; n++) {
//        for (size_t c = 0; c < nClass; c++) {
//            alpha[n][c] = exp(logAlphaOnline[n][c]);
//        }
//    }

    
    
    
    
    
    
//    std::vector< std::vector<double> > beta(dataSize);
//    // n=N
//    beta[dataSize-1].resize(nClass);
//    for (size_t c = 0; c < nClass; c++) {
//        beta[dataSize-1][c] = 1.0;
//    }
//    // n<N
//    for (int n = dataSize-2; n >= 0; n--) {
//        beta[n].resize(nClass);
//        for (size_t i = 0; i < nClass; i++) {
//            beta[n][i] = 0;
//            for (size_t c = 0; c < nClass; c++) {
//                size_t val1[] = {c};
//                size_t val2[] = {i,c};
//                beta[n][i] += beta[n+1][c] * gm[n+1](val1) * gm[dataSize+n](val2);
//            }
//        }
//    }
    
    
    
    std::vector< std::vector<double> > betaHat(dataSize);
    // n=N
    betaHat[dataSize-1].resize(nClass);
    for (size_t c = 0; c < nClass; c++) {
        betaHat[dataSize-1][c] = 1.0;
    }
    // n<N
    for (int n = dataSize-2; n >= 0; n--) {
        betaHat[n].resize(nClass);
        for (size_t i = 0; i < nClass; i++) {
            betaHat[n][i] = 0;
            for (size_t c = 0; c < nClass; c++) {
                betaHat[n][i] += betaHat[n+1][c] * data[n+1][c] * potts(i,c,same,diff);
            }
            betaHat[n][i] /= cHat[n+1];
        }
    }
    
    
    
    
//    std::vector< std::vector<double> > logBeta(dataSize);
//    // n=N
//    logBeta[dataSize-1].resize(nClass);
//    for (size_t c = 0; c < nClass; c++) {
//        logBeta[dataSize-1][c] = 0.0;
//    }
//    // n<N
//    for (int n = dataSize-2; n >= 0; n--) {
//        logBeta[n].resize(nClass);
//        for (size_t i = 0; i < nClass; i++) {
//            std::vector<double> whichIsMax(nClass);
//            logBeta[n][i] = 0;
//            for (size_t c = 0; c < nClass; c++) {
//                whichIsMax[c] = (logBeta[n+1][c] + std::log(data[n+1][c])
//                                 + std::log(potts(i,c,same,diff)));
//            }
//            double maxVal = *std::max_element(whichIsMax.begin(), whichIsMax.end());
//            for (size_t c = 0; c < nClass; c++) { logBeta[n][i] += exp(whichIsMax[c] - maxVal); }
//            logBeta[n][i] = std::log(logBeta[n][i]) + maxVal;
//        }
//    }
//    
//    for (size_t n = 0; n < dataSize; n++) {
//        for (size_t c = 0; c < nClass; c++) {
//            beta[n][c] = exp(logBeta[n][c]);
//        }
//    }
    
    

//    // check
//    for (size_t n = 0; n < dataSize; n++) {
//        std::cout << "a ";
//        for (size_t c = 0; c < nClass; c++) {
//            std::cout << alpha[n][c] << " ";
//        }
//        std::cout << " aH ";
//        double cm = 1;
//        for (size_t m = 0; m <= n; m++) {
//            cm *= cHat[m];
//        }
//        for (size_t c = 0; c < nClass; c++) {
//            std::cout << cm * alphaHat[n][c] << " ";
//        }
//        std::cout << " aHnn ";
//        for (size_t c = 0; c < nClass; c++) {
//            std::cout << alphaHat[n][c] << " ";
//        }
//        std::cout << "b ";
//        for (size_t c = 0; c < nClass; c++) {
//            std::cout << beta[n][c] << " ";
//        }
//        std::cout << " bH ";
//        cm = 1;
//        for (size_t m = n+1; m < dataSize; m++) {
//            cm *= cHat[m];
//        }
//        for (size_t c = 0; c < nClass; c++) {
//            std::cout << cm * betaHat[n][c] << " ";
//        }
//        
//        
//        std::cout << std::endl;
//        
//    }

    
    
    
    

//    double partitionZ = 0; // p(X)
//    for (size_t c = 0; c < nClass; c++) {
//        partitionZ += alpha[dataSize-1][c];
//    }



    
    // print alpha(z_n)
//    if (! Opt.outfile_alpha_z_n.empty() ) {
//        std::ofstream ofs(Opt.outfile_alpha_z_n);
//        for(size_t n = 0; n < dataSize; n++) {
//            double sum = 0;
//            for (size_t c = 0; c < nClass; c++)
//                sum += alpha[n][c];
//            
//            for (size_t c = 0; c < nClass; c++)
//                ofs << alpha[n][c]/sum << (c < nClass-1 ? "," : "");
//            
//            ofs << std::endl;
//        }
//        ofs.close();
//    }
    if (! Opt.outfile_alpha_z_n.empty() ) {
        std::ofstream ofs(Opt.outfile_alpha_z_n);
        for(size_t n = 0; n < dataSize; n++) {

            for (size_t c = 0; c < nClass; c++)
                ofs << alphaHat[n][c] << (c < nClass-1 ? "," : "");
            
            ofs << std::endl;
        }
        ofs.close();
    }

    
    // print p(z_n|X)
//    if (! Opt.outfile_p_z_n_X.empty() ) {
//        std::ofstream ofs(Opt.outfile_p_z_n_X);
//        for(size_t n = 0; n < dataSize; n++) {
//            for (size_t c = 0; c < nClass; c++)
//                ofs << alpha[n][c] * beta[n][c] / partitionZ << (c < nClass-1 ? "," : "");
//            
//            ofs << std::endl;
//        }
//        ofs.close();
//    }
    if (! Opt.outfile_p_z_n_X.empty() ) {
        std::ofstream ofs(Opt.outfile_p_z_n_X);
        for(size_t n = 0; n < dataSize; n++) {
            for (size_t c = 0; c < nClass; c++)
                ofs << alphaHat[n][c] * betaHat[n][c] << (c < nClass-1 ? "," : "");
            
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
