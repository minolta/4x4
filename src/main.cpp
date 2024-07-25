#include <Arduino.h>
#include <Ticker.h>
#define rfwbutton 13 // สวิท rfw
#define _4h 14       // สวิท 4h
#define rfw 27
#define SolenoidA 4  // output ไปขับ ตัวดูด A
#define SolenoidB 16 // output ไปขับ ตัวดูด B
#define _4hled 17
#define rfwled 5

const int version = 2;
#define mode2h 0 // ขับสอง rfw ไม่ lock
#define mode4h 1
#define mode2hrfwlock 2
#define mode4hrfwnotlock 3

int currentmode = mode2h; // สถานะของรถ
Ticker t;

void ticker()
{
  digitalWrite(2, !digitalRead(2));
}
void setPort()
{
  pinMode(rfwbutton, INPUT_PULLUP); // กำหนด สวิทกด rfw ให้เป็น input default 1 หรือ ต่อบวก(5V ของ MCU) ไว้
  pinMode(_4h, INPUT_PULLUP);       // กำหนด ให้สวิท 4h เป็น input ต่อบวก (5V ของ MCU)ไว้
  pinMode(rfw, INPUT_PULLUP);       // กำหนดให้ rfw ที่เพลา เป็น input default hi (5V ของ MCU)

  pinMode(SolenoidA, OUTPUT);
  pinMode(SolenoidB, OUTPUT);
  pinMode(_4hled, OUTPUT);
  pinMode(rfwled, OUTPUT);
  pinMode(2, OUTPUT);
}
/**
 * @brief ตรวจสอบสถานะรถว่าอยู่ใน mode ไหน
 *
 */
void checkstatus()
{
  int _4x4_status = digitalRead(_4h);
  int rfw_status = digitalRead(rfw);
  // ค่า ของการอ่าน ถ้า อ่านได้ค่ามาเป็น 1 แสดงว่าสวิทไม่มีการกดจะลอยจาก g cpu เลยอ่านได้เป็น  1 ถ้ามีการกดจะอ่านได้เป็น 0
  if (_4x4_status == 0 && rfw_status == 1)
  {
    //
    currentmode = mode2h;
    Serial.println("mode 2h");
  }
  else if (_4x4_status == 1 && rfw_status == 1)
  {
    currentmode = mode4hrfwnotlock;
    Serial.println("mode4hrfwnotlock");
  }
  else if (_4x4_status == 1 && rfw_status == 0)
  {
    // เข้าระบบขับสี่แล้ว rfw lock แล้ว
    currentmode = mode4h;
    Serial.println("mode4h");
  }
  else if (_4x4_status == 0 && rfw_status == 0)
  {
    // อยู่ในสถานะ ขับสองแต่ rfwlock
    currentmode = mode2hrfwlock;
    Serial.println("mode2hrfwlock");
  }
}

void SolenoidTo2h()
{

  while (true)
  {
    int rfwstatus = digitalRead(rfw);

    digitalWrite(SolenoidB, 0);
    digitalWrite(SolenoidA, 1);
    if (rfwstatus == 1)
      break;
    Serial.println("Tryto 2h");
    Serial.println(rfwstatus);
  }
}
void SolenoidTo4H()
{
  digitalWrite(SolenoidA, 0);
  // delay(100);
  digitalWrite(SolenoidB, 1);
}
int readRfwButton()
{
  return digitalRead(rfwbutton);
}
void f()
{
  int rfwpush = readRfwButton(); // อ่านการกดของ rfw button
  if (rfwpush == 0)
  {

    delay(200);                // หน่วงเวลานิดหนึ่งกันมือไปโดนสวิท rfw
    rfwpush = readRfwButton(); // อ่านอีกครั้งว่ากดจริงๆหรือเปล่า

    if (rfwpush == 0)
    {
      Serial.println("RFW push...");
      // ถ้ามีการกดระบบทำการสั่ง solenoid ให้อยู่ใน mode 2h ทันที
      SolenoidTo2h();
    }
  }
}
/**
 * @brief เป็นการทำงานตาม mode ที่กำหนด
 *
 */
void run()
{
  if (currentmode == mode2h)
  {
    digitalWrite(rfwled, 0);
    digitalWrite(_4hled, 0);
    SolenoidTo2h();
  }
  else if (currentmode == mode2hrfwlock)
  {
    digitalWrite(_4hled, 0); // ปิดไฟขับ 4h
    digitalWrite(rfwled, 1);
    SolenoidTo4H();
  }
  else if (currentmode == mode4h)
  {
    SolenoidTo4H();
    digitalWrite(_4hled, 1);
    digitalWrite(rfwled, 1);
  }
  else if (currentmode == mode4hrfwnotlock)
  {
    digitalWrite(rfwled, 0);
    digitalWrite(_4hled, 1);
    SolenoidTo4H(); // พยามสั่งให้ rfw lock
  }
}
void rfwpush()
{
  if (currentmode == mode2hrfwlock)
  {
    f();
  }
}
void setup()
{
  Serial.begin(9600);
  setPort();
  t.attach(1, ticker);
}

void loop()
{
  checkstatus();
  run();
  rfwpush();
}
