#ifndef UTILITIES_H
#define UTILITIES_H

inline unsigned long xorshift(unsigned long x) {
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}

char* get_time();

#endif /* UTILITIES_H */