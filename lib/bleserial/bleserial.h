#ifndef BLESERIAL_H
#define BLESERIAL_H

#include <Arduino.h>

// Log is the project's output: USB serial always, plus a ring buffer the BLE link drains when on.
// Replaces Serial.print/println/printf in project code. Serial.begin/available/read are unchanged.
class BleTee : public Print {
 public:
  size_t write(uint8_t c) override;
  size_t write(const uint8_t* buf, size_t n) override;
  size_t printf(const char* fmt, ...);

  void setEnabled(bool on);  // BLE copy on or off; USB output is unchanged
  bool enabled() const;

  // Used by the BLE sender
  size_t available() const;               // bytes waiting in the ring
  size_t take(uint8_t* out, size_t max);  // move up to max bytes out of the ring
  uint32_t takeDropped();                 // bytes lost to a full ring since last call
};

extern BleTee Log;

// BLE serial link (implemented in bleserial.cpp)
void bleSerialStart();          // first-time init; safe to call repeatedly
void bleSerialApply(bool on);   // turn the link on or off now
void bleSerialPump();          // call from loop(): sends queued output

bool bleSerialIsRunning();  // true once bleSerialStart() has brought BLE up
#endif  // BLESERIAL_H
