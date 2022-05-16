#include <stdio.h>
#include <stdlib.h>

#include "mp.h"

#define ctoi(c) ((unsigned int)(c - '0'))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

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
	return MAX(8 * sizeof(unsigned int), ct);
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

void swap(mpz *op1, mpz *op2) {
	int i;
	unsigned char tmp;
	for(i = 0; i < op1->block_count; ++i) {
		tmp = op1->blocks[i];
		op1->blocks[i] = op2->blocks[i];
		op2->blocks[i] = tmp;
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
	if(a->block_count != b->block_count)
		b->block_count > a->block_count ? -1 : 1;
		
	int i;
	for(i = a->block_count - 1; i >= 0; --i) {
		if(a->blocks[i] != b->blocks[i])
			return b->blocks[i] > a->blocks[i] ? -1 : 1;
	}
	return 0;
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

void set_bit(mpz *a, unsigned int x) {
	unsigned char mask = 1 << (x % 8);
	a->blocks[(x / 8)] |= mask;
}

void clear_bit(mpz *a, unsigned int x) {
	unsigned char mask = 1 << (x % 8);
	a->blocks[(x / 8)] &= ~mask;
}

int get_bit(const mpz *a, unsigned int x) {
	unsigned char block = a->blocks[(x / 8)];
	return 0x1 & (block >> (x % 8));
}	

void mul(mpz *rop, const mpz *op1, const mpz *op2, mpz *tmp) {
	clear_mpz(rop);
	
	int provided = !!tmp;
	if(!provided) tmp = create_mpz(0, op1->block_count);

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
	
	if(!provided) destroy_mpz(tmp);
}

int bit_count(const mpz *a) {
	int i, ct = 0;
	for(i = a->block_count - 1; i >= 0; --i) {
		unsigned char block = a->blocks[i];
		int j;
		for(j = 0; j < 8; ++j) {
			if(!!(block & 0x80)) {
				return (a->block_count * 8) - ct;
			}
			block <<= 1;
			ct++;
		}
	}
	return 0; // 0x0
}

void lower(mpz *a, int n) {
	int i;
	for(i = 0; i < a->block_count; ++i) {
		if(n > 8)
			n -= 8;
		else if(n > 0) { 
			a->blocks[i] &= ((1 << n) - 1);
			n = 0;
		}
		else
			a->blocks[i] = 0x00;
	}
}

void load_mpz_str(mpz *rop, char *in) {
	mpz *off, *ten, *par, *rs1, *rs2;
	
	clear_mpz(rop);
	
	off = create_mpz(2 * safe_bit_ct(in), 0);
	ten = create_mpz(2 * safe_bit_ct(in), 0);
	par = create_mpz(2 * safe_bit_ct(in), 0);
	rs1 = create_mpz(2 * safe_bit_ct(in), 0);
	rs2 = create_mpz(0, sizeof(unsigned int)); 
	load_ui(off, 1);
	load_ui(ten, 10);
	
	int i, j;
	for(i = str_len(in) - 1, j = 0; i >= 0; --i, ++j) {
		// load digit into 'result2'
		load_ui(rs2, ctoi(in[i]));

		// 'result1' = digit * power of 10
		mul(rs1, off, rs2, NULL); //OK to ask for a buffer here

		//add 'result1' to accumulator, rop
		add(rop, rop, rs1);
		
		//off *= 10
		mul(par, off, ten, NULL);
		load(off, par);
	}
	
	destroy_mpz(off);
	destroy_mpz(ten);
	destroy_mpz(par);
	destroy_mpz(rs1);
	destroy_mpz(rs2);
}

void push_bit(mpz *rop, int bit) {
	ls2(rop);
	rop->blocks[0] |= (bit & 0x1);
}

void idiv(mpz *rop, const mpz *op1, const mpz *op2, mpz *rem, mpz *tmp) {
	//set up memory
	int provided  = !!rem;
	if(!provided) rem = create_mpz(0, op1->block_count);
	int provided1 = !!tmp;
	if(!provided) tmp = create_mpz(0, op1->block_count);
	
	if(rop) clear_mpz(rop);
	clear_mpz(rem);
	
	int i = bit_count(op1) - 1;
	while(i != -1) {
		push_bit(rem, get_bit(op1, i));
		if(compare(rem, op2) < 0) {
			if(rop) push_bit(rop, 0);
		}
		else {
			if(rop) push_bit(rop, 1);
			abs_sub(tmp, rem, op2);
			load(rem, tmp);
		}
		i -= 1;
	}
	if(!provided)  destroy_mpz(rem);
	if(!provided1) destroy_mpz(tmp);
}

void abs_sub(mpz *rop, const mpz *op1, const mpz *op2) {
	int cut = MAX(bit_count(op1), bit_count(op2));
	
	load(rop, op2);
	inv(rop);
	inc(rop);
	
	add(rop, rop, op1);
	lower(rop, cut);
}