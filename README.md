yunYaler
========
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
   
   python yunYaler.py try.yaler.net gsiot-q4f3-p0y9 nobridge
   
   nobridge means that there is no bridge communication between the two processors on the Yun *
   You can now connect from anywhere with any webbrowser with 
   http://try.yaler.net/gsiot-q4f3-p0y9/arduino/led/13/1
   and Yun should give some response in the ssh terminal
 7) download this sketch to the Arduino (you must have changed the relaydomain definition to your domain first). Also set the DEBUG definition to either true or false. true requires serial monitor to be opened
 8) wait until the led 13 has stopped blinkning (ca 2 minutes)
    NOTE1: it will blink fast (5 times a second) for 1 minute, stop blinkning for 
    some seconds and start blinking slow (once a second) for another minute
    NOTE2: if debug=true there will be no delay. Instead led 13 will blink very fast 
    until the serial monitor is opened
 9) Test it!
    set D13 to HIGH (turn built in led on) with 
    http://try.yaler.net/gsiot-q4f3-p0y9/arduino/digital/13/1
    set D13 to LOW (turn built in led off) with
    http://try.yaler.net/gsiot-q4f3-p0y9/arduino/digital/13/0
    read D13 state with 
    http://try.yaler.net/gsiot-q4f3-p0y9/arduino/digital/13
    read analog input A4 with
    http://try.yaler.net/gsiot-q4f3-p0y9/arduino/analog/4
    
   
* the command for running yunYaler.py with the bridge enabled is

python yunYaler.py try.yaler.net gsiot-q4f3-p0y9 bridge 

and this is done by the runYaler function in this sketch
