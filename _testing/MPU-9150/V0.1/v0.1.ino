#include<Wire.h>
#define Gyroscale 131

const int MPU_addr = 0x68; // I2C address
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
double timeStep, time , timePrev;
int Ax, Ay, Az, Gx, Gy, Gz, Rx, Ry, Rz;

byte i;
void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU)
  Wire.endTransmission(true);
  Serial.begin(9600);
  time = millis();
  i = 0;
}
void loop() {
  /* set up time for integration */
  timePrev = time;
  time = millis();
  timeStep = (time - timePrev) / 1000; // time-step in s

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  /*dividing the raw gyroscope values by 131 gives angular velocity in degrees per second*/
  Gx = GyX / Gyroscale; Gy = GyY / Gyroscale; Gz = GyZ / Gyroscale;

  /*accelerometer angles*/
  Ax = (180 / PI) * atan(AcX / sqrt(AcY * AcY + AcZ * AcZ));
  Ay = (180 / PI) * atan(AcY / sqrt(AcX * AcX + AcZ * AcZ));
  Az = (180 / PI) * atan(sqrt(AcX * AcX + AcY * AcY) / AcZ);

  /*Computing orientation from the gyroscope sensor is different,
    since the gyroscope measures angular velocity (the rate of change in orientation angle), not angular orientation itself.
    To compute the orientation, we must first initialize the sensor position with a known value (possibly from the accelerometer),
    then measure the angular velocity (ω) around the X, Y and Z axes at measured intervals (Δt).   Then ω × Δt = change in angle.
    The new orientation angle will be the original angle plus this change.*/
  // set initial values equal to accel values
  if (i == 1) {
    GyX = Ax;
    GyY = Ay;
    GyZ = Az;
  }
  // integrate to find the gyro angle
  else {
    /*Gyroscopic value multiplied by the time between sensor readings, gives the change in angular position.
      If we save the previous angular position, we simply add the computed change each time to find the new value.*/
    GyX = GyX + (timeStep * Gx);
    GyY = GyY + (timeStep * Gy);
    GyZ = GyZ + (timeStep * Gz);
  }

  /*Filtered Angle = α × (Gyroscope Angle) + (1 − α) × (Accelerometer Angle)  where
    α = τ/(τ + Δt)   and   (Gyroscope Angle) = (Last Measured Filtered Angle) + ω×Δt
    Δt = sampling rate= 1s, τ = time constant greater than timescale of typical accelerometer noise*/
  Rx = (0.96 * Ax) + (0.04 * Gx);
  Ry = (0.96 * Ay) + (0.04 * Gy);
  Rz = (0.96 * Az) + (0.04 * Gz);

  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp / 340.00 + 36.53); //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
  Serial.print(" | Ax = "); Serial.print(Ax);
  Serial.print(" | Ay = "); Serial.print(Ay);
  Serial.print(" | Az = "); Serial.print(Az);
  Serial.print(" | Gx = "); Serial.print(Gx);
  Serial.print(" | Gy = "); Serial.print(Gy);
  Serial.print(" | Gz = "); Serial.println(Gz);
  Serial.print(" | Rx = "); Serial.print(Rx);
  Serial.print(" | Ry = "); Serial.print(Ry);
  Serial.print(" | Rz = "); Serial.print(Rz);

  i++;
  delay(333); 
}
