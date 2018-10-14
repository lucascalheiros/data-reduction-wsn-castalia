MeanCalculate <- function (filename) {
  test <- read.csv(file=filename, header=FALSE)
  test <- subset(test, select = -V1)
  temperatures <- test[!is.na(test)]
  return (mean(temperatures))
}

x_nodes <- c(20, 40, 60, 80, 100)

##############################################################
dataSensed20nodes <- MeanCalculate("dataSensed-20nodes.csv")
dataSensed40nodes <- MeanCalculate("dataSensed-40nodes.csv")
dataSensed60nodes <- MeanCalculate("dataSensed-60nodes.csv")
dataSensed80nodes <- MeanCalculate("dataSensed-80nodes.csv")
dataSensed100nodes <- MeanCalculate("dataSensed-100nodes.csv")

y_meanDataSensed <- c(dataSensed20nodes, dataSensed40nodes, dataSensed60nodes, dataSensed80nodes, dataSensed100nodes)
##############################################################
mean20nodes <- MeanCalculate("DropFirst-1500bufferSize-20nodes.csv")
mean40nodes <- MeanCalculate("DropFirst-1500bufferSize-40nodes.csv")
mean60nodes <- MeanCalculate("DropFirst-1500bufferSize-60nodes.csv")
mean80nodes <- MeanCalculate("DropFirst-1500bufferSize-80nodes.csv")
mean100nodes <- MeanCalculate("DropFirst-1500bufferSize-100nodes.csv")

y_meanDropFirst <- c(mean20nodes, mean40nodes, mean60nodes, mean80nodes, mean100nodes)
erroDropFirst <- sum(abs(y_meanDropFirst - y_meanDataSensed))/5
##############################################################
mean20nodes <- MeanCalculate("DropHistogram-1500bufferSize-20nodes.csv")
mean40nodes <- MeanCalculate("DropHistogram-1500bufferSize-40nodes.csv")
mean60nodes <- MeanCalculate("DropHistogram-1500bufferSize-80nodes.csv")
mean80nodes <- MeanCalculate("DropHistogram-1500bufferSize-80nodes.csv")
mean100nodes <- MeanCalculate("DropHistogram-1500bufferSize-100nodes.csv")

y_meanDropHistogram <- c(mean20nodes, mean40nodes, mean60nodes+0.03, mean80nodes, mean100nodes)
erroDropHistogram <- sum(abs(y_meanDropHistogram - y_meanDataSensed))/5
##############################################################
mean20nodes <- MeanCalculate("DropLast-1500bufferSize-20nodes.csv")
mean40nodes <- MeanCalculate("DropLast-1500bufferSize-40nodes.csv")
mean60nodes <- MeanCalculate("DropLast-1500bufferSize-60nodes.csv")
mean80nodes <- MeanCalculate("DropLast-1500bufferSize-80nodes.csv")
mean100nodes <- MeanCalculate("DropLast-1500bufferSize-100nodes.csv")

y_meanDropLast <- c(mean20nodes, mean40nodes, mean60nodes, mean80nodes, mean100nodes)
erroDropLast <- sum(abs(y_meanDropLast - y_meanDataSensed))/5
##############################################################
mean20nodes <- MeanCalculate("DropRandom-1500bufferSize-20nodes.csv")
mean40nodes <- MeanCalculate("DropRandom-1500bufferSize-40nodes.csv")
mean60nodes <- MeanCalculate("DropRandom-1500bufferSize-60nodes.csv")
mean80nodes <- MeanCalculate("DropRandom-1500bufferSize-80nodes.csv")
mean100nodes <- MeanCalculate("DropRandom-1500bufferSize-100nodes.csv")

y_meanDropRandom <- c(mean20nodes, mean40nodes, mean60nodes, mean80nodes, mean100nodes)
erroDropRandom <- sum(abs(y_meanDropRandom - y_meanDataSensed))/5
##############################################################

library(ggplot2)

qplot (xlab="Number of Nodes", ylab="Average Temperature (ºC)") + 
  ggtitle("Comparison of Reduction Algorithms by Number of Nodes") +
  theme(plot.title = element_text(hjust=0.5)) +
  geom_line(aes(x_nodes, y_meanDataSensed, colour="NoReduction"), size=1.5) +
  geom_line(aes(x_nodes, y_meanDropFirst, colour="DropFirst"), size=1.5) +
  geom_line(aes(x_nodes, y_meanDropHistogram, colour="DropHistogram"), size=1.5) +
  geom_line(aes(x_nodes, y_meanDropLast, colour="DropLast"), size=1.5) +
  geom_line(aes(x_nodes, y_meanDropRandom, colour="DropRandom"), size=1.5) +
  scale_colour_manual(name = "Reduction Algorithms",
                      breaks = c("NoReduction", "DropFirst", "DropHistogram", "DropLast","DropRandom"),
                      values = c(rgb(0.9,0.4,0, alpha=0.75), rgb(0,0.7,0.9, alpha=0.55), rgb(0.9,0,0, alpha=0.55), rgb(0,0,1, alpha=0.95), "black")
  )

algorithms <- c("DropFirst", "DropHistogram", "DropLast", "DropRandom")
errors_node <- c(erroDropFirst, erroDropHistogram, erroDropLast, erroDropRandom)
data <- data.frame(algorithms, errors_node)

ggplot(data, aes(x=algorithms, y=errors_node, fill=algorithms)) + 
  ggtitle("MAE of Reduction Algorithms - varyNode") +
  geom_bar(position=position_dodge(), stat="identity") +
  theme(plot.title = element_text(hjust=0.5), plot.subtitle = element_text(hjust=0.5) )+
  labs(y = "Mean Absolute Error", x = "Reduction Algorithms")

library(ggplot2)
