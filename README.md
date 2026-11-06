# The Actual Bot — Capstone Rover

Raspberry Pi 5 + 2D 360° RPLIDAR + camera. Goal: drive a track autonomously,
avoid obstacles, read AprilTags on a wall, and park in a designated spot.

**Software stack:** Ubuntu Server 24.04 LTS (64-bit) on the Pi → ROS 2 Jazzy.

## Folder layout

```
The Actual Bot/
├── README.md                ← you are here
├── docs/
│   ├── SETUP_LOG.md         ← dated log of everything done to the Pi / laptop
│   ├── hardware.md          ← parts, ports, wiring, what's known about each part
│   ├── 01_flash_sd_card.md
│   ├── 02_ethernet_to_pi.md
│   ├── 03_ros2_install.md
│   ├── 04_lidar_bringup.md
│   ├── 05_hallway_demo.md   ← hallway centring + obstacle avoidance + start button
│   ├── 06_drivetrain_link.md ← Pi → ESP32 drivetrain over USB, calibration
│   └── TEAM_ACCESS.md       ← GitHub + how a teammate gets onto the Pi
├── pi_setup/                ← scripts that run ON the Pi
│   ├── bootstrap_ros2_jazzy.sh
│   ├── 99-rplidar.rules
│   ├── pin_lidar_port.sh    ← lock /dev/rplidar to the LiDAR (if ESP32 uses the same chip)
│   ├── rover-demo.service   ← start the demo at boot (untethered runs)
│   └── install_autostart.sh
├── ros2_ws/                 ← ROS 2 workspace (copied to the Pi, built with colcon)
│   └── src/rover_bringup/   ← our package
│       ├── launch/          ← lidar.launch.py, demo.launch.py
│       ├── config/          ← lidar.yaml, hallway.yaml, drive.yaml (tuning)
│       ├── rover_bringup/   ← hallway_logic/follower, drive_bridge, start_button, scan_monitor
│       └── test/            ← unit tests + sim_drive.py hallway simulator
├── firmware/                ← drivetrain ESP32 code: mecanum/ (original), mecanum_auto/ (use this)
├── tools/                   ← laptop-side helper scripts
│   └── lidar_identify.py
└── Old Secon Bot USB for PI/   ← READ-ONLY reference. Do not edit.
```

## Status (2026-10-24)
- ✅ Pi 5 on Ubuntu 24.04 + ROS 2 Jazzy, reachable at `ssh rover@rover.local`
- ✅ RPLIDAR C1 streaming `/scan`, orientation verified (arrow = front)
- ✅ Hallway follow + obstacle avoidance logic written and passing simulation
- ✅ Drivetrain link written: ESP32 AUTO mode (`firmware/mecanum_auto`) + `drive_bridge`,
  tested off-robot. Controller **Y** = start, Y/B/stick = take back control
- ⏳ Next: flash ESP32, plug into Pi, wheels-up test, calibrate (`docs/06_drivetrain_link.md`)

> **Old Secon Bot USB for PI** turned out to be a Duet 3D-printer SD card
> (RepRapFirmware config + print G-code). No rover code in it. Left untouched.
>
> The stray `lidar/` folder from the first session is a duplicate of
> `tools/lidar_identify.py` and can be deleted.

## Quick start (once the SD card is flashed)

1. Follow `docs/01_flash_sd_card.md` → `docs/02_ethernet_to_pi.md`.
2. From the laptop, in PowerShell inside this folder:
   ```powershell
   ssh rover@rover.local "mkdir -p ~/rover"
   scp -r pi_setup ros2_ws tools rover@rover.local:~/rover/
   ssh rover@rover.local
   ```
3. On the Pi:
   ```bash
   bash ~/rover/pi_setup/bootstrap_ros2_jazzy.sh
   ```
4. Plug in the LiDAR, then follow `docs/04_lidar_bringup.md`.
