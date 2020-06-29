# MQTTNMEADisplay
A wireless display of NMEA 0183 Data for a boat, using a M5Stack Core.

This is designed to be used with OpenPlotter, but should work with any system that outputs NMEA0183 strings via WiFi. 

It makes use of the excellent M5ez library, and connects to OpenPlotter via Node-Red. It also needs a couple of other libraries, as can be seen from the #includes.

I am no longer working on this project. I have ported this code to a new display that uses SignalK. https://github.com/andyrbarrow/SignalKM5StackInstrument
