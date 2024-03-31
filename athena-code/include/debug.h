#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG

#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x, y) Serial.printf(x, y)

#endif

#endif