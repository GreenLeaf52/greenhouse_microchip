// devices_state.h
#ifndef DEVICES_STATE_H
#define DEVICES_STATE_H

struct DevicesState {
  bool heat;
  bool vent; 
  bool pH; 
  bool ec;
  bool light; 
  bool watering;
  int code;
};

#endif  // DEVICES_STATE_H