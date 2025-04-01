#define BLINKER_PRINT Serial
#define BLINKER_WIFI

#include <Arduino.h>
#include <Blinker.h>
#include <Servo.h>

  //定义引脚
#define Pin 14
#define Pin_ADC A0
  //定义舵机操作变量
Servo myServo;
  //定义定时器变量
unsigned long previousMillis = 0;
const long interval = 5 * 60 * 1000;  // 10分钟
float voltage = 0.0;
  //初始化网络以及上位机识别码
char auth[] = "************";
char ssid[] = "************";
char pswd[] = "************";
  //初始化UI界面
BlinkerText stated_1("text1");// wifi状态
BlinkerText stated_2("text2");// 自检状态
BlinkerNumber BatteryLevel("battery-level");// 数字显示
BlinkerButton Button1("open");// 按钮1
BlinkerButton Button2("close");// 按钮2
BlinkerButton Button3("PTK");// 按钮3状态回传（用于自检）

// void dataRead(const String & data)
// {
//     // 串口打印手机APP端发过来的数据
//     BLINKER_LOG("Blinker readString: ", data);
//     // 获取当前时间（以毫秒为单位）
//     uint32_t BlinkerTime = millis();
    
//     // 发送文本数据到Blinker服务器
//     Blinker.textData("text data");
// }

void sendBatteryLevelToBlinker(float batteryVoltage) {
  // 将电压转换为电量百分比（4.2V=100%，3.2V=0%）
  float percentage = (batteryVoltage - 3.2) / (4.2 - 3.2) * 100;
  percentage = constrain(percentage, 0, 100); // 限制在0-100%范围内
  BatteryLevel.print(round(percentage));      // 四舍五入为整数百分比
}

float readBatteryVoltage() {
float adcValue = analogRead(Pin_ADC);
voltage = (adcValue / 1023.0) * 3.3 / 0.816;
 return voltage;
}
// 数据回传
void button3_callback(const String & state) {
  if(WiFi.status() == WL_CONNECTED) stated_1.print("现已连接到网络");
   else if(WiFi.status() != WL_CONNECTED) stated_1.print("正在连接网络...");
   sendBatteryLevelToBlinker(voltage);
}

void Time_BatteryLevel()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    voltage = readBatteryVoltage();
    sendBatteryLevelToBlinker(voltage);
    previousMillis = currentMillis;  // 重置计时器
  }
}
/**
 * @brief 按钮1按下时的回调函数
 * 
 * @param state 按钮的状态
 */
void button1_callback(const String & state) {
    // 旋转舵机到120度
    myServo.write(120);
    // 延迟1.5秒
    delay(1500);
    // 旋转舵机到90度
    myServo.write(90);
    // 取消睡眠定时器
    WiFi.setSleepMode(WIFI_MODEM_SLEEP); // 设置为Modem-Sleep模式
}
  // 按钮按下执行操作
/**
 * @brief 按钮2按下时的回调函数
 * 
 * @param state 按钮的状态
 */
void button2_callback(const String & state){
    //旋转角度
    myServo.write(30);
    //延迟1.5秒
    delay(1500);
    //旋转角度
    myServo.write(90);
    // 取消睡眠定时器
    WiFi.setSleepMode(WIFI_MODEM_SLEEP); // 设置为Modem-Sleep模式
}

void Serve_detect(){ 
 // 开机自检 检测是否可以正常工作
 // 延迟500毫秒
 delay(500);
 // 旋转舵机到120度（开灯检测）
 myServo.write(120); 
 // 延迟1.5秒
 delay(1500);
 // 旋转舵机到90度 (舵机归位)
 myServo.write(90);
 // 延迟2秒
 delay(2000);
 // 旋转舵机到30度（关灯结束检测）
 myServo.write(30); 
 // 延迟1.5秒
 delay(1500);
 // 旋转舵机到90度 (舵机归位)
 myServo.write(90);
 // 更新UI界面显示自检完成
 stated_2.print("自检完成");
 // 设置标志位，避免重复执行自检
}

void setup() {
    // 将舵机连接到指定引脚
    myServo.attach(Pin);
    // 初始化串口通信，设置波特率为115200
    Serial.begin(115200);
    // 设置Blinker库的调试输出流为串口
    BLINKER_DEBUG.stream(Serial);
    // 初始化Blinker库，连接到指定的WiFi网络
    Blinker.begin(auth, ssid, pswd);

    // 注册数据读取回调函数
    // Blinker.attachData(dataRead);

    // 注册按钮1的回调函数
    Button1.attach(button1_callback);
    // 注册按钮2的回调函数
    Button2.attach(button2_callback);
    // 注册按钮3的回调函数
    Button3.attach(button3_callback);
}
int i=0,k=0;
void loop() {
    // 运行Blinker库，处理事件
    Blinker.run();
    // 读取舵机的当前角度
    myServo.read();
    // 检查WiFi连接状态
    if(WiFi.status() == WL_CONNECTED && i == 0) {
        // 如果已连接，更新UI界面显示已连接到网络
        stated_1.print("已连接到网络"); 
        // 设置标志位，避免重复打印
        i = 1;
    } else if(WiFi.status() != WL_CONNECTED) {
        // 如果未连接，更新UI界面显示正在连接网络
        stated_1.print("正在连接网络..."); 
        // 重置标志位，以便重新打印连接状态
        i = 0;
    }
    // 开机自检
    if(WiFi.status() == WL_CONNECTED && k == 0){
        Serve_detect();
        k = 1;
      // 检查按钮是否在10秒内被按下
      static uint32_t lastButtonPressTime = 0;
      if (millis() - lastButtonPressTime > 20000) { // 10秒
          // 按钮在10秒内未被按下，进入睡眠模式
        WiFi.disconnect();
        WiFi.setSleepMode(WIFI_MODEM_SLEEP); // 设置为Modem-Sleep模式
      }
    }
    Time_BatteryLevel();
}
