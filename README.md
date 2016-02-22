# AIR Gallery Talking Birds

The [Air Gallery](http://airgallery.net) in Portland, OR has a flock of
blackbirds as mascots. For the February 2016 show the director, Fred Swan,
wanted the blackbirds to greet visitors as they entered the gallery. This
project is a simple device that detects someone approaching the display,
greeting them with a chirp and special welcome message from one of the
resident artists.

## Bill of Materials

Except for the Arduino 101 and cigar box, I purchased all of the parts from
[Adafruit](https://www.adafruit.com/).

* [Arduino 101](https://www.arduino.cc/en/Main/ArduinoBoard101)
  * The Arduino Uno should be compatible with these instructions; though
    you will want to be sure to account for power differences.
* [Sharp GP2Y0A21YK0F IR distance sensor w/cable](https://www.adafruit.com/products/164)
* [Adafruit Audio FX Mini Sound Board - 2MB](https://www.adafruit.com/products/2342)
* [Adafruit Mono 2.5W Class D Audio Amplifier](https://www.adafruit.com/products/2130)
* [3" Speaker](https://www.adafruit.com/products/1314)
* [Adafruit Perma-Proto Half-sized Breadboard PCB](https://www.adafruit.com/products/1609)
* 4.7uF capacitor
* USB A to B cable
* 5V 1A USB wall adapter
* Cigar box
  * Check your local store with a humidor. They will likely have empty
    boxes you can buy for a few dollars.
* (optional) [3" male/male jumper wires](https://www.adafruit.com/products/759)
* (optional) [6" male/male jumper wires](https://www.adafruit.com/products/1957)

## Assembly

* Connect 5V and GND from the Arduino 101 to the power rails on the perma-
  proto board.
* Solder the pins onto the audio board and amplifier
* Choose a location on the perma-proto board for the amplifier and audio
  board and solder the following pins to the perma-proto board
  * Amplifier: Vin, Gnd, A+, A-
  * Sound Board: Vin, Gnd, TX, RX, UG, R, Gnd (paired with R), Rst, Act
* Connect 5V rail to Vin on sound board and amplifier
* Connect GND rail to Gnd (paired with Vin) and UG on the sound board and
  Gnd on the amplifier
* Connect R and Gnd (paired with R) on the sound board to A+ and A- on the
  amplifier, respectively
* Connect TX and RX on the sound board to RX (pin 0) and TX (pin 1) on the
  Arduino 101, respectively
* Connect Rst on the sound board to pin 4 on the Arduino 101
* Connect Act on the sound board to pin 5 on the Arduino 101
* Choose two traces on the perma-proto board as 5V and GND to connect the
  distance sensor and connect the 5V and GND rails
* Connect the capacitor across the traces, making sure the positive lead is
  connected to the 5V trace
* Connect the red and black wires from the distance sensor to the 5V and
  GND traces next to the capacitor
* Connect the white wire from the distance sensor to pin A0 on the
  Arduino 101
* Print the
  [board stand](3d_parts/arduino_101_adafruit_half_perma_proto_stand.stl)
  and attach the Arduino 101 and perma-proto board
  * You may need to use a knife to open up the inside of the post caps,
    depending on your printer.


* Print the [distance sensor flange](3d_parts/dist_sensor_flange.stl)
* Cut a hole in the front of the cigar box to fit the sensor flange snug
* Drill a 3/4" hole in the back of the cigar box for the USB power cable
* Use a 3" hole saw to make a hole in the top of the box for the speaker
* Attach the speaker to the box over the hole
* Insert the distance sensor into the flange from inside the cigar box
  * You might need to chisel out some wood below the flange to make room
    for ther terminal block on the bottom of the sensor
* Attach the speaker to the terminal block on the amplifier
* Insert the USB cable through the hole in the back of the box
* Print the [cable hole plug](3d_parts/rear_plug.stl) (two halves), clamp
  over the USB cable and insert from the outside to seal the hole.

## Installing the Software

The project requires the
[Arduino IDE](https://www.arduino.cc/en/Main/Software) and the
[Adafruit Sound Board library](https://github.com/adafruit/Adafruit_Soundboard_library).
Installing the sound board library can be done by checking cloning a copy
from github into the Arduino library directory. On OS X, that directory is
/Users/__username__/Documents/Arduino/libraries.

```
cd ~/Documents/Arduino/libraries
git clone https://github.com/adafruit/Adafruit_Soundboard_library.git
```

## Adding Audio

There are two groups of audio files used by the project: chirps and
welcome messages. There can be up to MAX_SOUNDS in each group. If there are
more than MAX_SOUNDS files that would be part of a group, the extras are
ignored.

The chirp files contain the sounds the bird makes whenever it chirps. A
random chirp file is chosen whenever one is played. The files should be WAV
format, not OGG. The audio board does not signal on the Act pin properly
for OGG files, and it is important for this pin to have a correct signal
for proper operation of the device. I used 16 bit mono audio at 22500Hz to
save space. The file names must start with "CHIRP" (case insensitive) to be
part of this group.

The welcome messages play after the first chirp. A random message is played
whenever one is played. The files can be WAV or OGG, since a reliable
signal on Act is not required. I used Mono audio at 22500Hz in OGG format
to fit all of the artist's messages into the 2MB space.

To load the audio onto the audio board:
1. Make sure the USB cable is **NOT** connected to the Arduino 101
2. Connect your computer to the USB port on the audio board. A USB storage
   device will connect to your computer
3. Copy the audio files to the device
4. Safely eject the audio device USB storage. Watch for the drive to
   disconnect and remove the USB connection quickly, or else the device
   will reconnect to the computer

**IMPORTANT** Never have USB connected to the audio board and the
Arduino 101 at the same time. You could destroy the Arduino 101, the audio
board, or both if power is connected to both USB ports at the same time.
