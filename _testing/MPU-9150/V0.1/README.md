V 0.1 has accelo values, accelo angles, gyro values and gyro angles calculated based on their efficient timespan of almost accurate readings.

- Accelo angles are counted by Tilt computation shown in the pdf located at this.parent.

- Gyro values are raw values. To find angular velocity from gyro raw values, divide them by gyroscale value(131).

- Computing orientation from the gyroscope sensor is different,since the gyroscope measures angular velocity (the rate of change in    
  orientation angle), not angular orientation itself.To compute the orientation, we must first initialize the sensor position with a known   value (possibly from the accelerometer),then measure the angular velocity (ω) around the X, Y and Z axes at measured intervals (Δt).       Then ω × Δt = change in angle.The new orientation angle will be the original angle plus this change.
  
- Gyroscopic value multiplied by the time between sensor readings, gives the change in angular position.If we save the previous angular     position, we simply add the computed change each time to find the new value.

- Simplified formula to find filtered angle :
        Filtered Angle = α × (Gyroscope Angle) + (1 − α) × (Accelerometer Angle)  where
        α = τ/(τ + Δt)   and   (Gyroscope Angle) = (Last Measured Filtered Angle) + ω×Δt
        Δt = sampling rate= 1s, τ = time constant greater than timescale of typical accelerometer noise
