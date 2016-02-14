#pragma once

inline int sha256_S(int X, int n) { return ((unsigned)X >> n) | (X << (32 - n)); }
inline int sha256_R(int X, int  n) { return ((unsigned)X >> n); }
int sha256_Ch(int x, int  y, int z) { return ((x & y) ^ ((~x) & z)); }
int sha256_Maj(int x, int y, int z) { return ((x & y) ^ (x & z) ^ (y & z)); }
int sha256_Sigma0256(int x) { return (sha256_S(x, 2) ^ sha256_S(x, 13) ^ sha256_S(x, 22)); }
int sha256_Sigma1256(int x) { return (sha256_S(x, 6) ^ sha256_S(x, 11) ^ sha256_S(x, 25)); }
int sha256_Gamma0256(int x) { return (sha256_S(x, 7) ^ sha256_S(x, 18) ^ sha256_R(x, 3)); }
int sha256_Gamma1256(int x) { return (sha256_S(x, 17) ^ sha256_S(x, 19) ^ sha256_R(x, 10)); }
int sha256_Sigma0512(int x) { return (sha256_S(x, 28) ^ sha256_S(x, 34) ^ sha256_S(x, 39)); }
int sha256_Sigma1512(int x) { return (sha256_S(x, 14) ^ sha256_S(x, 18) ^ sha256_S(x, 41)); }
int sha256_Gamma0512(int x) { return (sha256_S(x, 1) ^ sha256_S(x, 8) ^ sha256_R(x, 7)); }
int sha256_Gamma1512(int x) { return (sha256_S(x, 19) ^ sha256_S(x, 61) ^ sha256_R(x, 6)); }
int sha256_K[] = {

	1116352408, 1899447441, -1245643825, -373957723, 961987163, 1508970993,
	-1841331548, -1424204075, -670586216, 310598401, 607225278, 1426881987,
	1925078388, -2132889090, -1680079193, -1046744716, -459576895, -272742522,
	264347078, 604807628, 770255983, 1249150122, 1555081692, 1996064986,
	-1740746414, -1473132947, -1341970488, -1084653625, -958395405, -710438585,
	113926993, 338241895, 666307205, 773529912, 1294757372, 1396182291,
	1695183700, 1986661051, -2117940946, -1838011259, -1564481375, -1474664885,
	-1035236496, -949202525, -778901479, -694614492, -200395387, 275423344,
	430227734, 506948616, 659060556, 883997877, 958139571, 1322822218,
	1537002063, 1747873779, 1955562222, 2024104815, -2067236844, -1933114872,
	-1866530822, -1538233109, -1090935817, -965641998
};
inline int safe_add(int x, int y)
{
	return x + y;
	/*int lsw = (x & 0xFFFF) + (y & 0xFFFF);
	int msw = (x >> 16) + (y >> 16) + (lsw >> 16);
	return (msw << 16) | (lsw & 0xFFFF);*/
}
int * rstr2binb(const std::string& buffer,int len) {
	int strLen = buffer.length();
	int* result = (int*)malloc(len);
	byte * data = (byte*)buffer.data()+3;
	byte* temp = (byte*)result;
	for (int i = 0; i < strLen; data += 8,i+=4) {
		*(temp++) = *(data--);
		*(temp++) = *(data--);
		*(temp++) = *(data--);
		*(temp++) = *(data--);
	}
	int pad = len - strLen;
	if (pad > 0)
	for (int i = 0; i < 4;i++, data--, temp++) {
		*(temp) = i < pad ? 0 : *(data);
	}
	/*for (int i = 0; i <len * 8; i += 8) {
		result[i >> 5] |= ((byte)buffer[i / 8]) << (24 - i % 32);
	}*/
	return result;
}
void binb_sha256(int* m, int mlen, int l)
{
	int HASH[] = { 1779033703, -1150833019, 1013904242, -1521486534,
		1359893119, -1694144372, 528734635, 1541459225 };
	int W[64];
	int a, b, c, d, e, f, g, h;
	int i, j, T1, T2;

	int m1 = l >> 5, m2 = (((l + 64) >> 9) << 4) + 15;
	int mv1 = m1 >= mlen ? 0 : m[m1];
	mv1 |= 0x80 << (24 - l % 32);

	int mNewLen = mlen;
	if (m1 >= mNewLen)mNewLen = m1 + 1;
	if (m2 >= mNewLen)mNewLen = m2 + 1;
	//m[l >> 5] |= 0x80 << (24 - l % 32);
	//m[(((l + 64) >> 9) << 4) + 15] = l;

	for (i = 0; i < mNewLen; i += 16)
	{
		a = HASH[0];
		b = HASH[1];
		c = HASH[2];
		d = HASH[3];
		e = HASH[4];
		f = HASH[5];
		g = HASH[6];
		h = HASH[7];

		for (j = 0; j < 64; j++)
		{
			if (j < 16) W[j] = (j + i == m1 ? mv1 : (j + i == m2 ? l : ((j + i) >= mlen ? 0 : m[j + i])));
			else W[j] = safe_add(safe_add(safe_add(sha256_Gamma1256(W[j - 2]), W[j - 7]),
				sha256_Gamma0256(W[j - 15])), W[j - 16]);

			T1 = safe_add(safe_add(safe_add(safe_add(h, sha256_Sigma1256(e)), sha256_Ch(e, f, g)),
				sha256_K[j]), W[j]);
			T2 = safe_add(sha256_Sigma0256(a), sha256_Maj(a, b, c));
			h = g;
			g = f;
			f = e;
			e = safe_add(d, T1);
			d = c;
			c = b;
			b = a;
			a = safe_add(T1, T2);
			//cout << i <<":"<< j<<":" << a << endl;
		}

		HASH[0] = safe_add(a, HASH[0]);
		HASH[1] = safe_add(b, HASH[1]);
		HASH[2] = safe_add(c, HASH[2]);
		HASH[3] = safe_add(d, HASH[3]);
		HASH[4] = safe_add(e, HASH[4]);
		HASH[5] = safe_add(f, HASH[5]);
		HASH[6] = safe_add(g, HASH[6]);
		HASH[7] = safe_add(h, HASH[7]);
	}
	memcpy(m, HASH, 8 * 4);
}
std::string binb2rstr(int *input, int len)
{
	std::string output(len << 2, '\0');
	byte * data = (byte*)output.data() ;
	byte* temp = (byte*)input;
	temp += 3;
	for (int i = 0; i < len; temp += 8, i++) {
		*(data++) = *(temp--);
		*(data++) = *(temp--);
		*(data++) = *(temp--);
		*(data++) = *(temp--);
	}
	/*for (int i = 0; i < len * 32; i += 8)
		output[i / 8] = (unsigned)input[i >> 5] >> (24 - i % 32);*/
	return output;
}
std::string rstr_hmac_sha256(const std::string& key, const std::string& data) {
	//return data.size();
	int keyLength = key.length();
	int dataLength = data.length();
	int len = keyLength >> 2;
	int data_len = dataLength >> 2;
	if (keyLength % 4 != 0)len++;
	if (dataLength % 4 != 0)data_len++;
	int * bkey = rstr2binb(key, len<<2);
	if (len > 16)binb_sha256(bkey, len, keyLength * 8);
	int* ipad = (int *)malloc(16 * 4 + dataLength);
	int opad[16 + 8];
	for (int i = 0; i < 16; i++)
	{
		ipad[i] = (i<len ? bkey[i] : 0) ^ 0x36363636;
		opad[i] = (i<len ? bkey[i] : 0) ^ 0x5C5C5C5C;
	}
	free(bkey);
	int * bdata = rstr2binb(data, data_len<<2);
	for (int i = 0; i <data_len; i++) {
		ipad[16 + i] = bdata[i];
	}
	free(bdata);
	binb_sha256(ipad, 16 + data_len, 512 + dataLength * 8);
	for (int i = 0; i < 8; i++) {
		opad[16 + i] = ipad[i];
	}
	free(ipad);
	binb_sha256(opad, 16 + 8, 512 + 256);
	return binb2rstr(opad, 8);
}
