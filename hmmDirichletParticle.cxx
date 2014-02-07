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

#include "dirichletParticleFilter.h"
#include "dataplot.h" // gnuplot class

#include "dirichletEstimate.h"

#include "boost/program_options.hpp"

struct options {
    std::string filename;

    double theta1;
    double theta2;
    size_t nParticles;
    
    std::string outfile;
};


options parseOptions(int argc, char* argv[]) {

    options Opt;
    
    namespace po = boost::program_options;
    
    po::options_description desc("Options");
    desc.add_options()
    ("help", "This help message.")
    ("file", po::value<std::string>(), "data filename (csv)")

    ("p1", po::value<double>()->default_value(100.0), "parameter theta1: default 100")
    ("p2", po::value<double>()->default_value(5.0), "parameter theta2: default 5")
  
    ("out", po::value<std::string>()->default_value(""), "output filename of particle average")
    
    ("npar", po::value<size_t>()->default_value(100), "number of particles: default 100")
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
    
    if (vm.count("p1"))
        Opt.theta1 = vm["p1"].as<double>();
    if (vm.count("p2"))
        Opt.theta2 = vm["p2"].as<double>();
    if (vm.count("npar"))
        Opt.nParticles = vm["npar"].as<size_t>();

    
    if (vm.count("out"))
        Opt.outfile = vm["out"].as<std::string>();
    
    
    return Opt;
}










int main ( int argc, char **argv )
{
    
    options Opt = parseOptions(argc, argv);

    //
    // read data from file
    //
    std::vector< std::vector< double > > data;
    data.reserve(1000);
    
    readFile(Opt.filename, data);
    
    data.shrink_to_fit();
    size_t dataSize = data.size();
    //size_t nClass = data[0].size();
    //
    //
    //
   
    
    //
    // particle filter
    //
    dirichletParticleFilter<double> dpf(Opt.nParticles);
    dpf.theta1 = Opt.theta1;
    dpf.theta2 = Opt.theta2;
    dpf.init(data[0]);
    //
    //
    //
    
    
    //
    // filtering
    //
    if (! Opt.outfile.empty() ) {
        std::ofstream ofs(Opt.outfile);
        std::vector< double > ave;
        for (size_t n = 0; n < dataSize; n++) {

            dpf.update(data[n]);

            
//            ave = dpf.average();
//            std::copy(ave.begin(), ave.end(),
//                      std::ostream_iterator<double>(ofs, ","));
//            ofs << std::endl;

            dirichletEstimate<double> de(dpf.getParticles());
            std::vector< double > alpha = de.mean();
            std::vector< double > variance = de.variance();
            alpha.insert(alpha.end(), variance.begin(), variance.end());

            std::copy(alpha.begin(), alpha.end(),
                      std::ostream_iterator<double>(ofs, ","));

            ofs << std::endl;
            
        }
        ofs.close();
    }
    //
    //
    //
    
    return 0;
}
