#ifndef __BITS_H__
#define __BITS_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdint.h>
#define  INLINE inline


#define BYTE_NUMBIT     8
#define BYTE_NUMBIT_LD  3
#define bit2byte(a) ((a+7)>>BYTE_NUMBIT_LD)

	typedef struct _bitfile
	{
		/* bit input */
		uint32_t bufa;
		uint32_t bufb;
		uint32_t bits_left;
		uint32_t buffer_size; /* size of the buffer in bytes */
		uint32_t bytes_left;
		uint8_t error;
		uint32_t *tail;
		uint32_t *start;
		const void *buffer;
	} bitfile;


	void bitbuffer_initbits(bitfile *ld, const void *buffer, const uint32_t buffer_size);
	void bitbuffer_endbits(bitfile *ld);
	void bitbuffer_initbits_rev(bitfile *ld, void *buffer,
		uint32_t bits_in_buffer);
	uint8_t bitbuffer_byte_align(bitfile *ld);
	uint32_t bitbuffer_get_processed_bits(bitfile *ld);
	void bitbuffer_flushbits_ex(bitfile *ld, uint32_t bits);
	void bitbuffer_rewindbits(bitfile *ld);
	void bitbuffer_resetbits(bitfile *ld, int bits);
	uint8_t *bitbuffer_getbitbuffer(bitfile *ld, uint32_t bits);


	/* circumvent memory alignment errors on ARM */
	static  uint32_t getdword(void *mem)
	{
		uint32_t tmp;
#ifndef ARCH_IS_BIG_ENDIAN
		((uint8_t*)&tmp)[0] = ((uint8_t*)mem)[3];
		((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[2];
		((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[1];
		((uint8_t*)&tmp)[3] = ((uint8_t*)mem)[0];
#else
		((uint8_t*)&tmp)[0] = ((uint8_t*)mem)[0];
		((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[1];
		((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[2];
		((uint8_t*)&tmp)[3] = ((uint8_t*)mem)[3];
#endif

		return tmp;
	}

	/* reads only n bytes from the stream instead of the standard 4 */
	static /*INLINE*/ uint32_t getdword_n(void *mem, int n)
	{
		uint32_t tmp = 0;
#ifndef ARCH_IS_BIG_ENDIAN
		switch (n)
		{
		case 3:
			((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[2];
		case 2:
			((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[1];
		case 1:
			((uint8_t*)&tmp)[3] = ((uint8_t*)mem)[0];
		default:
			break;
		}
#else
		switch (n)
		{
		case 3:
			((uint8_t*)&tmp)[2] = ((uint8_t*)mem)[2];
		case 2:
			((uint8_t*)&tmp)[1] = ((uint8_t*)mem)[1];
		case 1:
			((uint8_t*)&tmp)[0] = ((uint8_t*)mem)[0];
		default:
			break;
		}
#endif

		return tmp;
	}

	static  uint32_t bitbuffer_showbits(bitfile *ld, uint32_t bits)
	{
		if (bits <= ld->bits_left)
		{
			//return (ld->bufa >> (ld->bits_left - bits)) & bitmask[bits];
			return (ld->bufa << (32 - ld->bits_left)) >> (32 - bits);
		}

		bits -= ld->bits_left;
		//return ((ld->bufa & bitmask[ld->bits_left]) << bits) | (ld->bufb >> (32 - bits));
		return ((ld->bufa & ((1 << ld->bits_left) - 1)) << bits) | (ld->bufb >> (32 - bits));
	}

	static void bitbuffer_flushbits(bitfile *ld, uint32_t bits)
	{
		/* do nothing if error */
		if (ld->error != 0)
			return;

		if (bits < ld->bits_left)
		{
			ld->bits_left -= bits;
		}
		else {
			bitbuffer_flushbits_ex(ld, bits);
		}
	}

	/* return next n bits (right adjusted) */
	static /*INLINE*/ uint32_t bitbuffer_getbits(bitfile *ld, uint32_t n)
	{
		uint32_t ret;

		if (n == 0)
			return 0;

		ret = bitbuffer_showbits(ld, n);
		bitbuffer_flushbits(ld, n);

		return ret;
	}

	static  uint8_t bitbuffer_get1bit(bitfile *ld)
	{
		uint8_t r;

		if (ld->bits_left > 0)
		{
			ld->bits_left--;
			r = (uint8_t)((ld->bufa >> ld->bits_left) & 1);
			return r;
		}

		r = (uint8_t)bitbuffer_getbits(ld, 1);
		return r;
	}

	/* reversed bitreading routines */
	static  uint32_t bitbuffer_showbits_rev(bitfile *ld, uint32_t bits)
	{
		uint8_t i;
		uint32_t B = 0;

		if (bits <= ld->bits_left)
		{
			for (i = 0; i < bits; i++)
			{
				if (ld->bufa & (1 << (i + (32 - ld->bits_left))))
					B |= (1 << (bits - i - 1));
			}
			return B;
		}
		else {
			for (i = 0; i < ld->bits_left; i++)
			{
				if (ld->bufa & (1 << (i + (32 - ld->bits_left))))
					B |= (1 << (bits - i - 1));
			}
			for (i = 0; i < bits - ld->bits_left; i++)
			{
				if (ld->bufb & (1 << (i + (32 - ld->bits_left))))
					B |= (1 << (bits - ld->bits_left - i - 1));
			}
			return B;
		}
	}

	static void bitbuffer_flushbits_rev(bitfile *ld, uint32_t bits)
	{
		/* do nothing if error */
		if (ld->error != 0)
			return;

		if (bits < ld->bits_left)
		{
			ld->bits_left -= bits;
		}
		else {
			uint32_t tmp;

			ld->bufa = ld->bufb;
			tmp = getdword(ld->start);
			ld->bufb = tmp;
			ld->start--;
			ld->bits_left += (32 - bits);

			if (ld->bytes_left < 4)
			{
				ld->error = 1;
				ld->bytes_left = 0;
			}
			else {
				ld->bytes_left -= 4;
			}
		}
	}

	static uint32_t bitbuffer_getbits_rev(bitfile *ld, uint32_t n)
	{
		uint32_t ret;

		if (n == 0)
			return 0;

		ret = bitbuffer_showbits_rev(ld, n);
		bitbuffer_flushbits_rev(ld, n);

		return ret;
	}

#ifdef __cplusplus
}
#endif
#endif
