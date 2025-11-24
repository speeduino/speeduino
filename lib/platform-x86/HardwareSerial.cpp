/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 3 December 2013 by Matthijs Kooijman
*/

#include <cinttypes>
#include <Arduino.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#else
#error POSIX operating systems supported only (Linux/Mac) for now.
#endif

HardwareSerial Serial(1);
HardwareSerial Serial2(2);
HardwareSerial Serial3(3);
HardwareSerial Serial4(4);

#include <HardwareSerial.h>

HardwareSerial::HardwareSerial(int id) {
  this->id = id;
}

void HardwareSerial::begin(unsigned long baud) {

  master_fd = posix_openpt(O_RDWR | O_NOCTTY);
  grantpt(master_fd);
  unlockpt(master_fd);
  char *slave_name = ptsname(master_fd);

  int flags = fcntl(master_fd, F_GETFL, 0);

  if (fcntl(master_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    log(HARDWARE_SERIAL, "fcntl(F_SETFL | O_NONBLOCK) failed\n");
    exit(1);
  }

  log(HARDWARE_SERIAL, "HardwareSerial slave device: %s\n", slave_name);

}

void HardwareSerial::begin(unsigned long baud, byte config)
{
  this->begin(baud);
}

void HardwareSerial::end()
{
  log(HARDWARE_SERIAL, "HardwareSerial(%d)::end()\n", this->id);
  close(master_fd);
}

int HardwareSerial::available(void)
{

  if (this->idx != 0) {
    // there is a buffer, empty it
    return idx;
  }

  memset(&buffer, 0, sizeof(buffer));

  int read = ::read(master_fd, &buffer, sizeof(buffer));

  if (read > 0) {
    this->out_idx = 0;
    this->idx = read;
    return read;
  }

  return 0;
}

int HardwareSerial::peek(void)
{
    return this->buffer[out_idx];
}

int HardwareSerial::read(void)
{
  uint8_t out = buffer[out_idx];
  idx--;
  out_idx++;
  return out;
}

int HardwareSerial::availableForWrite(void)
{
  return this->idx == 0;
}

void HardwareSerial::flush()
{
  log(HARDWARE_SERIAL, "HardwareSerial(%d)::flush()\n", this->id);
  tcdrain(master_fd);
}

size_t HardwareSerial::write(uint8_t c)
{
  log(HARDWARE_SERIAL, "HardwareSerial(%d)::write(%c)\n", this->id, c);
  int out = ::write(master_fd, &c, 1);
  if (out < 0) {
    log(HARDWARE_SERIAL, "HardwareSerial: write failed!");
  }
  return out;
}
