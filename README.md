# Prototype
The goal of the escape room is to steal and upload secret prototype data. This group was responsible for designing the storage device, the first step for uploading the data and surrounding puzzles. We chose floppy disks as storage devices. The floppys were hidden inside a safe, which was build by the safe group. The floppys are lying on a scale, which was build by us, so that it can be detected if they were taken. Using these floppys the players are able to "upload" the data by solving a picture rearranging puzzle in the server room. 

## Scale Puzzle
### Building the scale
Plexiglas plates, a 5 kg load cell with an HX711 and an ESP8266 were used for building the scale. The ESP is responsible for checking the weight and handling the MQTT connection. The state machine handling the communication can be seen in the [presentation](Ubilab_Scale_Presentation.pdf).

### Solving the puzzle by tricking STASIS
To get the 4 floppy disks (e.g. the prototype) out of the safe, you have to replace them with 4 different floppy disks, that can be found in the room. If you remove the prototype disks without replacing them, STASIS is very angry and wants the prototype back.

### Hints
1. Have you watched Indiana Jones? Maybe a famous scene inspires you.
2. Can you see more floppy disks outside the safe?
3. Try to replace the floppy disks inside the safe.

## Reading the prototype

### Building the puzzle
An raspberry pi, a 7 inch touchscreen display and two USB floppy drives were used for this puzzle. The parts were mounted to a lasercutted and engraved wooden panel. This panel is set into a "server rack" inside the server room. 

We changed the disk identifiers of the four prototype floppy disks to 1, 2, 3, 4. Which floppy drive is inserted into which drive can be detected by running this [script](check_floppy.sh) peridodically. This output is then used for rotating the image accordingly. For the MQTT communication the [paho library](https://github.com/eclipse/paho.mqtt.c) was used.

#### Hack for malfunctioning floppy
One of the floppy drives got stuck if the floppy was ejected at the wrong time. This led to a timeout of the script. If a timeout occured the usb ports were turned off and on to restart the floppy drive. [Uhubctl](https://github.com/mvp/uhubctl) was used for this. This could be removed if the puzzle was rebuilded with a new floppy drive.

### Solving the puzzle by reassembling the image
To read the data from the prototype you have to reassemble an image. The image shifts depending on which disk is inserted into which reader. The colors of the disks match the part of the image that is changed. One reader shifts the position of the matching colored part, the other rotates the matching colored parts themselves.

### Hints
1. Be patient, watch closely and reassemble the image
2. You need all four floppy disks
3. The floppy disk colors match the image colors

