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

mpz *create_mpz(int bits, int block) {
	mpz *this = malloc(sizeof(mpz));
	if(!block)
		this->block_count = (bits >> 3) == 0 ? 1 : (bits >> 3);
	else
		this->block_count = block;
	
	this->blocks = calloc(this->block_count,sizeof(char));
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

	printf("%s\n",bin);

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

int inv(mpz *rop) {
	int i;
	for(i = 0; i < rop->block_count; ++i) {
		rop->blocks[i] = ~rop->blocks[i];
	}
}

int add(mpz *rop, const mpz *op, const mpz *op2) {
	//now, add 'op'
	int i, sum, overflow = 0;
	for(i = 0; i < op->block_count; ++i) {
		sum = op->blocks[i] + op2->blocks[i] + overflow;
		rop->blocks[i] = sum & 0xFF;
		overflow = sum >> 8;
	}
	return overflow;
}


int shift_add(mpz *rop, const mpz *op, const mpz *op2, int off8) {
	int i, sum, overflow = 0;
	for(i = 0; i < op->block_count; ++i) {
		//todo: remove branching
		if(i - off8 < 0)
			sum = op->blocks[i] + overflow;
		else
			sum = op->blocks[i] + op2->blocks[i - off8] + overflow;
			
		rop->blocks[i] = sum & 0xFF;
		overflow = sum >> 8;
	}
	return overflow;
}

void ls4(mpz *rop, const mpz *op, int off8) {
	int i;
	for(i = op->block_count - 1; i >= off8; --i) {
		rop->blocks[i] = op->blocks[i - off8];
	}
	
	for(i = 0; i < off8; ++i) {
		rop->blocks[i] = 0x00;
	}
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

void ls3(mpz *rop, const mpz *op, unsigned int n) {
	int i, cur, old = 0, mask, mask2;
	mask  = (2 << n) - 1;
	mask2 = mask << (8 - n);
	for(i = 0; i < rop->block_count; ++i) {
		//paste
		rop->blocks[i] = op->blocks[i];
		
		cur = (rop->blocks[i] & mask2) >> (8 - n);
		rop->blocks[i] <<= n;
		rop->blocks[i] |= old;
		old = cur;
	}
}

int get_bit(const mpz *a, unsigned int x) {
	unsigned char block = a->blocks[(x / 8)];
	return 0x1 & (block >> (x % 8));
}	

void mul(mpz *rop, const mpz *op1, const mpz *op2) {
	clear_mpz(rop);
	mpz *tmp = create_mpz(0, op1->block_count);
	
	//with bit coarseness = 1, multiplication == operator&
	//ls3 to shift. yw :-)
	
	//we will only consider the lower significant bits
	int lsb = 8 * (op1->block_count >> 1);
	int i, other;
	for(i = 0; i < lsb; ++i) {
		other = get_bit(op2, i);
		
		//todo: no branch
		if(other) {
			ls4(tmp,op1, i / 8);
			ls3(tmp,tmp, i % 8);
			add(rop,rop,tmp);
		}
	}
	destroy_mpz(tmp);
}

void idiv(mpz *rop, const mpz *op1, const mpz *op2) {
	
}

//a mod b = a - floor(a / b) * b
void mod(mpz *rop, const mpz *op1, const mpz *op2) {
	
}

void sub(mpz *rop, const mpz *op1, const mpz *op2) {

}

int main(int argc, char **argv) {
	mpz *a = create_mpz(0,4), *b = create_mpz(0,4), *c = create_mpz(0,4);
	
	load_ui(a, 512);
	print_mpz(a);
	load_ui(b, 512);
	print_mpz(b);
	
	mul(c, a, b);
	print_mpz(c);
	
	destroy_mpz(a);
	destroy_mpz(b);
	destroy_mpz(c);
}










