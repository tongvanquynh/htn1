# TCS34725 Linux Device Driver
This project provides a **Linux kernel module** for the **TCS34725 RGB color sensor**, which communicates via the **I2C bus**. The driver exposes a character device interface (`/dev/tcs34725`) that allows user-space applications to retrieve **Red**, **Green**, **Blue**, and **Clear** channel data using **ioctl commands**.
## Hardware Requirements
- Raspberry Pi 5 Model B+ (or equivalent with I2C support)
- TCS34725 Color Sensor
- I2C jumper wires
## Driver Overview
| Component         | Description       |
| ----------------- | ----------------- |
|     Driver name   |  tcs34725_driver  |
|     Device node   |  /dev/tcs34725    |
|     Device class  |  tcs34725         |
|     Communication | I2C               |
|     Device Tree   |  "ams,tcs34725"   |
## Sensor Communication
* Communicates with the **TCS34725** over the **I2C** bus
* Uses key registers:
  * ENABLE, ATIME, CONTROL
  * CDATAL, RDATAL, GDATAL, BDATAL
* Data is read in 16-bit format for each color channel
## Key Driver Functions
| Function Name                | Description                                   |
| ---------------------------- | --------------------------------------------- |
|   tcs34725_read_rgbc()       | Reads raw Red, Green, Blue, and Clear values  |
|   tcs34725_ioctl()           | Interface for user-space to fetch sensor data |
|   tcs34725_open()            | Handles device open                           |
|   tcs34725_release()         | Handles device release                        |
|   tcs34725_probe()           | Initializes the sensor when detected          |
|   tcs34725_remove()          | Cleans up resources on removal                |
|   tcs34725_init() / exit()   | Registers/unregisters the kernel module       |
## IOCTL Interface
| IOCTL Command              | Description                    |
| -------------------------- | ------------------------------ |
| TCS34725_IOCTL_READ_RGBC | Reads RGB + Clear color values |
## Device Tree Binding
Add this snippet to your **Device Tree Source (DTS)** file:

```dts
i2c@7e804000 {
    compatible = "brcm,bcm2835-i2c";
    reg = <0x7e804000 0x1000>;
    interrupts = <0x02 0x15>;
    clocks = <0x08 0x14>;
    #address-cells = <0x01>;
    #size-cells = <0x00>;
    status = "okay";
    pinctrl-names = "default";
    pinctrl-0 = <0x15>;
    clock-frequency = <0x186a0>;

    tcs34725@29 {
        compatible = "ams,tcs34725";
        reg = <0x29>;
    };
};
Then recompile your device tree:
```bash
sudo dtc -I dts -O dtb -o Your_Ras.dtb Your_Ras.dts
Enable I2C via raspi-config and reboot:
```bash
sudo raspi-config
# Interfacing Options → I2C → Enable
sudo reboot
## Wiring (for Raspberry Pi 3B+)
| Pin | Function | Connect To   |
| --- | -------- | ------------ |
| 3   | SDA      | TCS34725 SDA |
| 5   | SCL      | TCS34725 SCL |
| 1   | 3.3V     | TCS34725 VCC |
| 6   | GND      | TCS34725 GND |

---

## Building and Installing
### Step 1: Clone the Repository
```bash
git clone https://github.com/yourname/TCS34725-Driver.git
cd TCS34725-Driver
```
### Step 2: Verify I2C Communication
Use i2cdetect to check if the sensor is connected (address 0x29):
```bash
sudo i2cdetect -y 1
Expected output:
     28: -- -- -- -- -- -- -- 29
### Step 3: Build and Load the Driver
```bash
make
sudo insmod tcs34725_driver.ko
Verify with:
```bash
dmesg | tail
ls /dev | grep tcs
## 🧪 Testing the Sensor
### Compile the test program:

```bash
gcc -o test_tcs test_tcs.c
sudo ./test_tcs
```
### Sample output
Red:    2431
Green:  3102
Blue:   2845
Clear:  6623
## ❌ Uninstall the Driver
```bash
sudo rmmod tcs34725_driver
## Tips & Debugging
* View kernel logs:
  ```bash
  dmesg | tail
  ```
* Scan I2C bus:
  ```bash
  i2cdetect -y 1
  ```
* List device nodes:
  ```bash
  ls /dev
  ```
* Remove existing driver if loaded:
  ```bash
  lsmod | grep tcs
  sudo rmmod tcs34725_driver


