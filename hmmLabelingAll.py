#!/usr/bin/python
#coding:utf-8

#
# usage : python ./hmmtestall.py
#


import sys
import os


files=[
"data/A_NBI11-10153A.csv",
"data/A_NBI11-10200A.csv",
"data/A_NBI11-10200B.csv",
"data/A_NBI11-10230.csv",
"data/B_NBI11-10153B.csv",
"data/B_NBI11-10155.csv",
"data/B_NBI11-10161.csv",
"data/B_NBI11-10179.csv",
"data/B_NBI11-10201B.csv",
"data/C_NBI11_10244.csv",
"data/C_NBI11_IIa+IIc_1.csv",
"data/C_NBI11_IIa+IIc_2.csv"]


prange = [x / 10. for x in range(1,10,1)]

for i in range(len(files)):
    for p in range(len(prange)):
        outfile = files[i] + ".out"
        pdffile = files[i] + "." + str(prange[p]) + ".pdf"
        title = "\"" + files[i] + " (p=" +str(prange[p])+ ")\"";
        
        os.system("./hmmLabeling --file " + files[i] + " --p " + str(prange[p]) + " > " + outfile)
        os.system("Rscript --vanilla --slave hmmLabelingPlot.R " + outfile + " " + pdffile + " " + title)
