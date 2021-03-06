/*
 This file is part of TinyGarble. It is modified version of JustGarble
 under GNU license.

 TinyGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TinyGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TinyGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_

#include "crypto/block.h"
#include <openssl/bn.h>
#include <ostream>
#include <string>

using std::string;

block RandomBlock();
void SrandSSE(unsigned int seed);
unsigned short Type2V(int gateType);
bool GateOperator(int gateType, bool input0, bool input1 = false);
int Str2Block(const string &s, block* v);
string to_string_hex(uint64_t v, int pad = 0);
string OutputBN2Str(BIGNUM* outputs, uint64_t clock_cycles,
                    uint64_t output_size, int output_mode);

#endif /* UTIL_UTIL_H_ */
