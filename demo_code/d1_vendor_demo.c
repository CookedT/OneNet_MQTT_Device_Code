#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Ticker.h>
Ticker tickerflash;
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(;
i<sizeof(p); i++) EEPROM.write(address+i,pp);}
#define EEPROM_read(address, p) {int i = 0; byte *pp =
(byte*)&(p);for(; i<sizeof(p); i++) pp=EEPROM.read(address+i);}
/* Set these to your desired credentials. */
static char Apid[9] = "NETGEAR";//根据你的路由器设置
static char softAPID[] = "KYSMART";
static char ApPass[10] = "zjky61448";//根据你的路由器设置
byte APip[] = { 192, 168, 20, 221 };//根据你的路由器设置
byte APGateWay[] = { 192, 168, 20, 254 };//根据你的路由器设置
byte APSubNet[] = { 255, 255, 255, 0 };
unsigned char openc[5][8] = {
{ 0x01, 0x06, 0x00, 0x01, 0x01, 0x01, 0x18, 0x5a},//1 号继电器开启
{ 0x01, 0x06, 0x00, 0x01, 0x02, 0x01, 0x18, 0xaa},//2 号继电器开启
{ 0x01, 0x06, 0x00, 0x01, 0x03, 0x01, 0x19, 0x3a},//3 号继电器开启
{ 0x01, 0x06, 0x00, 0x01, 0x04, 0x01, 0x1b, 0x0a},//4 号继电器开启
{ 0x01, 0x06, 0x00, 0x01, 0xff, 0xff, 0xd9, 0xba}//全亮
};
unsigned char closec[5][8] = {
{ 0x01, 0x06, 0x00, 0x01, 0x01, 0x00, 0xd9, 0x9a},//1 号继电器关闭
{ 0x01, 0x06, 0x00, 0x01, 0x02, 0x00, 0xd9, 0x6a},//2 号继电器关闭
{ 0x01, 0x06, 0x00, 0x01, 0x03, 0x00, 0xd8, 0xfa},//3 号继电器关闭
{ 0x01, 0x06, 0x00, 0x01, 0x04, 0x00, 0xda, 0xca},//4 号继电器关闭
{ 0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0xd8, 0x0a}//全灭
};
byte TSwitch[] = {
0, 0, 0, 0
};
byte StatSave[] = {
0xff, 0xff, 0xff, 0xff
};//掉电保护
byte Switchnum = 10;//13
const byte SwitchIO[] = {
D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D14, D15
};//开关列表
byte flashLed = D13; // 呼吸灯
int ledState = LOW;
char funcstr[800];
byte aptype = 0;// 模式：1 AP 0 CLIENT
const char pageS[] PROGMEM = "<meta name=\"viewport\"
content=\"width=device-width,initial-scale=1.0\">\r\n<meta
http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\r\n"
"<style
type=\"text/css\">"
"*
{margin:3;padding:3;}"
"input{width:90%;hei
ght:40px;font-size:20px;background:#999;}"
"</style>"
"<center><h2>欢迎使
用科友智能家居<br></h2><h3>"
"%s"
"<br><a
href=\"http://www.it15168.com\">ZJKEYOU SMART HOME</a></center></h3>";
const char LineS[] PROGMEM = "%s<br><br><a href=\"/\">返回</a><br>";
// Create an instance of the server
ESP8266WebServer server(80);
int RevB(int v, byte b) //设置第 B 位的值取反
{
return (v ^= 1 << b); //把 Number 的 POS 位取反
}
void SaveSta()
{
EEPROM.begin(512);
EEPROM_write(30, StatSave);
EEPROM.commit();
EEPROM.end();
}
void setSta(byte Aswitch, byte Sta )
{
byte i = Aswitch / 8;
byte j = Aswitch % 8;
bitWrite(TSwitch[3 - i], j, Sta);
}
void flash() {
digitalWrite(flashLed, ledState);
ledState = !ledState ;
}
void handleRoot() {
sprintf_P(funcstr, pageS, "<form action=/op1><input type=submit
value=k1 开></form>"
"<form action=/op2><input type=submit value=k1
关></form>"
"<form action=/op3><input type=submit value=k2
开></form>"
"<form action=/op4><input type=submit value=k2
关></form>"
"<form action=/op5><input type=submit value=k3
开></form>"
"<form action=/op6><input type=submit value=k3
关></form>"
"<form action=/op7><input type=submit value=k4
开></form>"
"<form action=/op8><input type=submit value=k4
关></form>"
"<form action=/op9><input type=submit value=全
开></form>"
"<form action=/op10><input type=submit value=全
关></form>");
server.send(200, "text/html", funcstr);
}
void op1() {
opoper(0, 1, "k1 已开");
}
void op2() {
opoper(0, 0, "k1 已关");
}
void op3() {
opoper(1, 1, "k2 已开");
}
void op4() {
opoper(1, 0, "k2 已关");
}
void op5() {
opoper(2, 1, "k3 已开");
}
void op6() {
opoper(2, 0, "k3 已关");
}
void op7() {
opoper(3, 1, "k4 已开");
}
void op8() {
opoper(3, 0, "k4 已关");
}
void op9() {
opoper(4, 1, "全已开");
}
void op10() {
opoper(4, 0, "全已关");
}
void opoper(byte port, byte oper, char *str)
{
char funcstr1[100];
if (oper == 1) {
Serial.write(openc[port], 8);
}
else
{
Serial.write(closec[port], 8);
}
if (port < 4) {
digitalWrite(SwitchIO[port], oper);
setSta(port, oper);
}
funcstr[0] = 0;
sprintf_P(funcstr1, LineS, str);
sprintf_P(funcstr, pageS, funcstr1);
server.send(200, "text/html", funcstr);
}
void handleNotFound() {
String message = "File Not Found\n\n";
message += "URI: ";
message += server.uri();
message += "\nMethod: ";
message += (server.method() == HTTP_GET) ? "GET" : "POST";
message += "\nArguments: ";
message += server.args();
message += "\n";
for (uint8_t i = 0; i < server.args(); i++) {
message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
}
server.send(404, "text/plain", message);
}
void setup(void) {
Serial.begin(9600);
pinMode(flashLed, OUTPUT);
for (int i = 0; i < Switchnum / 8; i++)
for (int j = 0; j < 8; j++) {
pinMode(SwitchIO[i * 8 + j], OUTPUT);
digitalWrite(SwitchIO[i * 8 + j], (bitRead(TSwitch[3 - i], j) >
0) && (bitRead(StatSave[3 - i], j) > 0));
}
Serial.print("Conn to:");
Serial.println(Apid);
if (aptype == 1) {
WiFi.softAP(softAPID, ApPass);
IPAddress myIP = WiFi.softAPIP();
Serial.print("AP IP address:");
Serial.println(myIP);
}
else
{
WiFi.config(APip, APGateWay, APSubNet);
WiFi.begin(Apid, ApPass);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("server started:");
Serial.println(WiFi.localIP());
}
server.on("/", handleRoot);
server.on("/op1", op1);
server.on("/op2", op2);
server.on("/op3", op3);
server.on("/op4", op4);
server.on("/op5", op5);
server.on("/op6", op6);
server.on("/op7", op7);
server.on("/op8", op8);
server.on("/op9", op9);
server.on("/op10", op10);
server.onNotFound(handleNotFound);
server.begin();
tickerflash.attach_ms(800, flash);
}
void loop(void) {
server.handleClient();
WiFiClient client = server.client();
client.flush();
}