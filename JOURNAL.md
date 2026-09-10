---
title: "BeetleBoard"
github: "avycado13/beetleboard"
description: "a basic board for making versatile beetleweight battlebots"
created_at: "2026-09-08"
---

# September 8
My friend wants to do more battlebot type things with 5th and 6th graders so I thought I might make a board that has enough motor and servo ports to be useful for all types of bots and not constrained to one. I guess im calling it BeetleBoard!
 
## September 8: Added basic power circuitry
I made my circuit to take a LiPo battery and then first it goes through a TVS Diode to proetect it from voltage spikes. Then it gets decoupled through 4 Capacitors and toned down to 5v with a buck converter. I also added some battery charge sensing. (I took heavy inspiration from kieran's [Holy Guacamole](https://github.com/taciturnaxolotl/holy-guacamole/))

![Power subsheet](https://cdn.hackclub.com/01a08893-910c-7a69-a05c-7df8fe556bb7/beetleboard-1.png)
**Total time spent: 2h**

# September 9

## September 9: Fixed power circuitry

So i might be a bit dumb and have for gotten some stuff, so i fixed that up and needed to fiddle with kicad libraries.
Also, i kinda gave up on implementing a proper undervolting lockout circuit in hardware, so i will just use my voltage sensing to control the enable pin in software. (I forgot to commit this 4 hours ago)

![Power Subsheet Rev. 2](https://cdn.hackclub.com/01a089e2-8680-7bc0-ba7c-a442a68f6c99/beetleboard-2.png)
**Total time spent: 1h**

## September 9: ESC Ports
After taking way too long to understand the difference between PWM and DShot, i understand it now. interestingly you can use the same connector layout for both (I think). I added 6 ports for ESCs, they all look like servo ports and can function as such because they can do both DShot and PWM. It might seem a bit excessive to have 6 Servo ports, but because this is for experimentation it should turn out fine. Also did I tell you I decided on a XIAO ESP32-C6 for it?

![Main Sheet](https://cdn.hackclub.com/01a089ff-d48e-718a-a0f3-08e72b2689d6/beetleboard-3.png)
**Total time spent: 2h**

# Sep 10
