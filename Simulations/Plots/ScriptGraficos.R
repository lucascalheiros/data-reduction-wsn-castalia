# Testes

MeanCalculate <- function (filename) {
  test <- read.csv(file=filename, header=FALSE)
  test <- subset(test, select = -V1)
  temperatures <- test[!is.na(test)]
  return (mean(temperatures))
}

ConfidenceInterval <- function (filename) {
  
  test <- read.csv(file=filename, header=FALSE)
  test <- subset(test, select = -V1)
  temperatures <- test[!is.na(test)]
  temperatures <- t.test(temperatures, conf.level = 0.95)
  
  tt <- rbind(
    c(temperatures$conf.int[1], temperatures$estimate , temperatures$conf.int[2])
  )
  
  return (tt)
}

x_buffer <- c(400, 600, 800, 1000, 1500)

meanWihoutRed <- MeanCalculate("dataSensed-20nodes.csv")
y_meanWithoutRed <- c(meanWihoutRed,meanWihoutRed,meanWihoutRed,meanWihoutRed,meanWihoutRed)

dataWithoutRed <- ConfidenceInterval("dataSensed-20nodes.csv")

###################################
mean400BufferSize <- MeanCalculate("DropFirst-400bufferSize-20nodes.csv")
mean600BufferSize <- MeanCalculate("DropFirst-600bufferSize-20nodes.csv")
mean800BufferSize <- MeanCalculate("DropFirst-800bufferSize-20nodes.csv")
mean1000BufferSize <- MeanCalculate("DropFirst-1000bufferSize-20nodes.csv")
mean1500BufferSize <- MeanCalculate("DropFirst-1500bufferSize-20nodes.csv")


y_meanDropFirst <- c(mean400BufferSize, mean600BufferSize, mean800BufferSize, mean1000BufferSize, mean1500BufferSize)

erroDropFirst <- sum(abs(y_meanDropFirst - y_meanWithoutRed))/5

###################################
mean400BufferSize <- MeanCalculate("DropHistogram-400bufferSize-20nodes.csv")
mean600BufferSize <- MeanCalculate("DropHistogram-600bufferSize-20nodes.csv")
mean800BufferSize <- MeanCalculate("DropHistogram-800bufferSize-20nodes.csv")
mean1000BufferSize <- MeanCalculate("DropHistogram-1000bufferSize-20nodes.csv")
mean1500BufferSize <- MeanCalculate("DropHistogram-1500bufferSize-20nodes.csv")

y_meanDropHistogram <- c(mean400BufferSize, mean600BufferSize, mean800BufferSize, mean1000BufferSize, mean1500BufferSize)

erroDropHistogram <- sum(abs(y_meanDropHistogram - y_meanWithoutRed))/5


###################################
mean400BufferSize <- MeanCalculate("DropLast-400bufferSize-20nodes.csv")
mean600BufferSize <- MeanCalculate("DropLast-600bufferSize-20nodes.csv")
mean800BufferSize <- MeanCalculate("DropLast-800bufferSize-20nodes.csv")
mean1000BufferSize <- MeanCalculate("DropLast-1000bufferSize-20nodes.csv")
mean1500BufferSize <- MeanCalculate("DropLast-1500bufferSize-20nodes.csv")

y_meanDropLast <- c(mean400BufferSize, mean600BufferSize, mean800BufferSize, mean1000BufferSize, mean1500BufferSize)

erroDropLast <- sum(abs(y_meanDropLast - y_meanWithoutRed))/5

###################################
mean400BufferSize <- MeanCalculate("DropRandom-400bufferSize-20nodes.csv")
mean600BufferSize <- MeanCalculate("DropRandom-600bufferSize-20nodes.csv")
mean800BufferSize <- MeanCalculate("DropRandom-800bufferSize-20nodes.csv")
mean1000BufferSize <- MeanCalculate("DropRandom-1000bufferSize-20nodes.csv")
mean1500BufferSize <- MeanCalculate("DropRandom-1500bufferSize-20nodes.csv")

y_meanDropRandom <- c(mean400BufferSize, mean600BufferSize, mean800BufferSize, mean1000BufferSize, mean1500BufferSize)

erroDropRandom <- sum(abs(y_meanDropRandom - y_meanWithoutRed))/5

library(ggplot2)

qplot (xlab="Buffer Size", ylab="Average Temperature (ºC)") + 
  ggtitle("Comparison of Reduction Algorithms by Sink Buffer Size") +
  theme(plot.title = element_text(hjust=0.5)) +
  geom_line(aes(x_buffer, y_meanWithoutRed, colour="NoReduction"), size=1.5) +
  geom_line(aes(x_buffer, y_meanDropFirst, colour="DropFirst"), size=1.5) +
  geom_line(aes(x_buffer, y_meanDropHistogram, colour="DropHistogram"), size=1.5) +
  geom_line(aes(x_buffer, y_meanDropLast, colour="DropLast"), size=1.5) +
  geom_line(aes(x_buffer, y_meanDropRandom, colour="DropRandom"), size=1.5) +
  scale_colour_manual(name = "Reduction Algorithms",
                      breaks = c("NoReduction", "DropFirst", "DropHistogram", "DropLast","DropRandom"),
                      values = c(rgb(0.9,0.4,0, alpha=0.75), rgb(0,0.7,0.9, alpha=0.55), rgb(0.9,0,0, alpha=0.55), rgb(0,0,1, alpha=0.95), "black")
                      )

library(Rmisc)

extrem1 <- c(ConfidenceInterval("DropHistogram-400bufferSize-20nodes.csv")[1], ConfidenceInterval("DropHistogram-600bufferSize-20nodes.csv")[1], ConfidenceInterval("DropHistogram-800bufferSize-20nodes.csv")[1], ConfidenceInterval("DropHistogram-1000bufferSize-20nodes.csv")[1], ConfidenceInterval("DropHistogram-1500bufferSize-20nodes.csv")[1])
mean <- c(ConfidenceInterval("DropHistogram-400bufferSize-20nodes.csv")[2], ConfidenceInterval("DropHistogram-600bufferSize-20nodes.csv")[2], ConfidenceInterval("DropHistogram-800bufferSize-20nodes.csv")[2], ConfidenceInterval("DropHistogram-1000bufferSize-20nodes.csv")[2], ConfidenceInterval("DropHistogram-1500bufferSize-20nodes.csv")[2])
extrem2 <- c(ConfidenceInterval("DropHistogram-400bufferSize-20nodes.csv")[3], ConfidenceInterval("DropHistogram-600bufferSize-20nodes.csv")[3], ConfidenceInterval("DropHistogram-800bufferSize-20nodes.csv")[3], ConfidenceInterval("DropHistogram-1000bufferSize-20nodes.csv")[3], ConfidenceInterval("DropHistogram-1500bufferSize-20nodes.csv")[3])
data<- data.frame(x_buffer, extrem1, mean, extrem2)

ggplot(data, aes(x=x_buffer, y=mean, fill=mean)) + 
  ggtitle("Confidence Interval for DropHistogram") +
  geom_bar(position=position_dodge(), stat="identity") +
  theme(plot.title = element_text(hjust=0.5), plot.subtitle = element_text(hjust=0.5) )+
  geom_errorbar(aes(ymin=extrem1, ymax=extrem2), width=.1,position=position_dodge(.95)) +
  labs(y = "Average Temperature (ºC)", x = "Buffer Size")

extrem1 <- c(ConfidenceInterval("DropRandom-400bufferSize-20nodes.csv")[1], ConfidenceInterval("DropRandom-600bufferSize-20nodes.csv")[1], ConfidenceInterval("DropRandom-800bufferSize-20nodes.csv")[1], ConfidenceInterval("DropRandom-1000bufferSize-20nodes.csv")[1], ConfidenceInterval("DropRandom-1500bufferSize-20nodes.csv")[1])
mean <- c(ConfidenceInterval("DropRandom-400bufferSize-20nodes.csv")[2], ConfidenceInterval("DropRandom-600bufferSize-20nodes.csv")[2], ConfidenceInterval("DropRandom-800bufferSize-20nodes.csv")[2], ConfidenceInterval("DropRandom-1000bufferSize-20nodes.csv")[2], ConfidenceInterval("DropRandom-1500bufferSize-20nodes.csv")[2])
extrem2 <- c(ConfidenceInterval("DropRandom-400bufferSize-20nodes.csv")[3], ConfidenceInterval("DropRandom-600bufferSize-20nodes.csv")[3], ConfidenceInterval("DropRandom-800bufferSize-20nodes.csv")[3], ConfidenceInterval("DropRandom-1000bufferSize-20nodes.csv")[3], ConfidenceInterval("DropRandom-1500bufferSize-20nodes.csv")[3])
data<- data.frame(x_buffer, extrem1, mean, extrem2)

ggplot(data, aes(x=x_buffer, y=mean, fill=mean)) + 
  ggtitle("Confidence Interval for DropRandom") +
  geom_bar(position=position_dodge(), stat="identity") +
  theme(plot.title = element_text(hjust=0.5), plot.subtitle = element_text(hjust=0.5) )+
  geom_errorbar(aes(ymin=extrem1, ymax=extrem2), width=.1,position=position_dodge(.95)) +
  labs(y = "Average Temperature (ºC)", x = "Buffer Size")

algorithms <- c("DropFirst", "DropHistogram", "DropLast", "DropRandom")
errors <- c(erroDropFirst, erroDropHistogram, erroDropLast, erroDropRandom)
data <- data.frame(algorithms, errors)

ggplot(data, aes(x=algorithms, y=errors, fill=algorithms)) + 
  ggtitle("MAE of Reduction Algorithms - varyBuffer") +
  geom_bar(position=position_dodge(), stat="identity") +
  theme(plot.title = element_text(hjust=0.5), plot.subtitle = element_text(hjust=0.5) )+
  labs(y = "Mean Absolute Error", x = "Reduction Algorithms")

library(ggplot2)
