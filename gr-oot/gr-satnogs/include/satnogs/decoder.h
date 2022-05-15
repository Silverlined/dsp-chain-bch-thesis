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

#ifndef INCLUDED_SATNOGS_DECODER_H
#define INCLUDED_SATNOGS_DECODER_H

#include <satnogs/api.h>
#include <cstdint>
#include <cstdlib>
#include <pmt/pmt.h>

namespace gr {
namespace satnogs {

/**
 * The status of the decoder
 *
 * This class contains all the necessary information that the
 * \ref decoder::decode() method returns and used by the frame_decoder()
 * to properly inform the GNU Radio scheduler and/or propagate decoded frames
 */
class decoder_status {
public:
  int                   consumed;       /**< The number of input items consumed */
  bool
  decode_success; /**< Indicated if there was a successful decoding */
  pmt::pmt_t
  data;           /**< a dictionary with the PDU with of decoded data and the corresponding metadata for the decoded frame */

  decoder_status() :
    consumed(0),
    decode_success(false),
    data(pmt::make_dict())
  {
  }
};

typedef class decoder_status decoder_status_t;


/**
 * \brief Abstract class that provided the API for the c decoders
 *
 * This is an abstract class providing the API for the SatNOGS decoders.
 *
 * The gr-satnogs module tries to provide a unified decoding framework,
 * for various satellites.
 * Specialization is performed by passing to the generic decoding block the
 * appropriate decoder class that implements this abstract class API.
 *
 */
class SATNOGS_API decoder {
public:
  typedef boost::shared_ptr<decoder> decoder_sptr;

  static int base_unique_id;

  int
  unique_id() const;

  decoder(const std::string &name, const std::string &version,
          int input_item_size, size_t max_frame_len = 8192);
  virtual ~decoder();

  /**
   * Decodes a buffer of input items contained in the in buffer.
   * This method is called continuously by the frame_decoder.
   * Based on the returned status data, the frame_decoder() instructs
   * properly the GNU Radio scheduler and/or propagates decoded data.
   *
   * As the number of input items may not enough to decode a frame, each decoder
   * should keep internal state, so decoding can be accomplished after an
   * arbitrary number of calls to this method
   *
   * @param in the input items
   *
   * @param nitems the number of input items contained in the in buffer
   *
   * @return the status of the decoder after the call of this method. For
   * more information refer to decoder_status()
   */
  virtual decoder_status_t
  decode(const void *in, int nitems) = 0;


  /**
   * Resets the internal state of the decoder to the initial defaults
   *
   */
  virtual void
  reset() = 0;

  virtual size_t
  input_multiple() const;

  size_t
  max_frame_len() const;

  int
  sizeof_input_item() const;

  std::string
  name() const;

  std::string
  version() const;

protected:
  void
  incr_nitems_read(size_t nitems);

  uint64_t
  nitems_read() const;

private:
  const std::string     d_name;
  const std::string     d_version;
  const int             d_sizeof_in;
  const size_t          d_max_frame_len;
  const int             d_id;
  uint64_t              d_nitems_read;
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_DECODER_H */

