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

// current payload: 40 Bit, 5 Bytes

// structured block to group cmd's together
struct cmd_payload {    //## const?,  include into CMD?
  uint16_t from_node;    //added for answer (apart from ACK), octal format
  uint16_t to_node;    //already defined in header for transmission but redefine here ## volatile problem
  uint8_t category;
  uint8_t function;
  uint8_t parameter;
};


// enum class to use namespace, prevents duplicates
class CMD{    // ### still not perfect, crash on duplicates
private:
  // nothing needed, just container for commands
public:
  // define your module address here, octal!!
  enum NODE : uint16_t {  // receiver only analyses BROADCAST or own name
    BROADCAST = START,  //enables the use of global functions like network reboot
    PC_NODE = 02,
    TESTER = 00,
    RS,     //radio socket
    TV,
    RGB
  };

  // ### not DRY, names of enmus below
  enum CATEGORY : uint8_t {
    GENERIC = START,  // receiver sends package with state ()
    PC_CTRL   //with ON or OFF
  };

  // ----------- Category with Functions:

  enum GENERIC{
    GET_STATUS = START,     //return what you want
    GET_CHARGE,             //return current carge of battery
    SET_TIME,               //with ON or OFF
    RESTART,
    SET_LED,                // 1 or 0
    BLINK                   // x times
  };

  enum PC_CTRL{
    GET = START,  // receiver sends package with state ()
    SET   //with ON or OFF
  };

  // ### error with duplicates, scoped scoped enum?
  /*enum RGB{
    GET = START,  // receiver sends package with state ()
    SET   //with ON or OFF
  };*/


};















#endif /* end of include guard:  */
