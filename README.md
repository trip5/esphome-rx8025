# `esphome-rx8025`

An `rx8025` component for `esphome`. This is a drop-in replacement of `ds1307`.
The driver was almost entirely cribbed from the official ds1307 driver with a few changes to match the RX8025 spec.

## Usage

See [Time](https://esphome.io/components/time.html) component.

```yaml
# import the component from GitHub
  - source:
      type: git
      url: https://github.com/trip5/esphome-rx8025
      ref: main
    refresh: 60s
    components: [ rx8025 ]

esphome:
  name: device-name
  on_boot:
    then:
      rx8025.read_time:

time:
  - platform: rx8025
    id: my_time
    update_interval: never
    timezone: "$time_zone"
    address: 0x64 # Epson's Default, check the logs during scan phase to be sure! (0x32 on my chip marked R8025 E0N25)
    battery1.5v: true # Sets low-voltage detection to 1.3V (default is 2.1V)

  # a time source is required (one of these 2)
  - platform: sntp
    id: sntp_time
  - platform: homeassistant
    id: my_time
    timezone: "AST4ADT,M3.2.0,M11.1.0"
    update_interval: 24h
    on_time_sync:
      then:
        rx8025.write_time:

i2c:
  sda: $sda_pin
  scl: $scl_pin
  scan: true
  id: i2cbus
  frequency: "400kHz" #  Leave out to default to 50kHz on ESP8266 and 100kHz on ESP32, but 400kHz is the fastest safe speed for the RX8025

```

## Status

* Tested on ESP-12 with VFD Clock but should work on all ESP chips normally
* Not actually sure low-voltage detection is useful

## Notes

On pages 20-22, there are ways to detect abnormalities in the RTC clock.  In your code, you should do a read first, then check the values of
3 variables.  Here they are with their normal values:

```cpp
rx8025_.reg.pon = false;
rx8025_.reg.xst = true;
rx8025_.reg.vdet = false;
```

To keep it simple, if pon is true and xst is false, a critical error has occured and the RTC clock has lost reliability.
If xst is false and vdet is true, the battery is dangerously low but the RTC clock is still usable.
These values are reset upon writing so only a read will fetch their true values.

## Logs

### `ESP-12`

Using 50kHz and battery1.5v set to true:


```console
... snipped ...

[01:50:21][C][i2c.arduino:072]:   SDA Pin: GPIO5
[01:50:21][C][i2c.arduino:073]:   SCL Pin: GPIO4
[01:50:21][C][i2c.arduino:074]:   Frequency: 400000 Hz
[01:50:21][C][i2c.arduino:086]:   Recovery: bus successfully recovered
[01:50:21][I][i2c.arduino:096]: Results from i2c bus scan:
[01:50:21][I][i2c.arduino:102]: Found i2c device at address 0x32

... snipped ...

[01:50:22][C][homeassistant.time:010]: Home Assistant Time:
[01:50:22][C][homeassistant.time:011]:   Timezone: 'AST4ADT,M3.2.0,M11.1.0'
[01:50:22][C][rx8025:022]: rx8025:
[01:50:22][C][rx8025:023]:   Address: 0x32
[01:50:22][C][rx8025:027]:   Timezone: 'AST4ADT,M3.2.0,M11.1.0'

... snipped ...

[01:50:26][D][api:102]: Accepted 192.168.1.31
[01:50:26][D][api.connection:1389]: Home Assistant 2024.9.1 (192.168.1.31): Connected successfully
[01:50:26][D][time:050]: Synchronized time: 2024-10-24 13:50:26

... snipped ...

[01:50:34][D][rx8025:097]: Read  16:50:29 2024-10-24 [ Checking Registers: 24 Hour: OFF vdet: Voltage Normal (vdsl: 1.3V Threshold) xst: Oscillator Normal pon: Power On Reset Detection Normal ]
[01:50:34][D][time:050]: Synchronized time: 2024-10-24 13:50:29

... snipped ...

[01:50:49][D][rx8025:110]: Write  16:50:44 2024-10-24  [ Setting defaults: 24 Hour: ON vdet: Voltage Normal (vdsl: 1.3V Threshold) xst: Oscillator Normal pon: Power On Reset Detection Normal ]
```
Note that it does appear normal that the 24 Hour bit is read as OFF even written as ON.  It is always set by the driver as true because ESPHome Time expects 24-Hour Time.

I'm guessing these registers only change when the RTC is receiving only battery power. While connected to the ESP, the RTC will still receive power so no failure of the RTC will occur and will only be detected when powering-up again. I wish I could test more but I can't get my VFD Clock to do proper serial monitoring (and Wi-fi monitoring does not catch enough pre-connection logging).
