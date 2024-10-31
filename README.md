<p align="center">
  <img src="https://img.shields.io/github/v/release/Witty-Wizard/LightWave" alt="GitHub Release">
  <img src="https://img.shields.io/github/stars/Witty-Wizard/LightWave?style=flat" alt="GitHub Repo stars">
  <img alt="GitHub Issues or Pull Requests" src="https://img.shields.io/github/issues/Witty-Wizard/LightWave">
  <img alt="GitHub forks" src="https://img.shields.io/github/forks/Witty-Wizard/LightWave?style=flat">
  <img src="https://img.shields.io/github/license/Witty-Wizard/LightWave" alt="GitHub License">
</p>

# Lightwave

LightWave is a project built using the ESP32 microcontroller. It provides a web-based GUI for configuration accessible via mDNS at `lightwave.local`. The primary purpose of this device is to control electrical appliances by turning them on and off at scheduled times.

## Features

- **Scheduled Control**: Automatically turn appliances on/off based on a set schedule.
- **Web GUI Configuration**: Configure settings easily through a web interface.
- **mDNS Support**: Access the device via `lightwave.local` without needing an IP address.
- **Real-Time Clock (RTC)**: Keeps accurate time even without internet connectivity.
- **Network Time Protocol (NTP)**: Synchronizes the device time over the internet.

## Hardware Requirements

- **ESP32 Development Board**
- **RTC Module**
- **Relay Module** for controlling appliances
- **Power Supply**
- **Optional**: Enclosure for the device

## WiFi Credentials
> [!Note]
> You can change them through the webportal, go to `lightwave.local`
- **SSID**: LightWave
- **Password**: therebelight

