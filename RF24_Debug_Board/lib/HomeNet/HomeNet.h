#ifndef CODES_H
#define CODES_H

#include <Arduino.h>
#include <stdint.h>
//...

/* Dictionary for en-/decoding of messages

send (receiver, device, function, parameter)

## pretty much a copy of IP?

Node  | Category  | Function  | Parameter
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
const uint8_t rows_dict = 32;

// enum class to use namespace, prevents duplicates
class HomeNet{    // ### still not perfect, crash on duplicates
private:
  typedef void (*f_node)(uint8_t);

  f_node *dict_f[rows_dict];  // array of functions
  uint8_t dict[rows_dict][2];   // array to map functions to
  uint8_t counter = 0;

public:
  // current payload: 40 Bit, 5 Bytes
  // structured block to group cmd's together
  struct payload {    //## const?,  include into CMD?
    uint16_t from_node;    //added for answer (apart from ACK), octal format
    uint16_t to_node;    //already defined in header for transmission but redefine here ## volatile problem
    uint8_t category;
    uint8_t function;
    uint8_t parameter;
  };

  static void print_payload(payload &pl);

  // could use enums as parameters, problem with "function" enum definition
  bool add(uint8_t category, uint8_t function, f_node p_function);
  void translate(payload &pl);                // calls function

  // sort?? -> sort numerically by category

  // define your module address here, octal!!
  enum class NODE : uint16_t {  // receiver only analyses BROADCAST or own name
    BROADCAST = START,  //enables the use of global functions like network reboot
    PC_NODE = 02,
    TESTER = 00,
    RS,     //radio socket
    TV,
    RGB
  };

  // ### not DRY, names of enmus below
  enum class CATEGORY : uint8_t {
    GENERIC = START,  // receiver sends package with state ()
    PC_CTRL   //with ON or OFF
  };

  // ----------- Category with Functions:

  enum class GENERIC : uint8_t{
    GET_STATUS = START,     //return what you want
    GET_NAME,               // format to send?
    GET_CHARGE,             //return current carge of battery
    SET_TIME,               //with ON or OFF
    RESTART,
    SET_LED,                // 1 or 0
    BLINK                   // x times
  };

  enum class PC_CTRL : uint8_t{
    GET = START,  // receiver sends package with state ()
    SET   //with duration, 1 -> 100ms, 255 -> 25s
  };

  // ### error with duplicates, scoped scoped enum?
  /*enum RGB{
    GET = START,  // receiver sends package with state ()
    SET   //with ON or OFF
  };*/


};















#endif /* end of include guard:  */
