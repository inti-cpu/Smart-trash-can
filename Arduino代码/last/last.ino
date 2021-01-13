#include <Servo.h>
#include <ArduinoJson.h>//wifi库
#include <dht11.h>
#include <DHT11.h>
#include <U8g2lib.h>
#include <Wire.h>
#define DHT11PIN A1
dht11 DHT11;
String DEVICEID = "19489"; // 你的设备编号
String APIKEY = "7584023f8"; // 设备密码
String INPUTID1 = "17474";//温度接口
String INPUTID2 = "17487"; //湿度接口
String INPUTID3 = "17549";//CO接口
String INPUTID4 = "17550";//PM2.5接口
const int ledPin = 7;//报警灯
const int ledPin2 = 13;//夜灯
float valTemperature;//定义温度变量
float valHumidity;//定义湿度变量
//iic驱动方式/
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
//DHT dht(DHT11PIN, DHTTYPE);
unsigned long lastCheckStatusTime = 0; //记录上次状态查询时间
unsigned long lastUpdateTime = 0; //记录上次状态查询时间
const unsigned long statusInterval = 100; // 每隔2000秒检测一次在线状态
const unsigned long updateInterval = 5000; // 每隔6秒上传一次光照强度
//WiFi
int dustPin = A2;//电容的模拟端口
float dustVal = 0;//污染值
int ledPower = 4;//pm2.5模块数字值
int delayTime = 280;
int delayTime2 = 40;
float offTime = 9680;
float pos = 0;//舵机初始值
const int trig = 10;//超声波引脚
const int echo = 11;//超声波引脚
float distance; //测试的距离值
unsigned int sensorValue = 0; //一氧化碳初始值
#define Sensor_AO A0//一氧化碳引脚
#define Sensor_DO 2//一氧化碳引脚
//char h_str[3];//oled 湿度字符数组
//char t_str[3];//oled  温度字符数组
//float h;//湿度
//float t;//温度
int buzzer = 8; //设置控制蜂鸣器的数字IO脚
Servo myservo;
void setup() {
  myservo.attach(9);
  Serial.begin(115200);
  //  u8g2.begin();
  pinMode(ledPower, OUTPUT);//pm2.5数字端口
  pinMode(dustPin, INPUT);//pm2.5模拟端口
  pinMode(Sensor_DO, INPUT);//一氧化碳输入值端口
  pinMode(trig, OUTPUT);//超声波输出端口
  pinMode(echo, INPUT);//超声波模拟端口
  pinMode(buzzer, OUTPUT); //设置数字IO脚模式，OUTPUT为输出
  pinMode(LED_BUILTIN, OUTPUT);//LED输出端口
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(12, OUTPUT);
  int chk = DHT11.read(DHT11PIN);
  delay(3000);
}
void loop() {
  Wifi();
  Dis();
  Openb();
  Co();
  //  Oled();
  Pm();
}
//距离检测
void Dis()
{
  digitalWrite(trig, 0);
  delayMicroseconds(2);
  digitalWrite(trig, 1);
  delayMicroseconds(10);
  digitalWrite(trig, 0);
  distance = pulseIn(echo, HIGH) / 58.00;
  Serial.print(distance);
  Serial.print("cm");
  Serial.println();
  //delay(35);
}
//超声波距离<=70cm时候打开垃圾桶
void Openb() {
  if (distance <= 70) {
    for (pos = 0; pos <= 110; pos += 0.25) // 从0度-110度
    { // 步进角度1度
      myservo.write(pos);// 输入对应的角度值，舵机会转到此位置
      //delay(2);  // 15ms后进入下一个位置
    }
    delay(4000);
    for (pos = 110; pos >= 0; pos -= 0.25) // 从110度-0度
    {
      myservo.write(pos);// 输入对应的角度值，舵机会转到此位置
      //delay(5);// 15ms后进入下一个位置
    }
  }
  else
  {
    pos = 0;
  }
}
//一氧化碳监测
void Co() {
  sensorValue = analogRead(Sensor_AO);
  Serial.print("Sensor AD Value = ");
  Serial.println(sensorValue);
  if (sensorValue > 200 )
  {
    Serial.println("Alarm!");
    Buf();
    Bao();
  }
  //  delay(100);
}
//PM2.5
void Pm() {
  digitalWrite(ledPower, LOW);
  delayMicroseconds(delayTime);
  dustVal = analogRead(dustPin);
  delayMicroseconds(delayTime2);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(offTime);
  delay(100);
  if (dustVal > 36.455) {
    float DustVaul = (float(dustVal / 1024) - 0.0356) * 120000 * 0.035;
    Serial.println(DustVaul);
    if (DustVaul >= 0 && DustVaul <= 150) {
      Serial.println("Excellent");
    }
    else if (DustVaul > 150 && DustVaul <= 300) {
      Serial.println("Good");
    }
    else if (DustVaul > 300 && DustVaul <= 1050) {
      Serial.println("Slight dust");
    }
    else {
      Serial.println("Severe");
      Buf();
      Bao();
    }
  }
}
//蜂鸣器
void Buf() {
  unsigned char i, j; //定义发量
  for (i = 0; i < 80; i++) //输出一个频率的声音
  {
    digitalWrite(buzzer, HIGH); //发声音
    delay(1);//延时1ms
    digitalWrite(buzzer, LOW); //不发声音
    delay(1);//延时ms
  }
  for (i = 0; i < 100; i++) //输出另一个频率的声音
  {
    digitalWrite(buzzer, HIGH); //发声音
    delay(2);//延时2ms
    digitalWrite(buzzer, LOW); //不发声音
    delay(2);//延时2ms
  }
}
//OLED显示屏
/*void Oled()
  //这个函数未实现字符串从左向右移动，而是出现了字符串叠加的现象。
  //原来是字符串太长了
  {
  h = valHumidity;//读湿度
  t = valTemperature;//读温度(摄氏度)
  strcpy(h_str, u8x8_u8toa(h, 2));   
strcpy(t_str, u8x8_u8toa(t, 2));    
u8g2.firstPage();
  do {
  u8g2.setFont(u8g2_font_fur20_tf);
  u8g2.drawStr(0, 23, "T");//y,x
  u8g2.drawStr(20, 23, ":");
  u8g2.drawStr(40, 23, t_str);
  u8g2.drawStr(90, 23, "C");
  u8g2.drawStr(0, 63, "H");
  u8g2.drawStr(20, 63, ":");
  u8g2.drawStr(40, 63, h_str);
  u8g2.drawStr(90, 63, "%");
  } while ( u8g2.nextPage() );
  }*/
//WIFI
void Wifi() {
  delay(5);
  //定时检查在线状态，兼做心跳
  if (millis() - lastCheckStatusTime > statusInterval) {
    checkStatus();
  }
  valTemperature = (float)DHT11.temperature;
  valHumidity = (float)DHT11.humidity;
  int sensorValue0 = analogRead(DHT11PIN);
  sensorValue = analogRead(Sensor_AO);
  dustVal = analogRead(dustPin);
  sensorValue = map(sensorValue, 0, 1023, 0, 100);
  digitalWrite(ledPower, LOW);
  delayMicroseconds(delayTime);
  dustVal = analogRead(dustPin);
  delayMicroseconds(delayTime2);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(offTime);
  float DustVaul = (float(dustVal / 1024) - 0.0356) * 120000 * 0.035;
  //定时上传光照数据
  if (millis() - lastUpdateTime > updateInterval) {
    //update1(DEVICEID, INPUTID3, sensorValue);
    update2(DEVICEID, INPUTID1, valTemperature, INPUTID2, valHumidity, INPUTID3, sensorValue, INPUTID4, DustVaul);//温湿度模块
    //update3(DEVICEID, INPUTID4, DustVaul);
    //update3(DEVICEID, INPUTID3, sensorValue, INPUTID4, DustVaull);
    lastUpdateTime = millis();
  }
  //监听esp01s透传过来的服务器指令
  if (Serial.available()) {
    String wifiInputString = Serial.readStringUntil('\n');
    processWifiMessage(wifiInputString);
  }
}
//贝壳物联设备登录指令
void checkIn() {
  Serial.print("{\"M\":\"checkin\",\"ID\":\"");
  Serial.print(DEVICEID);
  Serial.print("\",\"K\":\"");
  Serial.print(APIKEY);
  Serial.print("\"}\n");
}
//贝壳物联设备登出指令
void checkOut() {
  Serial.print("{\"M\":\"checkout\",\"ID\":\"");
  Serial.print(DEVICEID);
  Serial.print("\",\"K\":\"");
  Serial.print(APIKEY);
  Serial.print("\"}\n");
}
//贝壳物联查询设备连接状态指令
void checkStatus() {
  Serial.print("{\"M\":\"status\"}\n");
  lastCheckStatusTime = millis();
}
//处理接收到的指令
void processWifiMessage(String msg)
{
  //Serial.println(msg);
  int jsonBeginAt = msg.indexOf("{");
  if (jsonBeginAt == -1 )return;
  int jsonEndAt = msg.lastIndexOf("}");
  if (jsonEndAt == -1)return;
  msg = msg.substring(jsonBeginAt, jsonEndAt + 1);
  int len = msg.length() + 1;
  char jsonString[len];
  msg.toCharArray(jsonString, len);
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& msg_obj = jsonBuffer.parseObject(jsonString);
  if (!msg_obj.success()) return;
  String M = msg_obj["M"];
  if (M == "WELCOME TO BIGIOT") {
    checkOut();
    delay(100);
    checkIn();
  }
  if (M == "connected") {
    checkOut();
    delay(1000);
    checkIn();
  }
  if (M == "say" || M == "alert")
  {
    String C = msg_obj["C"];
    String F_C_ID = msg_obj["ID"];//可利用此ID判断命令来源
    if (C == "up") {
      for (pos = 0; pos <= 180; pos += 0.25) // 从0度-180度
      { // 步进角度1度
        myservo.write(pos);              // 输入对应的角度值，舵机会转到此位置
        //delay(2);  // 15ms后进入下一个位置
      }
      delay(5000);
      for (pos = 180; pos >= 0; pos -= 0.25) // 从180度-0度
      {
        myservo.write(pos);              // 输入对应的角度值，舵机会转到此位置
        //delay(5);                       // 15ms后进入下一个位置
      }
      sayToClient(F_C_ID, "up");
    }
    if (C == "down") {
      pos = 0;
      sayToClient(F_C_ID, "up");
    }
    if (C == "play") {
      digitalWrite(ledPin, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
      sayToClient(F_C_ID, "LED turn on!");
    }
    if (C == "stop") {
      digitalWrite(ledPin, LOW);
      digitalWrite(LED_BUILTIN, LOW);
      sayToClient(F_C_ID, "LED turn off!");
    }
  }
}
//贝壳物联say通讯指令
void sayToClient(String client_id, String content) {
  Serial.print("{\"M\":\"say\",\"ID\":\"");
  Serial.print(client_id);
  Serial.print("\",\"C\":\"");
  Serial.print(content);
  Serial.print("\"}\n");
}
//贝壳物联上传实时数据指令
//void update1(String did, String inputid, float value) {
//  Serial.print("{\"M\":\"update\",\"ID\":\"");
//  Serial.print(did);
//  Serial.print("\",\"V\":{\"");
//  Serial.print(inputid);
//  Serial.print("\":\"");
//  Serial.print(value);
//  Serial.println("\"}}");
//}
//同时上传两个接口数据调用此函数
void update2(String did, String inputid1, float value1, String inputid2, float value2, String inputid3, int value3, String inputid4, int value4) {
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid1);
  Serial.print("\":\"");
  Serial.print(value1);
  Serial.print("\",\"");
  Serial.print(inputid2);
  Serial.print("\":\"");
  Serial.print(value2);
  Serial.print("\",\"");
  Serial.print(inputid3);
  Serial.print("\":\"");
  Serial.print(value3);
  Serial.print("\",\"");
  Serial.print(inputid4);
  Serial.print("\":\"");
  Serial.print(value4);
  Serial.println("\"}}");
}
void Bao(){
  digitalWrite(ledPin, HIGH);
  digitalWrite(12,LOW);  
  }
void CloseBao(){
  digitalWrite(ledPin,LOW);
  digitalWrite(12,HIGH);
  }
/*void update3(String did, String inputid1, float value1, String inputid2, float value2){
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid1);
  Serial.print("\":\"");
  Serial.print(value1);
  Serial.print("\",\"");
  Serial.print(inputid2);
  Serial.print("\":\"");
  Serial.print(value2);
  Serial.println("\"}}");
  }*/
/*void update3(String did, String inputid, float value) {
  Serial.print("{\"M\":\"update\",\"ID\":\"");
  Serial.print(did);
  Serial.print("\",\"V\":{\"");
  Serial.print(inputid);
  Serial.print("\":\"");
  Serial.print(value);
  Serial.println("\"}}");
  }*/
