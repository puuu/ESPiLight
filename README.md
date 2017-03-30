[![Build Status](https://travis-ci.org/puuu/ESPiLight.svg?branch=master)](https://travis-ci.org/puuu/ESPiLight)

# ESPiLight

This Arduino library is a port of the [pilight](https://pilight.org/)
433.92MHz protocols to the Arduino platform. It was tested with a
ESP8266. The aim is to transmit, receive and parse many 433.92MHz
protocols, by providing a simple Arduino friendly API. This should
help to implement IoT bridges between the 434MHz-RF band and internet
protocols.

By porting the C modules of the pilight protocols, allows to
participate on the excellent work of the pilight community. Thus,
supporting many protocols for rc switches and weather stations without
reimplementing them.

A list of supported protocols can be found in the pilight wiki:
https://wiki.pilight.org/doku.php/protocols


## Installation

This library can be easily installed with the Arduino Library Manager.


## Usage

Please have a look to the examples.


### Requirements

This library was tested and developed for the
[ESP8266](https://github.com/esp8266/Arduino). It may not run on a
ATmega-based boards, because of memory usage.

For transmitting and receiving you need 434MHz-RF modules. More
information can be found here:
- https://wiki.pilight.org/doku.php/receivers
- https://wiki.pilight.org/doku.php/senders
- https://github.com/sui77/rc-switch/wiki/List_TransmitterReceiverModules


## Contributing

If you find any bug, feel free to open an issue at github.  Also, pull
requests are welcome. The development takes place in the `master`
branch. The `release` is used to integrate the pilight files and for
version tagging, like necessary for the Arduino Library Manager
crawler.


### Install from source

If you are interested to install this library from source, you need to
integrate the pilight source files. Since Arduino build all cpp/c
files in the src directory and sub directories, only necessary files
from pilight will be integrated.  This will be done with GNU make.

On Linux you can run:
```console
$ git clone https://github.com/puuu/ESPiLight/
$ cd ESPiLight
$ make
$ ln -s `pwd` ~/Arduino/libraries/
```


#### Update

To update EPiLight from git run:
```console
$ git pull
$ make update
```


## Acknowledgement

Big thanks goes to the pilight community, which implemented all the
434MHz protocols. If you want to integrate more protocols, please
contribute directly to [pilight](https://pilight.org/).


## Other similar projects

There are other, more lightweight 434Mhz implementations:
- https://github.com/sui77/rc-switch
- https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home
