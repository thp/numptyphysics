#include "Colour.h"

namespace NP {
namespace Colour {

const int values[] = {
    RED,
    YELLOW,
    DEFAULT,
  0x108710, //green
  0x101010, //black
  0x8b4513, //brown
  0x87cefa, //lightblue
  0xee6aa7, //pink
  0xb23aee, //purple
  0x00fa9a, //lightgreen
  0xff7f00, //orange
  0x6c7b8b, //grey
};

const int count = (sizeof(values)/sizeof(values[0]));

}; /* Colour */
}; /* NP */
