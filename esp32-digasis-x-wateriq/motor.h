// Pin kontrol motor
#define IN1 25
#define IN2 33

void motor_setup() {
  // Atur pin sebagai output
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN2, LOW); // Arah tetap
  Serial.begin(115200);
  Serial.println("Kontrol Motor L298N Siap");
}

void pompa(int pwm){
     analogWrite(IN1, pwm);
  }
