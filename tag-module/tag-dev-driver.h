#include "module.h"


#define LINE_SIZE 53                                // Size of a line in Char Device 
#define TAG_SIZE ((LINE_SIZE + 1) * (LEVELS + 1))   // Size of a a complete Tag in Char Device

void register_chardev(void);

void unregister_chardev(void);