#include "HomeNet.h"


void HomeNet::print_payload(HomeNet::payload &pl){
  Serial.println(F("-- Payload is: --"));
  Serial.print(F("\n From Node: "));
  Serial.print(pl.from_node);
  Serial.print(F("\n To Node: "));
  Serial.print(pl.to_node);
  Serial.print(F("\n category: "));
  Serial.print(pl.category);
  Serial.print(F("\n function: "));
  Serial.print(pl.function);
  Serial.print(F("\n parameter: "));
  Serial.print(pl.parameter);
  Serial.println(F("\n-----------------"));
}

bool HomeNet::add(uint8_t category, uint8_t function, f_node *p_function){
  if (counter >= rows_dict) return false;

  dict[counter][0] = category;
  dict[counter][1] = function;

  dict_f[counter] = p_function;

  counter++;

  return true;
}

void HomeNet::translate(payload &pl){
  if (counter == 0) {
    //no entries yet
    return;
  }

  // find category and function
  uint8_t index = 0;
  for (uint8_t i = 0; i <= counter; i++){
      if ((pl.category == dict[i][0]) && (pl.function == dict[i][1])){
        index = i;
      }
  }

  // access index in function list & call
  (*dict_f[index])(pl.parameter);

}
