/* ------------------------------------------------------------------
 * Copyright (C) 2009 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "wavwriter.h"
#include <interf_dec.h>

#ifdef AMRNB_TINY
enum Frame_Type_3GPP
{
	AMR_475 = 0,        /* 4.75 kbps    */
	AMR_515,            /* 5.15 kbps    */
	AMR_59,             /* 5.9 kbps     */
	AMR_67,             /* 6.7 kbps     */
	AMR_74,             /* 7.4 kbps     */
	AMR_795,            /* 7.95 kbps    */
	AMR_102,            /* 10.2 kbps    */
	AMR_122,            /* 12.2 kbps    */
	AMR_SID,            /* GSM AMR DTX  */
	GSM_EFR_SID,        /* GSM EFR DTX  */
	TDMA_EFR_SID,       /* TDMA EFR DTX */
	PDC_EFR_SID,        /* PDC EFR DTX  */
	FOR_FUTURE_USE1,    /* Unused 1     */
	FOR_FUTURE_USE2,    /* Unused 2     */
	FOR_FUTURE_USE3,    /* Unused 3     */
	AMR_NO_DATA         /* No data      */
};
#endif

/* From WmfDecBytesPerFrame in dec_input_format_tab.cpp */
const int sizes[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0 };


int main(int argc, char *argv[]) {
	FILE* in;
	char header[6];
	int n;
	void *wav, *amr;
	if (argc < 3) {
		fprintf(stderr, "%s in.amr out.wav\n", argv[0]);
		return 1;
	}

	in = fopen(argv[1], "rb");
	if (!in) {
		perror(argv[1]);
		return 1;
	}
	n = fread(header, 1, 6, in);
	if (n != 6 || memcmp(header, "#!AMR\n", 6)) {
		fprintf(stderr, "Bad header\n");
		return 1;
	}

	wav = wav_write_open(argv[2], 8000, 16, 1);
	if (!wav) {
		fprintf(stderr, "Unable to open %s\n", argv[2]);
		return 1;
	}

	amr = Decoder_Interface_init();
	while (1) {
		uint8_t buffer[500], littleendian[320], *ptr;
		int size, i;
		int16_t outbuffer[160];
		/* Read the mode byte */
		n = fread(buffer, 1, 1, in);
		if (n <= 0)
			break;
#ifdef AMRNB_TINY
		/* Check the mode equals AMR 122 */
		if (((buffer[0] >> 3) & 0x0f) != AMR_122) {
			fprintf(stderr, "Not supported mode %02X \n", buffer[0]);
			break;
		}
#endif /* AMRNB_TINY */

		/* Find the packet size */
		size = sizes[(buffer[0] >> 3) & 0x0f];
		n = fread(buffer + 1, 1, size, in);
		if (n != size)
			break;

		/* Decode the packet */
		Decoder_Interface_Decode(amr, buffer, outbuffer, 0);

		/* Convert to little endian and write to wav */
		ptr = littleendian;
		for (i = 0; i < 160; i++) {
			*ptr++ = (outbuffer[i] >> 0) & 0xff;
			*ptr++ = (outbuffer[i] >> 8) & 0xff;
		}
		wav_write_data(wav, littleendian, 320);
	}
	fclose(in);
	Decoder_Interface_exit(amr);
	wav_write_close(wav);
	return 0;
}

