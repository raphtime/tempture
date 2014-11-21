/*
维天监制|Sharp DN7C3A006 灰尘PM2.5传感器测试代码
http://weesky.taobao.com
版本:V1.0
发布：2014年11月2日 
作者：Weesky
修改者：（修改者请注明..）

所需元件：

DN7C3A006夏普PM2.5传感器*1
外围电路扩展板*1
DS18B20温度传感器 *1
10千欧上拉电阻*1

*/

#include <Wire.h> 
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 10 // 定义DS18B20数据口连接arduino的10号IO上

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);// 初始连接在单总线上的单总线设备
//DallasTemperature sensors;// 初始连接在单总线上的单总线设备

#define measurePin  0  // A0口读取传感器采样电压。
#define ledPower  5    // pin5口，用来给PWM控制采样。

#define samplingTime 280 // 这是采样脉冲周期，我们用了NPN三极管，所以高电平开启LED
#define deltaTime 40 
#define sleepTime 9680

// DallasTemperature DS18B20..one wire  DS18B20传感器做温度修正。
long startTime;
int tempSeconds=20000;
float air_Temp;

/*SHARP Sensor_DN7C3 QR code information:
 ## # ## ##### #.## #.## ##.#
 请注意夏普传感器整流罩上的"二维码"，扫描可得到QR信息。
 其中包括，无尘电压和标定时的温度。这两个参数为后续计算所用。
 某些情况下，这个无尘电压可能不太准，如果计算结果出现负值，可以考虑自己修正无尘电压。
 这里我们扫码得到的电压是1.050v、25.9摄氏度。

 Production year:
 Production month:
 Production day:
 Serial number:
 6|Measurements in no dust: 1.050v
 7|Measurements in dust (Note):
 8|Temperature:25.9℃ */

float Ao,sum_Ao,average_Ao,Vo,Vs,dlt_Mv,pm2d5;
float baseTemp=25.9;
float noDust=1280; //1050;  //Measurements in no dust! "mv"

void setup() {
  Serial.begin(9600);
  pinMode(ledPower,OUTPUT);
  sensors.begin();
  //sensors.begin(10);
  sensors.requestTemperatures(); // 发送命令获取温度
  air_Temp=sensors.getTempCByIndex(0);
  startTime=millis();
  Serial.println("  PM2.5_Sensor SHARP DN7C3A006 Demo. [Weesky.taobao.com]");
}

void loop()
{
  for (int i=0;i<100;i++){
    digitalWrite(ledPower,HIGH);  
    delayMicroseconds(280); 
    Ao= analogRead(measurePin); 
    delayMicroseconds(40); 
    digitalWrite(ledPower,LOW); 
    delayMicroseconds(9680);   
   //请注意！！ 采样周期里不要做别的事情，1秒钟采集100个样本。
    sum_Ao=Ao+sum_Ao;  
    /* 
     Serial.print(" ");
     Serial.print(Ao);
     if((count+1) %10 ==0){ Serial.println("");}
     */
  }

  average_Ao=sum_Ao/100.0;
  get_pm2d5(average_Ao);

//  Serial.print(" | Ao=");
//  Serial.print(average_Ao);
  Serial.print(" | Vo=");
  Serial.print(Vo);
  Serial.print(" | Vs=");
  Serial.print(Vs);
//  Serial.print(" | dlt_Mv=");
//  Serial.print(dlt_Mv);
  Serial.print(" | air_Temp=");
  Serial.print(air_Temp);
  Serial.print(" | *** pm2d5 = ");
  Serial.print(pm2d5);
  Serial.println(" ug/m3");
  
  sum_Ao=0;
  average_Ao=0;

}


float get_pm2d5(float x){

  Vo=(x/1024)*5000.0; //计算PM2.5具体可参考PDF
  dlt_Mv=Vo-get_Vs();
  pm2d5=0.6*dlt_Mv;  //此处0.6为经验K值，可根据实际测算，0.7或许也不错。
  return pm2d5;

}


float get_temp(){

  if (millis()-startTime>tempSeconds)
  {
    sensors.requestTemperatures(); // 发送命令获取温度
    air_Temp=sensors.getTempCByIndex(0);
    startTime=millis();
  }
  return air_Temp;

}

float get_Vs()  //此处是修正基准电压Vs漂移的。
{
  get_temp();

  if(air_Temp < 40 && air_Temp > -10)
  {
    float dlt_Temp= baseTemp-air_Temp;
    Vs=noDust-6*dlt_Temp;
  }
  else if( air_Temp>=40 )
  {
    float dlt_Temp= baseTemp-40;
    Vs=noDust-((dlt_Temp)*6+(air_Temp-40)*1.5);
  }

  return Vs;

}


