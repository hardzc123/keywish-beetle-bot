#include "Beetlebot.h"
#include "ProtocolParser.h"
#include "debug.h"

Beetlebot::Beetlebot(ProtocolParser *Package, uint8_t input2, uint8_t input1, uint8_t input3, uint8_t input4): SmartCar("Beetlebot", 0x01, E_BLUETOOTH_CONTROL)
{
	this->InPut2PIN = input2;
	this->InPut1PIN = input1;
	this->InPut3PIN = input3;
	this->InPut4PIN = input4;
	SetStatus(E_STOP);
	mProtocolPackage = Package;
	Speed = 0;
}

Beetlebot::~Beetlebot()
{
    delete mIrRecv;
    delete mPs2x;
    delete mInfraredTracing;
    delete mInfraredAvoidance;
    delete mUltrasonic;
}

void Beetlebot::init(void)
{
    pinMode(InPut2PIN, OUTPUT);
    digitalWrite(InPut2PIN, LOW);
    pinMode(InPut1PIN, OUTPUT);
    digitalWrite(InPut1PIN, LOW);
    pinMode(InPut3PIN, OUTPUT);
    digitalWrite(InPut3PIN, LOW);
    pinMode(InPut4PIN, OUTPUT);
    digitalWrite(InPut4PIN, LOW);
}

void Beetlebot::GoForward(void)
{
    int value = (Speed / 10) * 25;
    DEBUG_LOG(DEBUG_LEVEL_INFO, "GoForward\n");
    analogWrite(InPut3PIN, LOW);
    analogWrite(InPut4PIN, value);
    analogWrite(InPut2PIN, LOW);
    analogWrite(InPut1PIN, value);
    SetStatus(E_FORWARD);
}

void Beetlebot::GoBack(void)
{
    int value = (Speed / 10) * 25;
    DEBUG_LOG(DEBUG_LEVEL_INFO, "GoBack\n");
    SetStatus(E_BACK);
    analogWrite(InPut2PIN, value);
    analogWrite(InPut1PIN, LOW);
    analogWrite(InPut3PIN, value);
    analogWrite(InPut4PIN, LOW);
}

void Beetlebot::KeepStop(void)
{
    DEBUG_LOG(DEBUG_LEVEL_INFO, "KeepStop\n");
    SetStatus(E_STOP);
    analogWrite(InPut2PIN, LOW);
    analogWrite(InPut1PIN, LOW);
    analogWrite(InPut3PIN, LOW);
    analogWrite(InPut4PIN, LOW);
}

void Beetlebot::TurnLeft()
{
    int value = (Speed/10)*25.5;   //app contol beetle_speed is 0 ~ 100 ,pwm is 0~255
    DEBUG_LOG(DEBUG_LEVEL_INFO, "TurnLeft \n");
    analogWrite(InPut2PIN, value);
    analogWrite(InPut1PIN, LOW);
    analogWrite(InPut3PIN, LOW);
    analogWrite(InPut4PIN, value);
    SetStatus(E_LEFT);
}

void Beetlebot::TurnRight()
{
    DEBUG_LOG(DEBUG_LEVEL_INFO, "TurnRight \n");
    int value = (Speed/10)*25.5;   //app contol beetle_speed is 0 ~ 100 ,pwm is 0~255
    analogWrite(InPut2PIN, LOW);
    analogWrite(InPut1PIN, value);
    analogWrite(InPut3PIN, value);
    analogWrite(InPut4PIN, LOW);
    SetStatus(E_RIGHT);

}
void Beetlebot::Drive(void)
{
    Drive(Degree);
}

void Beetlebot::Drive(int degree)
{
    arduino_printf("degree = %d speed = %d\n", degree, Speed);
    int value = (Speed / 10) * 25.5;	 //app contol beetle_speed is 0 ~ 100 ,pwm is 0~255
    float f;
    if (degree >= 0 && degree <= 90) {
		f = (float)(degree) / 90;
		analogWrite(InPut2PIN, LOW);
		analogWrite(InPut1PIN, value);
		analogWrite(InPut3PIN, LOW);
		analogWrite(InPut4PIN, (float)(value * f));
		DEBUG_LOG(DEBUG_LEVEL_INFO, "TurnRight\n");
		SetStatus(E_RIGHT);
	} else if (degree > 90 && degree <= 180) {
		f = (float)(180 - degree) / 90;
		analogWrite(InPut2PIN, LOW);
		analogWrite (InPut1PIN, (float)(value * f));
		analogWrite(InPut3PIN, LOW);
		analogWrite(InPut4PIN, value);
		SetStatus(E_LEFT);
	} else if (degree > 180 && degree <= 270) {
		f = (float)(degree - 180) / 90;
		analogWrite(InPut2PIN, (float)(value * f));
		analogWrite(InPut1PIN, LOW);
		analogWrite(InPut3PIN, value);
		analogWrite(InPut4PIN, LOW);
		DEBUG_LOG(DEBUG_LEVEL_INFO, "TurnLeft\n");
		SetStatus(E_LEFT);
	} else if (degree > 270 && degree <= 360) {
		f = (float)(360 - degree) / 90;
		analogWrite(InPut2PIN, value);
		analogWrite(InPut1PIN, LOW);
		analogWrite(InPut3PIN, (float)(value * f));
		analogWrite(InPut4PIN, LOW);
		DEBUG_LOG(DEBUG_LEVEL_INFO, "TurnRight\n");
		SetStatus(E_RIGHT);
	} else {
		KeepStop();
	}
}
#if ARDUINO > 10609
void Beetlebot::SetIrPin(uint8_t pin = BE_IR_PIN)
#else
void Beetlebot::SetIrPin(uint8_t pin)
#endif
{
	IrPin = pin;
	mIrRecv = new IRremote (IrPin);
	mIrRecv->begin();  // Initialize the infrared receiver
}

#if ARDUINO > 10609
void Beetlebot::SetInfraredTracingPin(uint8_t Pin1 = BE_INFRARED_TRACING_PIN1, uint8_t Pin2 = BE_INFRARED_TRACING_PIN2, uint8_t Pin3 = BE_INFRARED_TRACING_PIN3)
#else
void Beetlebot::SetInfraredTracingPin(uint8_t Pin1 , uint8_t Pin2 , uint8_t Pin3 )
#endif

{
    static bool InfraredTracingInit = false;
    if (!InfraredTracingInit) {
        InfraredTracingPin1 = Pin1;
        InfraredTracingPin2 = Pin2;
        InfraredTracingPin3 = Pin3;
        mInfraredTracing = new InfraredTracing(InfraredTracingPin1, InfraredTracingPin2, InfraredTracingPin3);
        //mInfraredTracing->begin();
        InfraredTracingInit = true;
    }
}

#if ARDUINO > 10609
int Beetlebot::SetPs2xPin(uint8_t clk = BE_PS2X_CLK, uint8_t cmd = BE_PS2X_CMD, uint8_t att = BE_PS2X_ATT, uint8_t dat = BE_PS2X_DAT)
#else
int Beetlebot::SetPs2xPin(uint8_t clk , uint8_t cmd , uint8_t att, uint8_t dat )
#endif
{
    static bool Ps2xInit = false;
    int error = 0 ;
    if (!Ps2xInit) {
        DEBUG_LOG(DEBUG_LEVEL_INFO, "SetPs2xPin\n");
        Ps2xClkPin = clk;
        Ps2xCmdPin = cmd;
        Ps2xAttPin = att;
        Ps2xDatPin = dat;
        mPs2x = new PS2X();
        //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
        //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
        error = mPs2x->config_gamepad(Ps2xClkPin, Ps2xCmdPin, Ps2xAttPin, Ps2xDatPin, false, false);
        if (error == 1) {
            DEBUG_LOG(DEBUG_LEVEL_ERR, "No controller found, check wiring\n");
        } else if (error == 2) {
            DEBUG_LOG(DEBUG_LEVEL_ERR, "Controller found but not accepting commands\n");
        } else if (error == 3) {
            DEBUG_LOG(DEBUG_LEVEL_ERR, "Controller refusing to enter Pressures mode, may not support it\n");
        } else if (error == 0) {
            DEBUG_LOG(DEBUG_LEVEL_INFO, "Found Controller, configured successful\n");
        }
        Ps2xInit = true;
    }
    return error;
}
int Beetlebot::ResetPs2xPin(void)
{
    int error = mPs2x->config_gamepad(Ps2xClkPin, Ps2xCmdPin, Ps2xAttPin, Ps2xDatPin, false, false);
    if (error == 1) {
      DEBUG_LOG(DEBUG_LEVEL_ERR, "No controller found, check wiring\n");
  	} else if (error == 2) {
  		DEBUG_LOG(DEBUG_LEVEL_ERR, "Controller found but not accepting commands\n");
  	} else if (error == 3) {
  		DEBUG_LOG(DEBUG_LEVEL_ERR, "Controller refusing to enter Pressures mode, may not support it\n");
  	} else if (error == 0) {
  		DEBUG_LOG(DEBUG_LEVEL_INFO, "Found Controller, configured successful\n");
  	}
  	return error;
}

#if ARDUINO > 10609
void Beetlebot::SetUltrasonicPin(uint8_t Trig_Pin = BE_TRIGPIN, uint8_t Echo_Pin = BE_ECHOPIN, uint8_t Sevo_Pin = BE_SERVOPIN)
#else
void Beetlebot::SetUltrasonicPin(uint8_t Trig_Pin , uint8_t Echo_Pin , uint8_t Sevo_Pin )
#endif

{
    static bool UltrasonicInit = false;
    if (!UltrasonicInit) {
        EchoPin = Echo_Pin;
        TrigPin = Trig_Pin;
        ServoPin = Sevo_Pin;
        mUltrasonic = new Ultrasonic(TrigPin, EchoPin, ServoPin);
        UltrasonicInit = true;
    }
}

#if ARDUINO > 10609
void Beetlebot::SetInfraredAvoidancePin(uint8_t Left_Pin = BE_INFRARED_AVOIDANCE_LEFT_PIN, uint8_t Right_Pin = BE_INFRARED_AVOIDANCE_RIGHT_PIN)
#else
void Beetlebot::SetInfraredAvoidancePin(uint8_t Left_Pin , uint8_t Right_Pin )
#endif
{
	static bool InfraredAvoidanceInit = false;
	if (!InfraredAvoidanceInit) {
		InfraredAvoidancePin1 = Right_Pin;
		InfraredAvoidancePin2 = Left_Pin;
		mInfraredAvoidance = new InfraredAvoidance(InfraredAvoidancePin1, InfraredAvoidancePin2);
		InfraredAvoidanceInit = true;
	}
}
void Beetlebot::SendTracingSignal(void)
{
    unsigned int TracingSignal = mInfraredTracing->getValue();
    SendData.start_code = 0xAA;
    SendData.type = (E_TYPE)0x01;
    SendData.addr = 0x01;
    SendData.function = E_INFRARED_TRACKING;
    SendData.data = (byte *)&TracingSignal;
    SendData.len = 7;
    SendData.end_code = 0x55;
    mProtocolPackage->SendPackage(&SendData, 1);
}

void Beetlebot::SendInfraredData(void)
{
    unsigned int RightValue = mInfraredAvoidance->GetInfraredAvoidanceRightValue();
    unsigned int LeftValue = mInfraredAvoidance->GetInfraredAvoidanceLeftValue();
    byte buffer[2];
    SendData.start_code = 0xAA;
    SendData.type = (E_TYPE)0x01;
    SendData.addr = 0x01;
    SendData.function = E_INFRARED_AVOIDANCE_MODE;
    buffer[0] = LeftValue & 0xFF;
    buffer[1] = RightValue & 0xFF;
    SendData.data = buffer;
    SendData.len = 8;
    SendData.end_code = 0x55;
    mProtocolPackage->SendPackage(&SendData, 2);
}

void Beetlebot::SendUltrasonicData(void)
{
    unsigned int UlFrontDistance =  mUltrasonic->GetUltrasonicFrontDistance();
    SendData.start_code = 0xAA;
    SendData.type = (E_TYPE)0x01;
    SendData.addr = 0x01;
    SendData.function = E_ULTRASONIC_AVOIDANCE;
    SendData.data = (byte *)&UlFrontDistance;
    SendData.len = 7;
    SendData.end_code = 0x55;
    mProtocolPackage->SendPackage(&SendData, 1);
}
