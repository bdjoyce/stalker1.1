#Parsing functions for Stalker1.1 
#https://github.com/bdjoyce/stalker1.1




#parses a stalker txt file generated by the tool
importStalker1p1File <- function(filename) {

  #read raw data  
  filedata <- read.csv(filename, sep=",", header=FALSE)
  
  #calculate number of entries, remove 2 for a blank line and incomplete line
  dataend = length(filedata$V1)-2
  
  #chop off the first 5 lines (inti/cali)
  #then return the rest as a dataframe for analysis
  return_data = data.frame(
    millis = as.numeric(filedata$V1[5:dataend]),
    loadcell = as.numeric(filedata$V2[5:dataend]),
    distance = as.numeric(filedata$V3[5:dataend]),
    accel_x = as.numeric(filedata$V4[5:dataend]),
    accel_y = as.numeric(filedata$V5[5:dataend]),
    accel_z = as.numeric(filedata$V6[5:dataend]),
    angle = as.numeric(filedata$V7[5:dataend])
  )
  
  return(return_data)
}


#looks at stalker data and tries to figure out the distance (height) reading
stalker1p1Dist <- function(data) {
  
  
  #take out any really big readings because the stick is only 1m high so can't
  #have any valid measurements more than 1000
  sdata = subset(data, distance<1000)
  
  #first do a sanity check, height should be inversely correlated with angle
  #because of beam spread, if it is not, probably measuring a plant or something
  #and the distance data can be disregarded
  hacor = cor(sdata$angle, sdata$distance)
  
  #adjust strictness for correlation here
  if (hacor > -0.5) {
    return(NA)
  }

  #take data when stick is vertical at measurement start  
  sdata = subset(sdata, millis<7000)
  sdata = subset(sdata, angle<5)
  sdata = subset(sdata, angle>-5)
  
  return(mean(sdata$distance))
}


#looks at stalker data and tries to figure out the force reading at that
#angle - this will make it much easier to analyze data turning each
#measurement file into a single number
stalker1p1Loadcell <- function(data, meas_angle=45, meas_spread_down=-15, meas_spread_up=5) {
  
  #first throw away all data after the angle that we care about
  #this will remove the effect from inelastic strain, and avoid the
  #turn-around since the jerk of the instrument changing direction will
  #affect the load cell and the angle calculation
  anglemillistime = min(data$millis[which(data$angle>(meas_angle+meas_spread_up))])
  sdata = subset(data, millis<=anglemillistime)
  
  #next select only the points with angles near to the desired measurement
  #angle (+/- the spread) so the data will be closer to linear in this
  #region
  sdata = subset(sdata, angle>=(meas_angle-abs(meas_spread_down)))
  sdata = subset(sdata, angle<=(meas_angle+meas_spread_up))
  
  #if not enough points for a linear regression then return NA
  #could also sanity check for more points for accuracy
  if (length(sdata$angle) < 2) {
    return(NA)
  }
  
  
  #next do a linear correlation for this region's points
  model = lm(data=sdata, formula=loadcell~angle)
  
  #and take the linear model's estimate of force for measurement angle
  intercept = model$coefficients[[1]]
  slope = model$coefficients[[2]]
  model_loadcell = slope*meas_angle + intercept
  
  return(model_loadcell)
}
