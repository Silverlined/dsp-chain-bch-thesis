/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2018, Libre Space Foundation <http://librespacefoundation.org/>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <satnogs/shift_reg.h>

namespace gr {
namespace satnogs {

/**
 * Creates a new shift register
 * @param len the number of the memory stages
 */
shift_reg::shift_reg(size_t len)
  : d_len(len),
    d_reg(len, 0)
{
}

shift_reg::~shift_reg()
{
}

/**
 * Sets all the memory stages to 0
 */
void
shift_reg::reset()
{
  for (size_t i = 0; i < d_len; i++) {
    d_reg[i] = 0;
  }
}

/**
 * Sets all the memory stages to 1
 */
void
shift_reg::set()
{
  for (size_t i = 0; i < d_len; i++) {
    d_reg[i] = 1;
  }
}

/**
 *
 * @return the number of the memory stages of the shift register
 */
size_t
shift_reg::len() const
{
  return d_len;
}

/**
 *
 * @return the number of the memory stages of the shift register
 */
size_t
shift_reg::size() const
{
  return d_len;
}

/**
 *
 * @return the number of 1 bits
 */
size_t
shift_reg::count()
{
  size_t cnt = 0;
  for (bool i : d_reg) {
    cnt += i;
  }
  return cnt;
}

shift_reg
shift_reg::operator | (const shift_reg &rhs)
{
  shift_reg ret(d_len);
  for (size_t i = 0; i < d_len; i++) {
    ret[i] = d_reg[i] | rhs[i];
  }
  return ret;
}

shift_reg
shift_reg::operator & (const shift_reg &rhs)
{
  shift_reg ret(d_len);
  for (size_t i = 0; i < d_len; i++) {
    ret[i] = d_reg[i] & rhs[i];
  }
  return ret;
}

shift_reg
shift_reg::operator ^ (const shift_reg &rhs)
{
  shift_reg ret(d_len);
  for (size_t i = 0; i < d_len; i++) {
    ret[i] = d_reg[i] ^ rhs[i];
  }
  return ret;
}

shift_reg &
shift_reg::operator >>= (bool bit)
{
  push_front(bit);
  return *this;
}

bool &
shift_reg::operator [](size_t pos)
{
  return d_reg[pos];
}

bool
shift_reg::operator[](size_t pos) const
{
  return d_reg[pos];
}

shift_reg &
shift_reg::operator <<= (bool bit)
{
  push_back(bit);
  return *this;
}

/**
 * Push at the front a new value and pops from the back
 * @param bit the new value
 */
void
shift_reg::push_front(bool bit)
{
  d_reg.pop_back();
  d_reg.push_front(bit);
}

/**
 * Push at the back a new value and pops from the front
 * @param bit the new value
 */
void
shift_reg::push_back(bool bit)
{
  d_reg.pop_front();
  d_reg.push_back(bit);
}

/**
 *
 * @return the first element in the queue from right to left
 */
bool
shift_reg::front()
{
  return d_reg.front();
}

/**
 *
 * @return the last element in the queue from right to left
 */
bool
shift_reg::back()
{
  return d_reg.back();
}

std::ostream &
operator<<(std::ostream &os, const shift_reg &reg)
{
  for (bool bit : reg.d_reg) {
    os << " " << bit;
  }
  return os;
}

} /* namespace satnogs */
} /* namespace gr */

