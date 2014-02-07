OPENGM_EXTSRC = /Users/tamaki/dev/opengm/src/external
OPENGM_EXTLIB = /Users/tamaki/dev/opengm/build/src/external
OPENGM_INC    = /Users/tamaki/dev/opengm/include
EXTLIBS = -lboost_program_options-mt



OPTIONS =  -I$(OPENGM_INC) \
			`pkg-config --cflags opencv eigen3` `pkg-config --libs opencv eigen3` \
			-L$(OPENGM_EXTLIB) $(EXTLIBS) $(CFLAGS) -Wall -std=c++11 \
            -L/opt/local/lib -I/usr/local/include
ifdef DEBUG
  OPTIONS+= -O0 -g -DDEBUG
else
  OPTIONS+= -O3 -DNDEBUG
endif

all:  hmmLabeling hmmMarginals hmmDirichletParticle


hmmLabeling: hmmLabeling.cxx readCSV.cxx
	g++ -o hmmLabeling hmmLabeling.cxx readCSV.cxx \
	$(OPTIONS)

hmmMarginals: hmmMarginals.cxx readCSV.cxx
	g++ -o hmmMarginals hmmMarginals.cxx readCSV.cxx \
	$(OPTIONS)
    
hmmDirichletParticle: hmmDirichletParticle.cxx readCSV.cxx dirichlet.h dirichletEstimate.h dirichletParticleFilter.h
	g++ -o hmmDirichletParticle hmmDirichletParticle.cxx readCSV.cxx \
    asa266/asa266.cpp \
	$(OPTIONS) -lgsl -I../gnuplot-cpp/example2 -I../gnuplot-cpp

clean:
	rm -f hmmLabeling hmmMarginals hmmDirichletParticle
