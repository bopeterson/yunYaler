#define DEBUG false //NOTE: the serial monitor must be opened if debug is true
#define RELAYDOMAIN "gsiot-abcd-efgh" //Change this to your relay domain

/* 
This sketch makes it possible to control the digital outputs of the Yun connected to 
a network without having to know the Yun IP number or having to do any 
port forwarding settings on the router the yun is connected to. 
This is possible by using the yaler service (try.yaler.net)

As a bonus you can also read the analog inputs

A full tutorial can be found at http://asynkronix.se/internet-of-things-with-arduino-yun-and-yaler

1) register at try.yaler.net and note your yaler relay domain. 
2) Change the relay domain definition in this sketch (#define RELAYDOMAIN gsiot-...) to your domain
3) start terminal on mac or equivalent on pc
4) change direcory to the folder where yunYaler.py is located with something like 

   cd /Users/username/some/folder/somewhere
   
5) copy yunYaler.py to Yun with

   scp -r ./yunYaler.py root@arduino.local:/usr/lib/python2.7/bridge
   enter arduino password when prompted

   Note: change arduino.local to theNameOfYourArduino.local if you have changed the name.
   You can also change to the IP of your Arduino Yun
6) Optional: You can test yunYaler.py from the Yun command line if you like:
   connect to Yun from terminal with ssh:
   ssh root@arduino.local
   enter arduino password when prompted
   
   change directory on the yun with
   
   cd /usr/lib/python2.7/bridge
   
   start yunYaler.py with
   
   python yunYaler.py try.yaler.net gsiot-abcd-efgh nobridge
   
   nobridge means that there is no bridge communication between the two processors on the Yun *
   You can now connect from anywhere with any webbrowser with 
   http://try.yaler.net/gsiot-abcd-efgh/arduino/led/13/1
   and Yun should give some response in the ssh terminal
 7) download this sketch to the Arduino (you must have changed the relaydomain definition to your domain first). Also set the DEBUG definition to either true or false. true requires serial monitor to be opened
 8) wait until the led 13 has stopped blinkning (ca 2 minutes)
    NOTE1: it will blink fast (twice a second) for 1 minute, stop blinkning for 
    some seconds and start blinking slow (once a second) for another minute
    NOTE2: if debug=true there will be no delay. Instead led 13 will blink very fast 
    until the serial monitor is opened
 9) Test it!
    set D13 to HIGH (turn built in led on) with 
    http://try.yaler.net/gsiot-abcd-efgh/arduino/digital/13/1
    set D13 to LOW (turn built in led off) with
    http://try.yaler.net/gsiot-abcd-efgh/arduino/digital/13/0
    read D13 state with 
    http://try.yaler.net/gsiot-abcd-efgh/arduino/digital/13
    read analog input A4 with
    http://try.yaler.net/gsiot-abcd-efgh/arduino/analog/4
    
   
* the command for running yunYaler.py with the bridge enabled is

python yunYaler.py try.yaler.net gsiot-abcd-efgh bridge 

and this is done by the runYaler function in this sketch

*/
#include <Process.h>

String compiletime=__TIME__;
String compiledate=__DATE__;
String file=__FILE__;


Process p;		// Create a process and call it "p"


const int ledPin =  13;      // the number of the on board LED pin

unsigned long previousMillis = 0;        // will store last time bridge data was checked
unsigned long now;
const long interval = 200;           // interval at which to check updated bridge data

char frompython[33]; //data read from bridge that was put by python. temporary storage before it is compied to a string. Allocate space for a 32 character long rest command

String lastTime; //time when data was read from the bridge

void setup() {
  lastTime="99:99:99";
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  
  unsigned long pause;
  if (DEBUG) {
    pause=0UL;
  } else {
    pause=60UL*1000UL;
  }
  
  now=millis();

  //wait for the wlan to get established and everything else to start
  //fast blink
  while((millis()-now)<pause) {
    digitalWrite(ledPin,1-digitalRead(ledPin));
    delay(250); 
  }
  
  frompython[0]='-';
  frompython[1]='\0';

  
  // Initialize Bridge
  Bridge.begin();

  now=millis();
  //wait a little more for the wlan to get established
  //slow blink
  while((millis()-now)<pause) {
    digitalWrite(ledPin,1-digitalRead(ledPin));
    delay(500); 
  }

  if (DEBUG) {
    //this requires a serial cable between arduino and computer. 
    Serial.begin(9600);
    delay(25);
    // Wait until serial is opened. Blink fast while doing so
    digitalWrite(ledPin,LOW);
    while (!Serial) {
      digitalWrite(ledPin,1-digitalRead(ledPin));
      delay(25);
    }
    digitalWrite(ledPin,LOW);
  }
  
  
  printdebug(compiletime);
  printdebug(compiledate);
  printdebug(file);
  runYaler(); //start the yaler pyton process
}

void loop() {
  
  now = millis();
 
  if(now - previousMillis >= interval) {
    previousMillis = now;   

    //get the rest data from python
    Bridge.get("rest", frompython, 32);
    String rest=String(frompython);
    Bridge.get("time", frompython, 32);
    String time=String(frompython);
    
    if (lastTime!=time) {
      lastTime=time;
      process(rest,time);
    }   
  }
}

void runYaler() {
  p.begin("python");
  p.addParameter("yunYaler.py");
  p.addParameter("try.yaler.net");
  p.addParameter(RELAYDOMAIN); 
  p.addParameter("bridge"); //enable bridge mode
  p.runAsynchronously(); //makes it possible for the arduino to do other things while waiting for yaler request
}

void process(String rest, String time) {
  printdebug("----------");
  String temp="Processing ["+rest+"] at "+time;
  printdebug(temp);
  // read the command
  String command = head(rest);
  rest=tail(rest);
  // is "digital" command?
  if (command == "digital") {
    digitalCommand(rest);
  }
  // is "analog" command?
  else if (command == "analog") {
    analogCommand(rest);
  }
  // is "mode" command?
  else if (command == "mode") {
    modeCommand(rest);
  }
  else {
    String answer="{\"command\":\"unsupported\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);
  }
}

void digitalCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin=headNumber(rest);
  rest=tail(rest); //the part of the rest following the number

  //parse value from rest string
  value=headNumber(rest);
  rest=tail(rest);
  
  printdebug("digital command pin: "+String(pin)+", value: "+String(value));

  if (pin!=-1 && value!=-1) {
    digitalWrite(pin,value);
    String answer="{\"command\":\"digital\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"write\"}";
    Bridge.put("answer",answer);
  } else if (pin!=-1 && value==-1) {
    value=digitalRead(pin);
    String answer="{\"command\":\"digital\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"read\"}";
    Bridge.put("answer",answer);
  } else {
    String answer="{\"command\":\"digital\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
   } 
}

void analogCommand(String rest) {

  int pin, value;

  //parse pin number from rest string
  pin=headNumber(rest);
  rest=tail(rest); //the part of the rest following the number

  if (pin!=-1) {
    value=analogRead(pin);
    String answer="{\"command\":\"analog\",\"pin\":"+String(pin)+",\"value\":"+String(value)+",\"action\":\"read\"}";
    Bridge.put("answer",answer);
  } else {
    String answer="{\"command\":\"analog\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
   } 
}

void modeCommand(String rest) {
  //not implemented yet. Needed if you want to change digital pins between input and output
    String answer="{\"command\":\"mode\",\"action\":\"unsupported\"}";
    Bridge.put("answer",answer);  
}

int headNumber(String s) {
  int number;
  String numberString=head(s);
  if (numberString.length()>0) {
    number=numberString.toInt();
  } else {
    number=-1;
  }
  return number;
}  

String head(String s) {
  //returns text after leading slash until next slash 
  return s.substring(1,s.indexOf("/",1));
}

String tail(String s) {
  //returns text after second slash (if first char is a slash)
  return s.substring(s.indexOf("/",1),s.length());
}

void printdebug(String s) {
  if (DEBUG) {
    Serial.print(s);
    Serial.println(char(194));
  }
}

