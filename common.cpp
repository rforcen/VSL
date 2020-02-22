#include "common.h"

Common *c = nullptr;  // init in main window creator or audio will not work on
                      // static and general deploy scenarios
bool Common::processing = false;
