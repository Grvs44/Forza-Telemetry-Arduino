// Adapted from https://support.forzamotorsport.net/hc/en-us/articles/21742934024211-Forza-Motorsport-Data-Out-Documentation
#include <inttypes.h>

typedef struct {
  // = 1 when race is on. = 0 when in menus/race stopped …
  int32_t IsRaceOn;

  // Can overflow to 0 eventually
  uint32_t TimestampMS;

  float EngineMaxRpm;
  float EngineIdleRpm;
  float CurrentEngineRpm;

  // In the car's local space; X = right, Y = up, Z = forward
  float AccelerationX;
  float AccelerationY;
  float AccelerationZ;
  float VelocityX;
  float VelocityY;
  float VelocityZ;

  // In the car's local space; X = pitch, Y = yaw, Z = roll
  float AngularVelocityX;
  float AngularVelocityY;
  float AngularVelocityZ;

  float Yaw;
  float Pitch;
  float Roll;

  // Suspension travel normalized: 0.0f = max stretch; 1.0 = max compression
  float NormalizedSuspensionTravelFrontLeft;
  float NormalizedSuspensionTravelFrontRight;
  float NormalizedSuspensionTravelRearLeft;
  float NormalizedSuspensionTravelRearRight;

  // Tire normalized slip ratio, = 0 means 100% grip and |ratio| > 1.0 means loss of grip.
  float TireSlipRatioFrontLeft;
  float TireSlipRatioFrontRight;
  float TireSlipRatioRearLeft;
  float TireSlipRatioRearRight;

  // Wheels rotation speed radians/sec.
  float WheelRotationSpeedFrontLeft;
  float WheelRotationSpeedFrontRight;
  float WheelRotationSpeedRearLeft;
  float WheelRotationSpeedRearRight;

  // = 1 when wheel is on rumble strip, = 0 when off.
  int32_t WheelOnRumbleStripFrontLeft;
  int32_t WheelOnRumbleStripFrontRight;
  int32_t WheelOnRumbleStripRearLeft;
  int32_t heelOnRumbleStripRearRight;

  // = from 0 to 1, where 1 is the deepest puddle
  float WheelInPuddleDepthFrontLeft;
  float WheelInPuddleDepthFrontRight;
  float WheelInPuddleDepthRearLeft;
  float WheelInPuddleDepthRearRight;

  // Non-dimensional surface rumble values passed to controller force feedback
  float SurfaceRumbleFrontLeft;
  float SurfaceRumbleFrontRight;
  float SurfaceRumbleRearLeft;
  float SurfaceRumbleRearRight;

  // Tire normalized slip angle, = 0 means 100% grip and |angle| > 1.0 means loss of grip.
  float TireSlipAngleFrontLeft;
  float TireSlipAngleFrontRight;
  float TireSlipAngleRearLeft;
  float TireSlipAngleRearRight;

  // Tire normalized combined slip, = 0 means 100% grip and |slip| > 1.0 means loss of grip.
  float TireCombinedSlipFrontLeft;
  float TireCombinedSlipFrontRight;
  float TireCombinedSlipRearLeft;
  float TireCombinedSlipRearRight;

  // Actual suspension travel in meters
  float SuspensionTravelMetersFrontLeft;
  float SuspensionTravelMetersFrontRight;
  float SuspensionTravelMetersRearLeft;
  float SuspensionTravelMetersRearRight;

  // Unique ID of the car make/model
  int32_t CarOrdinal;

  // Between 0 (D -- worst cars) and 7 (X class -- best cars) inclusive
  int32_t CarClass;

  // Between 100 (worst car) and 999 (best car) inclusive
  int32_t CarPerformanceIndex;

  // 0 = FWD, 1 = RWD, 2 = AWD
  int32_t DrivetrainType;

  // Number of cylinders in the engine
  int32_t NumCylinders;
} Sled;

typedef struct : Sled {
  float PositionX;
  float PositionY;
  float PositionZ;
  float Speed;
  float Power;
  float Torque;
  float TireTempFrontLeft;
  float TireTempFrontRight;
  float TireTempRearLeft;
  float TireTempRearRight;
  float Boost;
  float Fuel;
  float DistanceTraveled;
  float BestLap;
  float LastLap;
  float CurrentLap;
  float CurrentRaceTime;
  uint16_t LapNumber;
  uint8_t RacePosition;
  uint8_t Accel;
  uint8_t Brake;
  uint8_t Clutch;
  uint8_t HandBrake;
  uint8_t Gear;
  int8_t Steer;
  int8_t NormalizedDrivingLine;
  int8_t NormalizedAIBrakeDifference;

  float TireWearFrontLeft;
  float TireWearFrontRight;
  float TireWearRearLeft;
  float TireWearRearRight;

  // ID for track
  int32_t TrackOrdinal;
} Dash;
