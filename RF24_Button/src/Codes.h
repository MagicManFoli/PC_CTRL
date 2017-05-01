#ifndef CODES_H
#define CODES_H

#include <Arduino.h>
#include <stdint.h>
//...

/* Dictionary for en-/decoding of messages

send (receiver, device, function, parameter)

## pretty much a copy of IP?

Node  | Category  | Funktion  | Parameter
0 - 7 | 8 - 15    | 16 - 23   | 24 - 31

 * optional *
+ transmitter (for feedback)
+ length of content (eases variable payload)
+ additional space for parameters (general data transmition)
+ package number (for comparison)
*/

/* -- examples -- *\
- BROADCAST | GENERIC | get_alive | ?
- PC_Node | PC_CTRL | SET | ON
-

\* -------------- */

// start codes on printable chars to increase readability and enable manual insertion
#define START 65

// structured block to group cmd's together
struct cmd {
  uint8_t node;
  uint8_t category;
  uint8_t function;
  uint8_t parameter;
};

// enum class to use namespace, prevents duplicates

enum class NODE{  // receiver only analyses BROADCAST or own name
  BROADCAST = START,  //enables the use of global functions like network reboot
  PC_NODE,
  RS,     //radio socket
  TV,
  RGB
};

// ----------- Category with Functions:

enum class GENERIC{
  GET_STATUS = START,     //return what you want
  GET_CHARGE,             //return current carge of battery
  SET_TIME,               //with ON or OFF
  RESTART,
  BLINK                   // x times
};

enum class PC_CTRL{
  GET = START,  // receiver sends package with state ()
  SET   //with ON or OFF
};






#endif /* end of include guard:  */
