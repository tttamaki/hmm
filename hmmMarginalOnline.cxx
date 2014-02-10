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






#include "readCSV.h"





#include "boost/program_options.hpp"

struct options {
    std::string filename;
    char del;
    double p;
    
    std::string outfile_alpha_z_n;
};


options parseOptions(int argc, char* argv[]) {

    options Opt;
    
    namespace po = boost::program_options;
    
    po::options_description desc("Options");
    desc.add_options()
    ("help", "This help message.")
    ("file", po::value<std::string>(), "data filename (csv)")
    ("p", po::value<double>()->default_value(0.9), "parameter p: default 0.9")
  
    ("azn", po::value<std::string>()->default_value(""), "output filename for p(z_n|x1..n)")
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
   
    

    
    
    // pairwise
    double same = Opt.p; // p: parameter
    double diff = (1.0-Opt.p) / (nClass-1); //2.0;
    

    //
    // forward algorithm implementaiton
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
        
        // update
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
        // update done;
        
        
    }
    
    
    
    
    // print p(z_n|x1..n)
    if (! Opt.outfile_alpha_z_n.empty() ) {
        std::ofstream ofs(Opt.outfile_alpha_z_n);
        for(size_t n = 0; n < dataSize; n++) {

            for (size_t c = 0; c < nClass; c++)
                ofs << alphaHat[n][c] << (c < nClass-1 ? "," : "");
            
            ofs << std::endl;
        }
        ofs.close();
    }

    
    
    return 0;
}
