# Notes as of February 8, 2025
Read the readme below carefully as ***this project repo is not complete*** and will not result in a working product if used (yet). I've sent boards off to everyone's favorite PCB fab to be printed, and have yet to edit the models to accommodate the added sensor. The code does work and appears to be stable in "contaminated air" (read: car exhaust) testing. The Oxygen sensor portion was confirmed as accurate against a known-good sensor at my local dive shop.

# DIYNitroxSensor
This is a fork of @rolandoman's fantastic Kitable Nitrox sensor, to add a CO Alarm in the event of contaminated air supply. Given recent dive incidents, a CO Sensor add-in for about $15 (Cheaper than the O2 Sensors too) seems prudent.
Having this kitable for remote sites (bring spare sensors/parts, easy), and keep the project budget low, are the primary goals. Ideally, this entire analyzer would be sub-$50 (USD) completed.

# This is LIFE SUPPORT EQUIPMENT.
As always, once your project is complete, I would ***highly recommend*** you test your sensor against a known-good sensor, e.g. at a local dive store or nitrox fill site with a known-good analyzer. Verify everything works; _your life depends on it_. Analyze your cylinder with more than one sensor, and don't fall victim to confirmation bias (I'm expecting EAN32, my DIY sensor reads 32, perfect let's go diving). Do not dive Enriched Air Nitrox without first taking a course with your local certifying agency regarding the dangers and analysis processes.

# Sources and other information
This project borrows heavily on https://www.divetech.com/post/the-20-nitrox-analyzer (Link is now dead, I'm e-mailing the DiveTech Employee who previously wrote that article), but adds a number of improvements including rechargeable battery circuit, custom PCB, and stronger case.

This repository will mostly contain arduino code files and KiCAD files, plus edits to rolandoman's STL files to house the CO Sensor and slightly different sized PCB.

Rolandoman's main project page for the analyzer **without the CO sensor** can be found on instructables: https://www.instructables.com/Yet-Another-DIY-Nitrox-Sensor/

Rolandoman's 3D printer files, again **without the CO Sensor** can be found on Thingiverse: https://www.thingiverse.com/thing:6300317

This is a diagram of the basic connections for the arduino pro mini below, including the CO Sensor on ADC Pin A2 (only a single pin is needed for the ZE15-CO Package):
![Nitrox Board](https://github.com/user-attachments/assets/3f729ca8-4dc0-4f18-84aa-167c8f28f5d8)

Here is a picture of the specialized PCB for the project designed on KiCAD. File included in the project.
![image](https://github.com/user-attachments/assets/ee5ab325-b9c6-4f4a-8b26-5af3328c9a80)
![image](https://github.com/user-attachments/assets/ba3e40bd-ecdb-4fda-9860-f9d4f8ab2f6e)
![image](https://github.com/user-attachments/assets/ba95cf91-aa78-4642-949e-c011bcfd209b)
