#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "mp2.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))


mpz *create_mpz(unsigned int block_ct) {
	mpz *this = malloc(sizeof(mpz));
	this->blocks = calloc(block_ct, sizeof(unsigned int));
	this->block_count = block_ct;
	return this;
}

void destroy_mpz(mpz *this) {
	free(this->blocks);
	free(this);
}

void swap(mpz *op1, mpz *op2) {
	if(op1 == op2) return;
	
	unsigned int i, tmp;
	for(i = 0; i < op1->block_count; ++i) {
		tmp = op1->blocks[i];
		op1->blocks[i] = op2->blocks[i];
		op2->blocks[i] = tmp;
	}
}

void load(mpz *rop, const mpz *op) {
	if(rop == op) return; //save time maybe
	
	int i;
	for(i = 0; i < op->block_count; ++i) {
		rop->blocks[i] = op->blocks[i];
	}
}

void load_ui(mpz *rop, unsigned int op, unsigned int off8) {
	rop->blocks[off8] = op;
}

int compare(const mpz *op1, const mpz *op2) {
	int i;
	for(i = op1->block_count - 1; i >= 0; --i) {
		if     (op1->blocks[i] < op2->blocks[i]) return -1;
		else if(op1->blocks[i] > op2->blocks[i]) return  1;
	}
	return 0;
}

int compare0(const mpz *op) {
	int i;
	for(i = 0; i < op->block_count; ++i) {
		if(op->blocks[i]) return 0;
	}
	return 1;
}

int compare1(const mpz *op) {
	int i;
	for(i = 1; i < op->block_count; ++i) {
		if(op->blocks[i]) return 0;
	}
	return *(op->blocks) == 1;
}

void zero(mpz *op) {
	int i;
	for(i = 0; i < op->block_count; ++i) { 
		op->blocks[i] = 0; 
	}
}

void add(mpz *rop, const mpz *op1, const mpz *op2) {
	int i; 
	unsigned long overflow;
	unsigned long sum;
	
	overflow = 0L;
	for(i = 0; i < op1->block_count; ++i) {
		sum = (unsigned long)op1->blocks[i] + (unsigned long)op2->blocks[i] 
		                           + overflow;
		rop->blocks[i] = sum & 0xFFFFFFFF;
		overflow = sum >> 32;
	}
	
	//return overflow;
}

void add_ui(mpz *rop, const mpz *op1, unsigned int op2) {
	int i;
	unsigned long overflow;
	unsigned long sum;
	
	//unroll first loop
	sum = (unsigned long)*op1->blocks + op2;
	*rop->blocks = sum & 0xFFFFFFFF;
	overflow = sum >> 32;
	
	//propagate carry
	for(i = 1; i < op1->block_count; ++i) {
		sum = (unsigned long)op1->blocks[i] + overflow;
		rop->blocks[i] = sum & 0xFFFFFFFF;
		overflow = sum >> 32;
	}
}

void inv(mpz *rop, const mpz *op) {
	int i;
	for(i = 0; i < op->block_count; ++i) {
		rop->blocks[i] = ~op->blocks[i];
	}
}

void subtract(mpz *rop, const mpz *op1, const mpz *op2) {
	int i; 
	unsigned char borrow;
	signed   long diff;  //!!!
	
	borrow = 0;
	
	for(i = 0; i < op1->block_count; ++i) {
		
		//perform the subtraction
		diff = (unsigned long)op1->blocks[i] - (unsigned long)op2->blocks[i] - borrow;
			   
		//TODO: get rid of conditional
		if(diff < 0) { //overflow
			borrow = 1;
			diff = diff + 0x100000000;
		}
		else
			borrow = 0;
		rop->blocks[i] = (unsigned long)diff & 0xFFFFFFFF;
	}
	
	//print(rop,0);
}

// a - b if a >= b else b - a
void difference(mpz *rop, const mpz *op1, const mpz *op2) {
	const mpz *high, *low;
	int comp;
	
	high = op1;
	low  = op1;
	comp = compare(op1, op2);
	
	if(comp < 0) //op1 < op2
		high = op2;
	else         //op1 >= op2
		low  = op2;

	subtract(rop, high, low);
}

//This is slow. But once ported, can replace with:
/**
	function ctz (uint x)
		x &= -x
		return 32 - (__clz(x) + 1)
*/
unsigned int ctz_(unsigned int x) {
    unsigned int n;
	n = 0;
    if ((x & 0x0000FFFF) == 0) {
		n = n + 16;
		x = x >> 16;
	}
    if ((x & 0x000000FF) == 0) {
		n = n + 8;
		x = x >> 8;
	}
    if ((x & 0x0000000F) == 0) {
		n = n + 4;
		x = x >> 4;
	}
    if ((x & 0x00000003) == 0) {
		n = n + 2;
		x = x >> 2;
	}
    if ((x & 0x00000001) == 0) {
		n = n + 1;
	}
    return n;
}

unsigned int ctz(const mpz *op) {
	int i, ct;
	unsigned int x;
	
	ct = 0;
	for(i = 0; i < op->block_count; ++i) {
		x = op->blocks[i];
		if(x) return ct + ctz_(x);
		ct += 32;
	}
	return ct;
}

//shift within a block (0 <= bits <= 31)
void ls(mpz *rop, const mpz *op, unsigned int bits) {
	int i;
	unsigned int  carry = 0x00000000;
	unsigned long copy  = 0x00000000;

	for(i = 0; i < op->block_count; ++i) {
		copy = op->blocks[i];
		rop->blocks[i] = (copy << bits) + carry;
		carry = copy >> (32 - bits);
	}
}

//shift by blocks
void ls2(mpz *rop, const mpz *op, int off8) {
	int i;
	
	for(i = op->block_count - 1; i >= off8; --i) {
		rop->blocks[i] = op->blocks[i - off8];
	}
	
	for(i = 0; i < off8; ++i) {
		rop->blocks[i] = 0x00;
	}
}

//shift by arbitrary bit ct
void ls3(mpz *rop, const mpz *op, unsigned int bits) {
	ls2(rop, op,  bits / 32);
	ls (rop, rop, bits % 32);
}

void rs(mpz *rop, const mpz *op, unsigned int bits) {
	int i;
	unsigned long carry = 0x00000000;
	unsigned long copy  = 0x00000000;

	for(i = op->block_count - 1; i >= 0; --i) {
		copy = op->blocks[i];
		rop->blocks[i] = (copy >> bits) + (carry << (32 - bits));
		carry = copy & ( (1 << bits) - 1 );
	}
}

void rs2(mpz *rop, const mpz *op, unsigned int off8) {
	int i;
	for(i = 0; i < op->block_count - off8; ++i) {
		rop->blocks[i] = op->blocks[i + off8];
	}
	
	for(i = op->block_count - off8; i < op->block_count; ++i) {
		rop->blocks[i] = 0x00;
	}
}

void rs3(mpz *rop, const mpz *op, unsigned int bits) {
	rs2(rop, op,  bits / 32);
	rs (rop, rop, bits % 32);
}

void gcd(mpz *a, mpz *b, mpz *scratch) {
	while(!compare0(b)) {
		idiv(NULL,scratch,a,b);
		load(a, b);
		load(b, scratch);
	}
}

void add_lu_off(mpz *rop, const mpz *op1, unsigned long op2,
                                          int off8) {
	int i;
	unsigned long overflow;
	unsigned long sum;

	//set low
	for(i = 0; i < off8; ++i) {
		rop->blocks[i] = op1->blocks[i];
	}

	//unroll first loop
	sum = (unsigned long)op1->blocks[off8] + (op2 & 0xFFFFFFFF);
	rop->blocks[off8] = sum & 0xFFFFFFFF;
	overflow = sum >> 32;
	
	//unroll second loop
	sum = (unsigned long)op1->blocks[off8 + 1] + (op2 >> 32)
	                                           + overflow;
	rop->blocks[off8 + 1] = sum & 0xFFFFFFFF;
	overflow = sum >> 32;
	
	//propagate carry
	for(i = off8 + 2; i < op1->block_count; ++i) {
		sum = (unsigned long)op1->blocks[i] + overflow;
		rop->blocks[i] = sum & 0xFFFFFFFF;
		overflow = sum >> 32;
	}
}


void mul(mpz *rop, const mpz *op1, const mpz *op2) {
	int a, b;
	unsigned long carry, a_block, b_block;
	
	int halb = (op1->block_count >> 1) + (op2->block_count >> 1);
	unsigned long product[halb];
	for(a = 0; a < halb; ++a) product[a] = 0L;
	
	for(b = 0; b < (op2->block_count >> 1); ++b) {
		carry = 0L;
		b_block = op2->blocks[b];
		for(a = 0; a < (op1->block_count >> 1); ++a) {
			a_block = op1->blocks[a];

			product[a + b] += carry + (a_block * b_block);
			
			carry = product[a + b] / 0x100000000;
			
			product[a + b] &= UINT_MAX;
		}
		product[b + a] += (unsigned int)carry; //last carry
	}

	for(a = 0; a < halb; ++a) {
		rop->blocks[a] = product[a] & 0xFFFFFFFF;
    }
	
	for(a = halb; a < rop->block_count; ++a) { 
		rop->blocks[a] = 0x00000000; //likely won't run
	}
}

int str_len(const char *string) {
	int len;
	
	len = 0;
	while(*string) {
		string++;
		len++;
	}
	return len;
}

//utility functions

void load_create(mpz **rop, const char *string, int sz) {
	//alloc buffers
	//log2(10) ~ 3.32, so we'll say 4 bits per base10 digit
	int digits = 4 * str_len(string);
	char cmd[64 + str_len(string)], buf[64 + digits];
	sprintf(cmd, 
	"export BC_LINE_LENGTH=0;echo \"obase=2;%s\"|bc|tr -d '\n'", 
	        string);

	//let BC do the work
	FILE *pipe = popen(cmd,"r");
	fgets(buf, 63 + digits, pipe);
	pclose(pipe);

	int len = str_len(buf);
	int blocks;
	
	if     (sz ==  0) blocks = (len / 32) + (len % 32 != 0);
	else if(sz == -1) blocks = 2*((len / 32) + (len % 32 != 0));
	else              blocks = sz;
	
	if(blocks % 2) ++blocks; //ensure we have upper + lower half
	                         //falling across block bound. 
							 //for purposes of mul
	
	*rop = create_mpz( blocks );
	
	unsigned int cur_block_id   = 0;
	unsigned int cur_block_data = 0;
	int i = len - 1, j, back;
	while(i != -1) {
		back = MAX(-1, i - 32);

		for(j = back + 1; j <= i; ++j) {
			
			
			cur_block_data <<= 1;
			cur_block_data |= (buf[j] - '0' == 0 ? 0x0 : 0x1);
		}

		(*rop)->blocks[cur_block_id++] = cur_block_data;
		cur_block_data = 0x0;
		i = back;
	}
}

void print_ui(unsigned int x, char *buf) {
	int i;
	for(i = 0; i < 32; ++i) {
		buf[32 - i - 1] = '0' + (x & 0x1);
		x >>= 1;
	}
}

void print(const mpz *x, int decimal) {
	char *buf, *cmd;
	int i;
	
	buf = calloc(1, 1 + (32 * x->block_count));
	
	for(i = x->block_count - 1; i >= 0; --i) {
		print_ui(x->blocks[i], 
		         buf + (32 * (x->block_count - i - 1)));
	}
	
	if(decimal) {
		cmd = calloc(1, 255 + (32 * x->block_count));
		sprintf(cmd, "echo \"ibase=2;%s\"|bc", buf);
		system(cmd);
		free(cmd);
	}
	else
		printf("%s\n",buf);
	
	free(buf);
}

int get_bit(const mpz *a, unsigned int x) {
	unsigned int block = a->blocks[(x / 32)];
	return 0x1 & (block >> (x % 32));
}

void set_bit(mpz *a, unsigned int x) {
	unsigned int mask = 1 << (x % 32);
	a->blocks[(x / 32)] |= mask;
}
 
void idiv(mpz *q, mpz *r, const mpz *n, const mpz *d) {
	zero(r);
	if(q) zero(q);
	
	int i;
	for(i = (32 * n->block_count) - 1; i >= 0; --i) {
		ls(r, r, 1);
		r->blocks[0] |= get_bit(n,i);

		if(compare(r, d) >= 0) {
			subtract(r, r, d);
			if(q) set_bit(q, i);
		}
	}
}