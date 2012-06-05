#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <SndStream.h>
#include "FLAC/stream_decoder.h"
#include "flac.h"
#include "decoder.h"

#define RESOLUTION (256)

typedef struct {
	const FLAC__Frame *frame;
	const FLAC__int32 * const *buffer;
	int   pos;
} FLACinfo;

FLAC__StreamDecoder *decoder = NULL;
FLACinfo            finfo;
static FLAC__uint64 total_samples = 0;
static int sample_rate = 0;
static int channels    = 0;
static int bps         = 0; //bits per sample

FILE * fp;

static FLAC__StreamDecoderWriteStatus write_callback(
    const FLAC__StreamDecoder *decoder,
    const FLAC__Frame *frame,
    const FLAC__int32 * const buffer[],
    void *client_data);
static void metadata_callback(
    const FLAC__StreamDecoder *decoder,
    const FLAC__StreamMetadata *metadata,
    void *client_data);
static void error_callback(const FLAC__StreamDecoder *decoder,
                           FLAC__StreamDecoderErrorStatus status,
                           void *client_data);


int openFile(const char * name)
{
	FLAC__StreamDecoderInitStatus init_status;

	finfo.frame  = NULL;
	finfo.buffer = NULL;
	finfo.pos    = 0;

	if((decoder = FLAC__stream_decoder_new()) == NULL)
		return 0;

	init_status = FLAC__stream_decoder_init_file(
	                  decoder, name,
	                  write_callback, metadata_callback, error_callback,
	                  NULL);
	if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		return 0;

	return FLAC__stream_decoder_process_until_end_of_metadata(decoder);
}

void getFlags(int* flags)
{
	*flags = (AUDIO_16BIT | AUDIO_INTERLEAVED);
}

int getSampleRate(void)
{
	return sample_rate;
}

int getnChannels(void)
{
	return channels;
}

int seek(int pos)
{
	return 0;
}

int getPosition(void)
{
	return 0;
}

int getResolution(void)
{
	return RESOLUTION;
}

void freeDecoder(void)
{
	FLAC__stream_decoder_delete(decoder);
	decoder = NULL;
}

int decSamples(int length, short * destBuf, void * context)
{
	int decoded = 0;
	FLAC__bool success = false;
	FLAC__StreamDecoderState state;

	if(length <= 0)
		return 0;

	state = FLAC__stream_decoder_get_state(decoder);
	if(state == FLAC__STREAM_DECODER_END_OF_STREAM)
		return DEC_EOF;
	else if(state == FLAC__STREAM_DECODER_ABORTED)
		return DEC_ERR;

	while(decoded < length) {
		if(finfo.frame == NULL || finfo.pos == finfo.frame->header.blocksize) {
			success = FLAC__stream_decoder_process_single(decoder);
			if(success == false)
				return DEC_ERR;
			state = FLAC__stream_decoder_get_state(decoder);
			if(state == FLAC__STREAM_DECODER_END_OF_STREAM
			   || state == FLAC__STREAM_DECODER_ABORTED)
				return decoded;
		}

		for(;
		    decoded < length && finfo.pos < finfo.frame->header.blocksize;
		    finfo.pos++, decoded++) {
			/* write to the buffer here; convert from BE to LE */
			destBuf[decoded*2]   = finfo.buffer[0][finfo.pos];
			destBuf[decoded*2+1] = finfo.buffer[1][finfo.pos];
		}
	}
	return decoded;
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	if(total_samples == 0) {
		fprintf(stderr, "No samples to decode!\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if(channels != 2 || bps != 16) {
		fprintf(stderr, "Not stereo 16-bit!\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	finfo.frame  = frame;
	finfo.buffer = buffer;
	finfo.pos    = 0;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		/* save for later */
		total_samples = metadata->data.stream_info.total_samples;
		sample_rate   = metadata->data.stream_info.sample_rate;
		channels      = metadata->data.stream_info.channels;
		bps           = metadata->data.stream_info.bits_per_sample;
	}
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	char *err;
	switch(status) {
	case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
		err = "Lost sync";
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
		err = "Bad header";
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
		err = "Frame CRC mismatch";
		break;
	case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
		err = "Unparseable stream";
		break;
	default:
		err = "???";
	}

	fprintf(stderr, "error: %s\n", err);
}
