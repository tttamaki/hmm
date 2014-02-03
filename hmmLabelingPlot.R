#
# usage:  Rscript --vanilla --slave hmmLabelingPlot.R csvfile pdffile title
#


args <- commandArgs(trailingOnly = T)

if(length(args) < 2) {
    write("usage: csvfile pdffile [title]", stderr())
    q()
}

filename <- args[1]

title <- filename
if(length(args) == 3) {
    title <- args[3]
}

#pdffile <- paste(c(filename,".pdf"), collapse="")
pdffile <- args[2]




#color of labels
blue <- rgb(79, 129, 189, maxColorValue=255)
red <- rgb(192, 80, 77, maxColorValue=255)
green <- rgb(155, 187, 89, maxColorValue=255)



table1 <- read.csv(filename, header=F, col.name=c("LABEL") )
N <- as.integer(length(table1$LABEL))
labels <- table1$LABEL + 1

lab1 <- replace(labels, which(labels != 1), NA)
lab2 <- replace(labels, which(labels != 2), NA)
lab3 <- replace(labels, which(labels != 3), NA)



pdf(pdffile, width=8, height=2)


par(mar=c(3,0.3,2,0.3), mgp=c(1.8,0.8,0))

if(sum(is.na(lab1)) != N){
    barplot(lab1, col=blue, yaxt="n", space=0, border=blue)
    par(new=T)
}
if(sum(is.na(lab2)) != N){
    barplot(lab2, col=red, yaxt="n", xlab="frame number", main=title, space=0, border=red)
    par(new=T)
}
if(sum(is.na(lab3)) != N){
    barplot(lab3, col=green, yaxt="n", space=0, border=green)
}

axis(1, at=seq(0,N,50), labels=seq(0,N,50))

dev.off()

q()
