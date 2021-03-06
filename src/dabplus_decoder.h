/*
    DABlin - capital DAB experience
    Copyright (C) 2015 Stefan Pöschel

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DABPLUS_DECODER_H_
#define DABPLUS_DECODER_H_

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <sstream>

#define DABLIN_AAC_FAAD2

#if !(defined(DABLIN_AAC_FAAD2) ^ defined(DABLIN_AAC_FDKAAC))
#error "You must select a AAC decoder by defining either DABLIN_AAC_FAAD2 or DABLIN_AAC_FDKAAC!"
#endif

#ifdef DABLIN_AAC_FAAD2
#include <neaacdec.h>
#endif

#ifdef DABLIN_AAC_FDKAAC
#include <fdk-aac/aacdecoder_lib.h>
#endif

extern "C" {
#include <fec.h>
}

#include "subchannel_sink.h"
#include "tools.h"


struct SuperframeFormat {
	bool dac_rate;
	bool sbr_flag;
	bool aac_channel_mode;
	bool ps_flag;
	int mpeg_surround_config;
};


// --- RSDecoder -----------------------------------------------------------------
class RSDecoder {
private:
	void *rs_handle;
	uint8_t rs_packet[120];
	int corr_pos[10];
public:
	RSDecoder();
	~RSDecoder();

	void DecodeSuperframe(uint8_t *sf, size_t sf_len);
};



// --- AACDecoder -----------------------------------------------------------------
class AACDecoder {
protected:
	SubchannelSinkObserver* observer;
	uint8_t asc[2];

	static int GetAACChannelConfiguration(SuperframeFormat sf_format);
	void CheckForPAD(const uint8_t *data, size_t len);
public:
	AACDecoder(std::string decoder_name, SubchannelSinkObserver* observer, SuperframeFormat sf_format);
	virtual ~AACDecoder() {}

	virtual void DecodeFrame(uint8_t *data, size_t len) = 0;
};


#ifdef DABLIN_AAC_FAAD2
// --- AACDecoderFAAD2 -----------------------------------------------------------------
class AACDecoderFAAD2 : public AACDecoder {
private:
	NeAACDecHandle handle;
	NeAACDecFrameInfo dec_frameinfo;
public:
	AACDecoderFAAD2(SubchannelSinkObserver* observer, SuperframeFormat sf_format);
	~AACDecoderFAAD2();

	void DecodeFrame(uint8_t *data, size_t len);
};
#endif


#ifdef DABLIN_AAC_FDKAAC
// --- AACDecoderFDKAAC -----------------------------------------------------------------
class AACDecoderFDKAAC : public AACDecoder {
private:
	HANDLE_AACDECODER handle;
	uint8_t *output_frame;
	size_t output_frame_len;
public:
	AACDecoderFDKAAC(SubchannelSinkObserver* observer, SuperframeFormat sf_format);
	~AACDecoderFDKAAC();

	void DecodeFrame(uint8_t *data, size_t len);
};
#endif


// --- SuperframeFilter -----------------------------------------------------------------
class SuperframeFilter : public SubchannelSink {
private:
	RSDecoder rs_dec;
	AACDecoder *aac_dec;

	size_t frame_len;
	int frame_count;
	int sync_frames;

	uint8_t *sf_raw;
	uint8_t *sf;
	size_t sf_len;

	bool sf_format_set;
	uint8_t sf_format_raw;
	SuperframeFormat sf_format;

	int num_aus;
	int au_start[6+1]; // +1 for end of last AU

	bool CheckSync();
	void ProcessFormat();
	void CheckForPAD(const uint8_t *data, size_t len);
public:
	SuperframeFilter(SubchannelSinkObserver* observer);
	~SuperframeFilter();

	void Feed(const uint8_t *data, size_t len);
};



#endif /* DABPLUS_DECODER_H_ */
