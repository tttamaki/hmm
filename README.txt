
* hmmLabeling: An implemenation of sequence labeling with HMM/DP

Reference:
Conf Proc IEEE Eng Med Biol Soc. 2013;2013:4831-4. doi: 10.1109/EMBC.2013.6610629.
Labeling colorectal NBI zoom-videoendoscope image sequences with MRF and SVM.
Hirakawa T, Tamaki T, Raytchev B, Kaneda K, Koide T, Yoshida S, Kominami Y, Matsuo T, Miyaki R, Tanaka S.

Usage:
- put csvfiles
- modify "files" variable in hmmLabelingAll.py for the csvfiles
- modify \r of line "std::getline(ifs,line,'\r')" in readCSV.cxx
  IF the line delimitor of the csvfiles is not '\r' (usually '\n')
- run "python hmmLabelingAll.py"
- you'll get *.out and *.pdf as results
- compile hmmLabeling.tex to put pdfs of a csvfile in a singel page

Requirements:
- OpenGM
- python
- R
- (pdflatex)


* hmmMarginals: Computing marginals of sequence labeling with HMM

Usage:
- put csvfiles
- modify "csvfiles" variable in hmmMarginalsAll.py for the csvfiles
- run "python hmmMarginalsAll.py"
- you'll get a bunch of *.csv and *.pdf as results
-- *.azn.*:   marginals at variable z_n given data x_1...n; p(z_n|X_1...n)
              = normalized alpha message of forward-backward algorithm
-- *.pznx.*:  marginals at variable z_n given all data x_1...N; p(z_n|X_1...N)
              = gamma message of forward-backward algorithm
-- *.marginals.*: marginal of variable n computed by OpenGM
- compile hmmMarginals.tex to put pdfs of a csvfile in a singel page

Requirements:
- OpenGM
- python
- (pdflatex)
