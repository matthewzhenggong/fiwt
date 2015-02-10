
XBee Zigbee API Test Programme
==============================

Screenshots
------------------
* Range Test
![ScreenShot of Range Test](https://raw.github.com/matthewzhenggong/fiwt/master/XbeeZBS2Test/screenshot_range_test.png)

    COM6 connected to a XBee router and COM3 connected to a coordinator. Router is sending ping packs to Coordinator in 50Hz.
    The ping pack is in the format of 'PXXXXXyyyyyyy', in which XXXXX means 5-digital index number and yyyyyyy is the input content. 

    In the left picture, payload  of each pack is 31 bytes with 28-bytes frame data. So the baudrate of payload is 15510bps and the rate of xbee serial is 29510(must be less than 115200). 
    In the ping information line, the format is [good pack]/[lost pack]/[dup pack] C[total pack]/T[time in secs] [packets per secs]Pps/[bits per secs]bps.
    
    The log window shows detailed message and error-trackback informations .

Funtion support
----------------
- AT command
- Remote AT command
- Send single TX request with response in const frequency
- Send continuous TX requests with/without response in const frequency
- Flow rate predict/measurement in Pps(Packet per Second) and bps(Bit per Second)
- Echo RX response for range test(missing function of X-CTU in API mode)

Programme based on
------------------
* [wxPython](http://www.wxpython.org)
* [pySerial](http://pyserial.sourceforge.net)
* [python-xbee](http://code.google.com/p/python-xbee/)


