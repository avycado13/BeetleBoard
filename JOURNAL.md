---
title: "BeetleBoard"
github: "avycado13/beetleboard"
description: "a basic board for making versatile beetleweight battlebots"
created_at: "2026-09-08"
---

# September 8
My friend wants to do more battlebot type things with 5th and 6th graders so I thought I might make a board that has enough motor and servo ports to be useful for all types of bots and not constrained to one. I guess im calling it BeetleBoard!
 
# September 8: Added basic power circuitry
I made my circuit to take a LiPo battery and then first it goes through a TVS Diode to proetect it from voltage spikes. Then it gets decoupled through 4 Capacitors and toned down to 5v with a buck converter. I also added some battery charge sensing. (I took heavy inspiration from kieran's [Holy Guacamole](https://github.com/taciturnaxolotl/holy-guacamole/))

![Power subsheet](https://cdn.hackclub.com/01a08893-910c-7a69-a05c-7df8fe556bb7/beetleboard-1.png)
**Total time spent: 2h**


# September 9: Fixed power circuitry

So i might be a bit dumb and have for gotten some stuff, so i fixed that up and needed to fiddle with kicad libraries.
Also, i kinda gave up on implementing a proper undervolting lockout circuit in hardware, so i will just use my voltage sensing to control the enable pin in software. (I forgot to commit this 4 hours ago)

![Power Subsheet Rev. 2](https://cdn.hackclub.com/01a089e2-8680-7bc0-ba7c-a442a68f6c99/beetleboard-2.png)
**Total time spent: 1h**

# September 9: ESC Ports
After taking way too long to understand the difference between PWM and DShot, i understand it now. interestingly you can use the same connector layout for both (I think). I added 6 ports for ESCs, they all look like servo ports and can function as such because they can do both DShot and PWM. It might seem a bit excessive to have 6 Servo ports, but because this is for experimentation it should turn out fine. Also did I tell you I decided on a XIAO ESP32-C6 for it?

![Main Sheet](https://cdn.hackclub.com/01a089ff-d48e-718a-a0f3-08e72b2689d6/beetleboard-3.png)
**Total time spent: 2h**


# September 10: added 6 axis IMU
I was on a huddle with Kian and he told me about the MPU-6050 which is a 6 axis imu. I was debating whether to put an imu on this as it would have been a lot of work to wire up multiple so this board simplifies it alot. it was actually really simple to wire up as it just used i2c (TIL that its pronoucned i squared c) I also added a test point on the 5V Rail and GND.
I might add a header on the board to connect to a radio reciever as apparently esp32 wifi signals dont work that great.

![New schematic with IMU](https://cdn.hackclub.com/01a08c5c-1dc9-76da-9a88-03068a8e085d/beetleboard-4.png)
**Total time spent: 1h**


# September 10: Added Power distrubution
I kinda forgot that the ESCs require a seperate source of power with a higher voltage to actually power the motors, so i added some power distrubution to the board. It goes directly from the battery through a TVS Diode and some capactors to clean up voltage spikes and decouple it then gets split into 8 connectors for the ESCs and one for the buck converter. I also added an SPST switch on the power connector where i can eventually put a FingerTech Switch or just solder them together.

![New power subsheet with distrubution](https://cdn.hackclub.com/01a08e46-c094-7faf-a64d-3834eafe02d6/beetleboard-5.png)
**Total time spent: 1h**
