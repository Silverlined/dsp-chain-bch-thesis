/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, Libre Space Foundation <http://libre.space>
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

#ifndef INCLUDED_SATNOGS_MOVING_SUM_H
#define INCLUDED_SATNOGS_MOVING_SUM_H

#include <satnogs/api.h>
#include <deque>

namespace gr {
namespace satnogs {

/*!
 * \brief Simple moving sum template using std::deque
 *
 */
template<typename T>
class SATNOGS_API moving_sum {
public:
  moving_sum(int len, T init_val) :
    d_val(len * init_val),
    d_buf(len, init_val)
  {
  }

  T
  insert(T newval);

  T
  val();

private:
  T                     d_val;
  std::deque<T>         d_buf;
};

template<class T> T
moving_sum<T>::insert(T newval)
{
  T old = d_buf.front();
  d_buf.pop_front();
  d_buf.push_back(newval);
  d_val += newval;
  d_val -= old;
  return d_val;
}

template <class T> T
moving_sum<T>::val()
{
  return d_val;
}

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_MOVING_SUM_H */
