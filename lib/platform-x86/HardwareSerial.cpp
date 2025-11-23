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

HardwareSerial Serial(1);
HardwareSerial Serial2(2);
HardwareSerial Serial3(3);
HardwareSerial Serial4(4);

#include <HardwareSerial.h>


HardwareSerial::HardwareSerial(int id) {
  this->id = id;
}

void HardwareSerial::begin(unsigned long baud) {
  printf("HardwareSerial(%d)::begin(%ld)\n", this->id, baud);
}

void HardwareSerial::begin(unsigned long baud, byte config)
{
  printf("HardwareSerial(%d)::begin(%ld)\n", this->id, baud);
}

void HardwareSerial::end()
{
  printf("HardwareSerial(%d)::end()\n", this->id);
}

int HardwareSerial::available(void)
{
    printf("HardwareSerial(%d)::available()\n", this->id);
    return 0;
}

int HardwareSerial::peek(void)
{
    printf("HardwareSerial(%d)::peek()\n", this->id);
    return 0;
}

int HardwareSerial::read(void)
{
  printf("HardwareSerial(%d)::read()\n", this->id);
  return 0;
}

int HardwareSerial::availableForWrite(void)
{
  printf("HardwareSerial(%d)::availableForWrite(void)\n", this->id);
  return 0;
}

void HardwareSerial::flush()
{
  printf("HardwareSerial(%d)::flush()\n", this->id);
}

size_t HardwareSerial::write(uint8_t c)
{
  printf("HardwareSerial(%d)::write(%c)\n", this->id, c);
  return 0;
}
