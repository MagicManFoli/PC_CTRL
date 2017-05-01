/*

Dictionary for en-/decoding of messages


send (receiver, device, function, parameter)

## pretty much a copy of IP?

Receiver  | Device  | Funktion  | Parameter
0 - 7     | 8 - 15  | 16 - 23   | 24 - 31

 * optional *
+ transmitter (for feedback)
+ length of content (eases variable payload)
+ additional space for parameters (general data transmition)
+ package number (for comparison)


enum class to use namespace, prevents duplicates
*/

// start codes on printable chars to increase readability and enable manual insertion
#define START 65

enum class receiver{  // receiver only analyses BROADCAST or own name
  BROADCAST = START,  //enables the use of global functions like network reboot
  PC_CTRL,
  RS,     //radio socket
  TV,
  RGB
};

enum class PC_CTRL{
  GET = START,  // receiver sends package with state ()
  //## decision: SET or (ON/OFF)
  SET,   //with 1 or 0
  ON,
  OFF
};

enum class FS{
  OFF = START,
  RS,     //radio socket
  TV,
  RGB
};
