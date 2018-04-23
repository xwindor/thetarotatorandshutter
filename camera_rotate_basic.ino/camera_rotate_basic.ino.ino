
#include <SparkFunESP8266WiFi.h>
#include <SoftwareSerial.h> 
#include "BasicStepperDriver.h"


#define MOTOR_STEPS 200
#define RPM 65
#define MICROSTEPS 1

#define DIR 5
#define STEP 4
//Number of steps per revolution

BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
const char destServer[] = "192.168.1.1";


const char* header = "POST /osc/commands/execute HTTP/1.1\r\n"
                           "Host: 192.168.1.1\r\n"
                           "Accept: */*\r\n"
                           "Content-Type: text/plain;charset=UTF-8\r\n"
                           "Connection: close\r\n";
                           
const char* takePicture =  "Content-Length: 70\r\n"
                           "\r\n"
                           "{\"name\": \"camera.takePicture\",\"parameters\": {\"sessionId\": \"SID_0001\"}}";

const char* sessionStart = "Content-Length: 48\r\n"
                           "\r\n"
                           "{\"name\": \"camera.startSession\",\"parameters\": {}}";

const char* setSettings =  "Content-Length: 133\r\n"
                           "\r\n"
                           "{\"name\": \"camera.setOptions\",\"parameters\":{\"sessionId\": \"SID_0001\", \"options\":{\"iso\": 100,\"exposureProgram\": 9,\"sleepDelay\": 65535}}}";

                           
const char mySSID[] = "THETAXS00259915.OSC";
const char myPSK[] = "00259915";

void setup(){
pinMode(11, OUTPUT);
digitalWrite(11, HIGH);

stepper.begin(RPM, MICROSTEPS);
esp8266.begin();
esp8266.setMode(ESP8266_MODE_STA);
esp8266.connect(mySSID, myPSK);
        ESP8266Client client;
        client.connect(destServer, 80);
sendRequest(header, sessionStart);
sendRequest(header, setSettings);
client.stop();

}

void loop(){
  
 while(true){
  while (true){ 
  bool pausePhoto = sendRequest(header, takePicture);

    while(pausePhoto){
        stepper.move(350);
        delay(1750);
        pausePhoto = false;        
         }

         
}
}
}

bool sendRequest(const char* header, const char* body){
        ESP8266Client client;
        client.connect(destServer, 80);
        client.print(header);
        client.print(body);
        client.flush();
        bool HTTPstatusOK = checkHTTPstatusOK(client);
        client.stop();
        return HTTPstatusOK;
}


 
bool checkHTTPstatusOK(ESP8266Client & client) {
  unsigned long timeout = millis();

  /* Read until the first space (SP) of the response. */
  char data = 0;
  while (data != ' ') {  // wait for a space character
    if (client.available() > 0) {  // if theres a character to read from the response
      data = client.read();  // read it
    }
    // if the space doesn't arrive within 5 seconds, timeout and return
    if (millis() - timeout > 5000) {  
      Serial.println("Error: Client Timeout !");
      return false;
    }
    yield();
  }
  
  /* The three next characters are the HTTP status code.
      Wait for 3 characters to be received, and read them into a buffer. */
  while (client.available() < 3) {
    if (millis() - timeout > 5000) {
      Serial.println("\r\nError:\tClient Timeout !");
      return false;
    }
    yield();
  }
  char statusCodeStr[4];  // three digits + null
  client.read((uint8_t*) statusCodeStr, 3);  // read the status code into the buffer
  statusCodeStr[3] = '\0';  // add terminating null character, needed for strcmp()

  /* Check if the status code was "200". If so, the request was successful. */
  if (strcmp(statusCodeStr, "200") != 0) {
    Serial.print("\r\nError:\tHTTP status ");
    Serial.println(statusCodeStr);
    return false;
  }
  
  return true;
}

