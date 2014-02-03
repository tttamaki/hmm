//
//  readCSV.cxx
//  hmm
//
//  Created by tamaki on 2014/02/03.
//  Copyright (c) 2014 tamaki. All rights reserved.
//

#include "readCSV.h"

void
readFile(const std::string &filename,
         std::vector< std::vector< float > > &data)
{
    
    std::ifstream ifs(filename.c_str());
    if (ifs.is_open()) {
        // std::cout << "read file: " << filename << std::endl;
    } else {
        std::cerr << "Fail to read:: " << filename << std::endl;
        exit(1);
    }
    
    typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
    
    std::vector< std::string > vecs;
    std::vector< float > vecf;
    std::string line;
    
    while (std::getline(ifs,line,'\r')) //  '\r' must be changed for different format!
    {
        Tokenizer tok(line);
        vecs.assign(tok.begin(),tok.end());
        vecf.resize(vecs.size());
        for (size_t i = 0; i < vecs.size(); i++) {
            vecf[i] = std::stof(vecs[i]);
        }
        
        data.push_back(vecf);
        
//        // print to std::cout
//        std::copy(vecf.begin(), vecf.end(),
//                  std::ostream_iterator<float>(std::cout, " "));
//        std::cout << std::endl;
        
    }
}

