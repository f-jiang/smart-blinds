
# smart-blinds
Smart-home blinds attachment with remote-control and scheduling capabilities. Device consists of a stepper motor with an attachment for turning blinds cords and is controlled by an Arduino Uno board. An ESP8266 enables the device to communicate on the network using MQTT. Home Assistant is used for scheduling and remote-control capabilities. The blinds can also be controlled manually using two buttons.

# Repo Contents

## `arduino-src`

Source code for the blinds attachment

```
arduino-src/
├── mqtt_client  # code for ESP8266, used to grant the Arduino board network access thru the MQTT protocol
│   └── mqtt_client.ino
└── smart_blinds  # arduino code for physical control of the blinds attachment
    ├── smart_blinds.h
    ├── smart_blinds.ino  # arduino sketch
    ├── tests.cpp  # "unit tests"
    ├── tests.h
    └── wear_leveled_eeprom_object.h  # utility for storing motor position, etc. in EEPROM in a wear-minimizing way
```

## `cad`

CAD drawings and assemblies. CAD packages used: AutoCAD 2016, SOLIDWORKS 2017

```
cad/
├── exports
│   └── baseplate.DWG  # chassis drawing, to be laser-cut on acrylic
├── parts  # SOLIDWORKS parts
│   ├── arduino-l298n-1.snapshot.1
│   ├── arduino-uno-4.snapshot.3
│   ├── baseplate.SLDPRT  # acrylic chassis
│   ├── cord gear  # 3d-printed blinds cord attachment
│   ├── motherboard-stand-standoff-screw-1.snapshot.1
│   ├── nema-17-nema-23-stepper-motor-mounts-1.snapshot.4
│   ├── nema-17-stepper-motor-13.snapshot.1
│   └── protoboard-5x7-cm.SLDPRT  # perfboard containing mostly power circuitry
└── smart-blinds.SLDASM  # SOLIDWORKS assembly
```

## `docs`

Misc. notes, including technical details, logged work, and unused ideas

## `home-assistant-config`

Setup files for Home Assistant home automation platform

```
home-assistant-config/
├── automations.yaml  # config code to make blinds open/close at sunrise/sunset
├── configuration.yaml  # basic setup including MQTT topic definitions
├── customize.yaml
├── groups.yaml
├── scripts.yaml
└── secrets.yaml
```

# Retrospective
- Could have used ESP32, which has both WiFi capability and enough pins for motor control etc.
- Linear regulators are too hot and inefficient for large voltage stepdowns
- Stepper motors are too big and heavy and lack position control on their own. Could have used a high-torque servo instead.
