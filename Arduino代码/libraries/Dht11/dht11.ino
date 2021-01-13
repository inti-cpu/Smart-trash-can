/*
  优创智能电子
  本实验使用有源蜂鸣器模块

*/

double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32;
}    //摄氏温度度转化为华氏温度

double Kelvin(double celsius)
{
  return celsius + 273.15;
}     //摄氏温度转化为开氏温度

// 露点（点在此温度时，空气饱和并产生露珠）
// 参考: http://wahiduddin.net/calc/density_algorithms.htm
double dewPoint(double celsius, double humidity)
{
  double A0 = 373.15 / (273.15 + celsius);
  double SUM = -7.90298 * (A0 - 1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / A0))) - 1) ;
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (A0 - 1))) - 1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
}

// 快速计算露点，速度是5倍dewPoint()
// 参考: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity / 100);
  double Td = (b * temp) / (a - temp);
  return Td;
}

#include "dht11.h"

dht11 DHT11;

#define DHT11PIN 2

int pin_buzzer = 7; //定义蜂鸣器引脚

int pin_led = 8; //定义led引脚

void setup()
{
  Serial.begin(9600);
  Serial.println("DHT11 TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_led, OUTPUT);
}

void loop()
{
  Serial.println("\n");

  int chk = DHT11.read(DHT11PIN);

  Serial.print("Read sensor: ");

  Serial.print("Humidity (%): ");
  Serial.println((float)DHT11.humidity, 2);

  //温度大于20，则蜂鸣器鸣叫，led关闭。否则相反
  if (DHT11.humidity > 20) {
    digitalWrite(pin_buzzer, HIGH);
    digitalWrite(pin_led, LOW);
  } else {
    digitalWrite(pin_buzzer, LOW);
    digitalWrite(pin_led, HIGH);
  }


  Serial.print("Temperature (oC): ");
  Serial.println((float)DHT11.temperature, 2);

  Serial.print("Temperature (oF): ");
  Serial.println(Fahrenheit(DHT11.temperature), 2);

  Serial.print("Temperature (K): ");
  Serial.println(Kelvin(DHT11.temperature), 2);

  Serial.print("Dew Point (oC): ");
  Serial.println(dewPoint(DHT11.temperature, DHT11.humidity));

  Serial.print("Dew PointFast (oC): ");
  Serial.println(dewPointFast(DHT11.temperature, DHT11.humidity));

  delay(2000);
}
