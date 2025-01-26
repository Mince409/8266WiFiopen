#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#include <Arduino.h>
#include <Blinker.h>
#include <Servo.h>
  //定义引脚
#define Pin 14
  //定义舵机操作变量
Servo myServo;
  //初始化网络以及上位机识别码
char auth[] = "93351b558299";
char ssid[] = "CMCC-中国移动";
char pswd[] = "123456789";
  //初始化UI界面
BlinkerText stated("text");
BlinkerText stated_1("text1");
BlinkerText stated_2("text2");
BlinkerButton Button1("open");
BlinkerButton Button2("close");
BlinkerButton Button3("PTK");

void dataRead(const String & data)
{
    // 串口打印手机APP端发过来的数据
    BLINKER_LOG("Blinker readString: ", data);
    // 获取当前时间（以毫秒为单位）
    uint32_t BlinkerTime = millis();
    
    // 发送文本数据到Blinker服务器
    Blinker.textData("text data");
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
    // 打印“灯已开”到UI界面
    stated.print("灯已开");
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
    //打印“灯已关”到UI界面
    stated.print("灯已关");
    // 取消睡眠定时器
    WiFi.setSleepMode(WIFI_MODEM_SLEEP); // 设置为Modem-Sleep模式
}

void button3_callback(const String & state) {
    if(WiFi.status() == WL_CONNECTED) stated_1.print("现已连接到网络");
     else if(WiFi.status() != WL_CONNECTED) stated_1.print("正在连接网络...");
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
    Blinker.attachData(dataRead);
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
        k = 1;

      // 检查按钮是否在10秒内被按下
      static uint32_t lastButtonPressTime = 0;
      if (millis() - lastButtonPressTime > 20000) { // 10秒
          // 按钮在10秒内未被按下，进入睡眠模式
        WiFi.disconnect();
        WiFi.setSleepMode(WIFI_MODEM_SLEEP); // 设置为Modem-Sleep模式
      }

    }
}
