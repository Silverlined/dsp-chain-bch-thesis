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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <satnogs/decoder.h>

namespace gr {
namespace satnogs {

int decoder::base_unique_id = 1;

/**
 * @note: For internal GNU Radio companion usage. DO NOT EDIT!
 * @return the unique id of the decoder object
 */
int
decoder::unique_id() const
{
  return d_id;
}

/**
 * @brief Construct a new decoder::decoder object
 *
 * @param name the name of the decoder
 * @param version the version of the decoder. The version string
 * should conform with the Semantic Versioning 2.0.0 scheme (https://semver.org/)
 * @param input_item_size the sizeof() the input stream item
 * @param max_frame_len the maximum allowed frame size in bytes
 */
decoder::decoder(const std::string &name, const std::string &version,
                 int input_item_size, size_t max_frame_len)
  : d_name(name),
    d_version(version),
    d_sizeof_in(input_item_size),
    d_max_frame_len(max_frame_len),
    d_id(base_unique_id++),
    d_nitems_read(0)
{
}

decoder::~decoder()
{
}

/**
 *
 * @return the name of the decoder
 */
std::string
decoder::name() const
{
  return d_name;
}

/**
 *
 * @return the version of the decoder. The string return conforms with the
 * Semantic Versioning 2.0.0 scheme
 */
std::string
decoder::version() const
{
  return d_version;
}

/**
 * By default, the number of input items can be arbitrary (multiple of 1).
 * Decoders that require the number of input items to be a multiple of a
 * specific number, should override this method.
 *
 * The frame_decoder block, will use the number returned from this method
 * to properly alter the behavior of the GNU Radio scheduler
 *
 * @return the number that input items should be multiple
 */
size_t
decoder::input_multiple() const
{
  return 1;
}

/**
 *
 * @return the maximum allowed frame length
 */
size_t
decoder::max_frame_len() const
{
  return d_max_frame_len;
}

/**
 * @brief Return the size of the input stream
 *
 * @return int the sizeof() the input stream item
*/
int
decoder::sizeof_input_item() const
{
  return d_sizeof_in;
}


/**
 * @brief Increaments the number of items read so far
 *
 * @param nitems the number of items read
 */
void
decoder::incr_nitems_read(size_t nitems)
{
  d_nitems_read += nitems;
}

/**
 * @brief Return the number of items read so far
 *
 * @return uint64_t number of items read so far
 */
uint64_t
decoder::nitems_read() const
{
  return d_nitems_read;
}


} /* namespace satnogs */
} /* namespace gr */

