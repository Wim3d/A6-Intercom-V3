
#include <SoftwareSerial.h>
#define A6Serial Serial

#define BLUELEDPIN 5
#define GREENLEDPIN 4
#define BUTTONPIN 2
#define SERIALDEBUG 0

SoftwareSerial DebugSerial(8, 9); // RX, TX

// variables for the phone numbers
char phone_no[] = "XXXXXXXXXXX"; // phone nr

int8_t answer;

int32_t maxoutgoing = 120000;   // 120000 ms max outgoing call duration is 2 min
int32_t maxincoming = 120000;   // 120000 ms max outgoing call duration is 2 min
int32_t calltime = 0;
int32_t starttime = 0;

int incomingcall = 0;

void setup()
{
  delay(500);
  pinMode(BLUELEDPIN, OUTPUT);
  pinMode(GREENLEDPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  // Open Serial communications
  if (SERIALDEBUG)
  {
    DebugSerial.begin(57600);
  }
  A6Serial.begin(115200);
  digitalWrite(GREENLEDPIN, HIGH);
  delay(500);
  digitalWrite(GREENLEDPIN, LOW);
  DebugSerial.println("");
  DebugSerial.println("start setup");
  smartDelay(17000);
  A6start();

  DebugSerial.println("end of setup");
  digitalWrite(GREENLEDPIN, HIGH);
}

void loop()
{
  //DebugSerial.println("loop");
  //handle the button
  if (digitalRead(BUTTONPIN) == LOW)
  {
    digitalWrite(BLUELEDPIN, HIGH);
    digitalWrite(GREENLEDPIN, LOW);
    A6Serial.print("ATD");
    A6Serial.println(phone_no); // call number
    smartDelay(2000);
    starttime = millis();
    calltime = 0;
    while (calltime < maxoutgoing && digitalRead(BUTTONPIN) == HIGH)        // loop for the call, ended by timer or a hang up from both sides
    {
      calltime = millis() - starttime;
      DebugSerial.println(calltime);
      DebugSerial.println(digitalRead(BUTTONPIN));
      smartDelay(50);
    }
    sendATcommand("ATH", "OK", 1000);                     // hang up the A6 module
    smartDelay(1000);
    digitalWrite(BLUELEDPIN, LOW);
    smartDelay(2000);
    digitalWrite(GREENLEDPIN, HIGH);
  }
  Incoming();  // check if an incoming call
  if (incomingcall == 1)  //incoming call
  {
    DebugSerial.println("Incoming call, answer call");
    digitalWrite(BLUELEDPIN, HIGH);
    digitalWrite(GREENLEDPIN, LOW);
    smartDelay(500);
    sendATcommand("ATA", "OK", 1000);   // answer phone call
    starttime = millis();
    calltime = 0;
    while (calltime < maxincoming && digitalRead(BUTTONPIN) == HIGH)        // loop for the call, ended by timer or a hang up from both sides
    {
      calltime = millis() - starttime;
      DebugSerial.println(calltime);
      DebugSerial.println(digitalRead(BUTTONPIN));
      smartDelay(50);
    }
    sendATcommand("ATH", "OK", 1000);                   // hang up the A6 module
    smartDelay(1000);
    digitalWrite(BLUELEDPIN, LOW);
    smartDelay(2000);
    digitalWrite(GREENLEDPIN, HIGH);
    incomingcall = 0;
  }
  delay(50);
}

void A6start()
{
  DebugSerial.println("A6 begin...");
  //smartDelay(2000);
  answer = 0;
  // checks if the module is started
  answer = sendATcommand("AT", "OK", 2000);
  if (answer == 0)
  {
    // waits for an answer from the module
    while (answer == 0)
    { // send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000);
    }
  }
  int8_t answer1 = 0;
  smartDelay(1000);
  answer1 = sendATcommand("AT+CREG?", "+CREG: 1,1", 2000);
  if (answer1 == 0)
  {
    // waits for an answer from the module
    while (answer1 == 0)
    { // checks whether module is connected to network
      answer1 = sendATcommand("AT+CREG?", "+CREG: 1,1", 2000);

    }
  }
  sendATcommand("AT+CMGD=1,4", "OK", 5000);
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialice the string
  delay(100);
  while (A6Serial.available() > 0) A6Serial.read();   // Clean the input buffer
  A6Serial.println(ATcommand);    // Send the AT command
  //Serial.println(ATcommand);    // Print the AT command on DebugSerial
  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    // if there are data in the UART input buffer, reads it and checks for the asnwer
    if (A6Serial.available() != 0) {
      response[x] = A6Serial.read();
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;
      }
    }

    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));
  DebugSerial.println(response);
  return answer;
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    if (A6Serial.available())
    {
      DebugSerial.write(A6Serial.read());
    }

  } while (millis() - start < ms);
}

void Incoming()
{
  while (A6Serial.available())
  {
    if (A6Serial.find("RING"))
    {
      incomingcall = 1;
      break;
    }
    else
      incomingcall = 0;
  }
}
