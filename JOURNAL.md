---
title: "BeetleBoard"
github: "avycado13/beetleboard"
description: "a basic board for making versatile beetleweight battlebots"
created_at: "2026-09-08"
---

My friend wants to do more battlebot type things with 5th and 6th graders so I thought I might make a board that has enough motor and servo ports to be useful for all types of bots and not constrained to one. I guess I am calling it BeetleBoard!
 
# September 8: Added Basic Power Circuitry
I made my circuit to take a LiPo battery and then first it goes through a TVS Diode to protect it from voltage spikes. Then it gets decoupled through 4 Capacitors and toned down to 5v with a buck converter. I also added some battery charge sensing. (I took heavy inspiration from Kieran's [Holy Guacamole](https://github.com/taciturnaxolotl/holy-guacamole/))

![Power subsheet](https://cdn.hackclub.com/01a08893-910c-7a69-a05c-7df8fe556bb7/beetleboard-1.png)
**Total time spent: 2h**


# September 9: Fixed Power Circuitry

So I might be a bit dumb and have for gotten some stuff, so I fixed that up and needed to fiddle with KiCad libraries.
Also, I kinda gave up on implementing a proper undervolting lock out circuit in hardware, so I will just use my voltage sensing to control the enable pin in software. (I forgot to commit this 4 hours ago)

![Power Subsheet Rev. 2](https://cdn.hackclub.com/01a089e2-8680-7bc0-ba7c-a442a68f6c99/beetleboard-2.png)
**Total time spent: 1h**

# September 9: ESC Ports
After taking way too long to understand the difference between PWM and DShot, I understand it now. Interestingly you can use the same connector layout for both (I think). I added 6 ports for ESCs, they all look like servo ports and can function as such because they can do both DShot and PWM. It might seem a bit excessive to have 6 Servo ports, but because this is for experimentation it should turn out fine. Also did I tell you I decided on a XIAO ESP32-C6 for it?

![Main Sheet](https://cdn.hackclub.com/01a089ff-d48e-718a-a0f3-08e72b2689d6/beetleboard-3.png)
**Total time spent: 2h**


# September 10: Added 6 Axis IMU
I was on a huddle with Kian and he told me about the MPU-6050 which is a 6 axis imu. I was debating whether to put an imu on this as it would have been a lot of work to wire up multiple so this board simplifies it a lot. it was actually really simple to wire up as it just used i2c (TIL that its pronounced I squared c) I also added a test point on the 5V Rail and GND.
I might add a header on the board to connect to a radio receiver as apparently ESP32 WiFi signals don't work that great.

![New schematic with IMU](https://cdn.hackclub.com/01a08c5c-1dc9-76da-9a88-03068a8e085d/beetleboard-4.png)
**Total time spent: 1h**


# September 10: Added Power distrubution
I kinda forgot that the ESCs require a seperate source of power with a higher voltage to actually power the motors, so i added some power distrubution to the board. It goes directly from the battery through a TVS Diode and some capactors to clean up voltage spikes and decouple it then gets split into 8 connectors for the ESCs and one for the buck converter. I also added an SPST switch on the power connector where i can eventually put a FingerTech Switch or just solder them together.

![New power subsheet with distrubution](https://cdn.hackclub.com/01a08e46-c094-7faf-a64d-3834eafe02d6/beetleboard-5.png)
**Total time spent: 1h**

# September 11: Fixed IMU
I realized that my original IMU wouldn't be able to handle more than 16G, which is really low because on impact it could hit 200G and for meltybrains it would overload because of spinny. Instead, I replaced it with an LSM6DSV320XTR 6 axis IMU that could handle up to 300Gs. I wired it over SPI to connect everything and I put it in its own sheet.

![New IMU over SPI](https://cdn.hackclub.com/01a08f7a-3afe-79aa-b7a9-d753bbda1060/beetleboard-6.png)
**Total time spent: 1.5h**

# September 11: Added Port for ExpressLRS Receiver
I added a connector for an ExpressLRS receiver because ESP32 bluetooth/WiFi is supposedly really unreliable. It should communicate over UART.

![New ELRS Reciever](https://cdn.hackclub.com/01a09315-656c-70c2-9617-f7255ab6a7de/beetleboard-7.png)
**Total time spent: 0.5h**

# September 13: Started PCB Layout
I assigned the footprints for components and made a really crappy layout for the PCB that kinda looks like a guitar.

![PCB](https://cdn.hackclub.com/01a09d70-c7a3-7786-b1bc-2a9173752f56/beetleboard-8.png)
**Total time spent: 0.5h**

# September 14: Redid PCB Layout and Routed
I redid the layout to be less weird shaped and cheaper to fab. I also routed it in just 2 layers!
Because its about 100mm by 70mm it should only cost 5 bucks to fab.

![New PCB Layout](https://cdn.hackclub.com/01a0a197-1997-7973-8d8f-44eda1789c8c/beetleboard-9.png)
**Total time spent: 1.5h**

# September 17: Wrote README
Today, I prepped it for shipping and mainly wrote the readme. I don't think I need a photo of the README because you can read that. I added a comprehensive list of features!

edit: turns out i do :(

![README](https://cdn.hackclub.com/01a0bf7b-d26e-766b-a8b5-6ad9637fb942/beetleboard-11.png)
**Total time spent: 1h**

# September 20: Made better BOM

I already have a BOM in my README, but I went to Octopart and made a better BOM and consolidated my parts so i will only have to order from three suppliers: LCSC, Mouser, and Newark.

![Altium BOM Portal](https://cdn.hackclub.com/01a0c145-5ef0-70d0-992e-49a1583e45ae/beetleboard-13.png)

**Total time spent: 1h**

# September 21: Redid Layout
I redid the layout of the PCB to be neater and changed the solder wire things to be a new custom footprint of 2 5mm circle pads spaced about 5mm apart. I also added M3 Mounting holes offset 6mm x 6mm from each edge. In addition, I also filleted the corners for looks. I forgot to put it when I took the picture but I also added some basic silkscreen.
![PCB with new layout](https://cdn.hackclub.com/01a0ca3d-5efc-70ac-b011-a76e1461c55b/beetleboard-14.png)
**Total time spent: 2h**
