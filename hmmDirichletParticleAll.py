#!/usr/bin/python
# -*- coding: utf-8 -*-


import matplotlib.pyplot as plt
import matplotlib
import os.path



def data2pdf(matrix, pdffilename, title, xlabel, ylabel):

    fig, ax = plt.subplots()
    fig.set_figwidth(10) # in inch
    fig.set_figheight(2) # in inch
    # fig.suptitle(title, fontsize=12)
    
    n=len(matrix)
    xs = []
    y1 = []
    y2 = []
    y3 = []
    for i in range(n):
        xs.append(i)
        y1.append(matrix[i][0])
        y2.append(matrix[i][1])
        y3.append(matrix[i][2])

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_ylim([0,1])

    ax.plot(xs, y1, 'b-', label=matrix.dtype.names[0])
    ax.plot(xs, y2, 'r-', label=matrix.dtype.names[1])
    ax.plot(xs, y3, 'g-', label=matrix.dtype.names[2])

    legend = ax.legend(loc='center right', frameon=False, fontsize=12) # http://matplotlib.org/api/axes_api.html?highlight=legend#matplotlib.axes.Axes.legend

    plt.savefig(pdffilename, bbox_inches='tight', format='pdf')
    plt.close()






csvfiles=[
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


p1range = [10, 100, 500, 1000] # theta1
p2range = [1, 5, 10]     # theta2


for filename in csvfiles:
    print(filename)
    filenamebase, ext = os.path.splitext(filename)

    # original data
    mat = matplotlib.mlab.csv2rec(filename, names=["A","B","C3"]) # http://matplotlib.org/api/mlab_api.html?highlight=csv2rec#matplotlib.mlab.csv2rec
    data2pdf(mat, filenamebase + ".pdf", filenamebase, 'frame number', 'values')
    
    for p1 in p1range:
        for p2 in p2range:
            fout = filenamebase + ".dpf." + str(p1) + "." + str(p2)
            
            command  = "./hmmDirichletParticle --file " + filename
            command += " --p1 " + str(p1)
            command += " --p2 " + str(p2)
            command += " --npar " + str(100)
            command += " --out " + fout + ".csv"
            
            print(command)
            os.system(command)
            
            mat = matplotlib.mlab.csv2rec(fout + ".csv", names=["A","B","C3"])
            data2pdf(mat, fout + ".pdf", fout, 'frame number', 'values')
