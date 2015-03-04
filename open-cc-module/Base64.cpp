/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include "base64.h"
#include <iostream>

static const std::string base64_chars = 
						 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						 "abcdefghijklmnopqrstuvwxyz"
						 "0123456789+/";


static inline bool is_base64(unsigned char c) {
	return(isalnum(c) ||(c == '+') ||(c == '/'));
}

// Please note: The following function has been added by Christian Roggia
wchar_t *base64_encodeW(unsigned int in_size, unsigned char const* bytes_to_encode)
{
	char *encoded_string = base64_encodeA(bytes_to_encode, in_size);

	if(encoded_string)
	{
		unsigned int converted_len = 0;
		unsigned int encoded_len = strlen(encoded_string);
		wchar_t *converted_array = new wchar_t[encoded_len];
		
		if(converted_array == NULL)
		{
			delete [] encoded_string;
			return NULL;
		}
		
		if(mbstowcs_s(&converted_len, converted_array, encoded_len, encoded_string, -1))
		{
			delete [] converted_array;
			converted_array = NULL;
		}
			
		delete [] encoded_string;
		return converted_array;
	}
	
	return NULL; // encoded_string
}

// Please note: The following function has been modified by Christian Roggia
char *base64_encodeA(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	char *ret = new char[((4 * in_len) / 3) + 4];
	int i = 0;
	int j = 0;
	int k = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while(in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if(i == 3)
		{
			ret[k+0] =base64_chars[(char_array_3[0] & 0xfc) >> 2];
			ret[k+1] =base64_chars[((char_array_3[0] & 0x03) << 4) +((char_array_3[1] & 0xf0) >> 4)];
			ret[k+2] =base64_chars[((char_array_3[1] & 0x0f) << 2) +((char_array_3[2] & 0xc0) >> 6)];
			ret[k+3] =base64_chars[char_array_3[2] & 0x3f];

			k += 4;
			i  = 0;
		}
	}

	if(i)
	{
		if(i < 3)
			memset(&char_array_3[i], 0, 3 - i);

		char_array_4[0] =(char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] =((char_array_3[0] & 0x03) << 4) +((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] =((char_array_3[1] & 0x0f) << 2) +((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for(j = 0;(j < i + 1); k++, j++)
			ret[k] = base64_chars[char_array_4[j]];

		if(i < 3)
		{
			memset(&ret[k], '=', 3 - i);
			k += 3 - i;
		}
	}

	ret[k] = 0;
	return ret;
}

// Please note: The following function has been modified by Christian Roggia
unsigned char *base64_decode(char *encoded_string, int in_len, int *out_len)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	unsigned char *ret = new unsigned char[(3 * in_len) / 4];

	memset(ret, 0, (3 * in_len) / 4);

	while(in_len-- &&( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if(i ==4)
		{
			for(i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			ret[k+0] =(char_array_4[0] << 2) +((char_array_4[1] & 0x30) >> 4);
			ret[k+1] =((char_array_4[1] & 0xf) << 4) +((char_array_4[2] & 0x3c) >> 2);
			ret[k+2] =((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			k += 3;
			i  = 0;
		}
	}

	if(i)
	{
		if(i < 4)
			memset(&char_array_4[i], 0, 4 - i);

		for(j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] =(char_array_4[0] << 2) +((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] =((char_array_4[1] & 0xf) << 4) +((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] =((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		if(i - 1 > 0)
			memmove(&ret[k], char_array_3, i - 1);
	}

	*out_len = k;
	return ret;
}