#include <stdio.h>
#include <stdlib.h>

#define ctoi(c) ((unsigned int)(c - '0'))

typedef struct {
	unsigned char *blocks; //little endian
	int block_count;
} mpz;

int str_len(char *in) {
	int ct;
	ct = 0;
	while(*in) {
		in++;
		ct++;
	}
	return ct;
}

int safe_bit_ct(char *in) {
	int ct;
	
	ct = 4 * str_len(in);
	while(ct % 4) { ++ct; }
	return ct;
}

void load_ui(mpz *rop, unsigned int in) {
	int i;
	for(i = 0; i < sizeof(unsigned int); ++i) {
		rop->blocks[i] = in & 0xFF;
		in >>= 8;
	}
}

void load(mpz *rop, const mpz *op1) {
	int i;
	for(i = 0; i < rop->block_count; ++i) {
		rop->blocks[i] = op1->blocks[i];
	}
}

mpz *create_mpz(int bits) {
	mpz *this = malloc(sizeof(mpz));
	this->block_count = (bits >> 3) + 1;
	this->blocks = calloc(this->block_count,
	                      sizeof(char));
	return this;
}

int compare(const mpz *a, const mpz *b) {
	if(a->block_count != b->block_count) return 0;
	
	int i;
	for(i = 0; i < a->block_count; ++i) {
		if(a->blocks[i] != b->blocks[i]) return 0;
	}
	return 1;
}

//1 = true, 0 = false
int compare1(const mpz *a) { return a->blocks[0] & 1; }

int compare0(const mpz *a) {
	int i;
	for(i = 0; i < a->block_count; ++i) {
		if(a->blocks[i]) return 0;
	}
	return 1;
}

void destroy_mpz(mpz *op) {
	free(op->blocks);
	free(op);
}

void load_mpz_str(mpz *rop, char *in) {
	//TODO
}

void clear_mpz(mpz *rop) {
	int i;
	for(i = 0; i < rop->block_count; ++i) {
		rop->blocks[i] = 0x00;
	}
}

void print_mpz(const mpz *rop) {
	int bin_len = 1 + (8 * rop->block_count);
	char bin[bin_len], buf[bin_len + 30];
	
	//bin
	int i, j = 0;
	for(i = rop->block_count - 1; i >= 0; --i) {
		int bl = rop->blocks[i], k;
		for(k = 0; k < 8; ++k) {
			bin[j++] = !!(bl & 0x80) + '0';
			bl <<= 1;
		}
	}
	bin[j] = '\0';

	//printf("%s\n",bin);

	//buf
	sprintf(buf, "echo \"ibase=2;%s\"|bc", bin);
	system(buf);
}

int inc(mpz *rop) {
	int i, carry = 1;
	for(i = 0; i < rop->block_count; ++i) {
		if(carry && rop->blocks[i] == 0xFF) {
			rop->blocks[i] = 0x00;
			carry = 1; //overflow!
		}
		else {
			rop->blocks[i] += carry;
			carry = 0;
		}
	}
	return carry;
}

//dec(mpz) -> negative overflow
int dec(mpz *rop) {
	int i, carry = 1;
	for(i = 0; i < rop->block_count; ++i) {
		if(carry && rop->blocks[i] == 0x00) {
			rop->blocks[i] = 0xFF;
			carry = 1; //overflow!
		}
		else {
			rop->blocks[i] -= carry;
			carry = 0;
		}
	}
	return carry;
}

int inv(mpz *rop, const mpz *op) {
	int i;
	for(i = 0; i < op->block_count; ++i) {
		rop->blocks[i] = ~(op->blocks[i]);
	}
}

int add(mpz *rop, const mpz *op, const mpz *op2) {
	//now, add 'op'
	int i, sum, overflow = 0;
	for(i = 0; i < op->block_count; ++i) {
		sum = op->blocks[i] + op2->blocks[i] + overflow;
		rop->blocks[i] = sum > 0xFF ? 0x00 : sum;
		overflow = sum - 0xFF > 0 ? sum - 0xFF : 0;       
	}
	return overflow;
}

void ls2(mpz *rop) {
	int i, cur, old = 0;
	for(i = 0; i < rop->block_count; ++i) {
		cur = !!(rop->blocks[i] & 0x80);
		rop->blocks[i] <<= 1;
		rop->blocks[i] |= old;
		old = cur;
	}
}

void mul(mpz *rop, const mpz *op1, const mpz *op2) {
	
}

void idiv(mpz *rop, const mpz *op1, const mpz *op2) {
	
}

//a mod b = a - floor(a / b) * b
void mod(mpz *rop, const mpz *op1, const mpz *op2) {
	
}

int main(int argc, char **argv) {
	mpz *a = create_mpz(32), *b = create_mpz(32), *c = create_mpz(32);
	
	load_ui(a, 12);
	load_ui(b, 20);
	mod(c, a, b);
	print_mpz(c);
	
	destroy_mpz(a);
	destroy_mpz(b);
	destroy_mpz(c);
}










