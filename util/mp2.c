#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct {
	unsigned int *blocks;
	unsigned int block_count;
} mpz;

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
	if(rop == op) return; //save time
	
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
	
	if(decimal) { //lul i'm not writing IDIV
		cmd = calloc(1, 255 + (32 * x->block_count));
		sprintf(cmd, "echo \"ibase=2;%s\"|bc", buf);
		system(cmd);
		free(cmd);
	}
	else
		printf("%s\n",buf);
	
	free(buf);
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

void subtract(mpz *rop, const mpz *op1, const mpz *op2) {
	int i; 
	unsigned long borrow;
	unsigned long diff;  //!!!
	
	borrow = 0L;
	for(i = 0; i < op1->block_count; ++i) {
		
		//perform the subtraction
		diff = (unsigned long)op1->blocks[i] - 
		       (unsigned long)op2->blocks[i] - borrow;
		//calculate borrow
		borrow = (diff & 0x80000000) >> 31;	
		
		//if borrow, offset. this is compiled to a cmove
		if(borrow) {
			diff += 0x200000000;
		}
		
		rop->blocks[i] = diff & 0xFFFFFFFF;
	}
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
	for(i = 0; i <= op->block_count - off8; ++i) {
		rop->blocks[i] = op->blocks[i + off8];
	}
	
	for(i = op->block_count - off8 + 1; i < op->block_count; ++i) {
		rop->blocks[i] = 0x00;
	}
}

void rs3(mpz *rop, const mpz *op, unsigned int bits) {
	rs2(rop, op,  bits / 32);
	rs (rop, rop, bits % 32);
}

//gcd -> gcd. THIS SCRAMBLES BOTH ARGUMENTS
//adapted from rust's uutils binary GCD
mpz *gcd(mpz *u, mpz *v) {
	int i, j, k;
	mpz *t;
	
	if     (compare0(u)) return u;
	else if(compare0(v)) return v;
	
	i = ctz(u);
	rs3(u, u, i);
	
	j = ctz(v);
	rs3(v, v, j);
	
	k = i < j ? i : j;
	
	while(1) {
		if(compare(u, v) > 0) {
			t = u;
			u = v;
			v = t;
		}
		
		subtract(v, v, u);
		
		if(compare0(v)) {
			ls3(u, u, k);
			return u;
		}
		
		rs3(v, v, ctz(v));
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
	sum = (unsigned long)op1->blocks[off8 + 1] + (op2 >> 32);
	rop->blocks[off8 + 1] = sum & 0xFFFFFFFF;
	overflow = sum >> 32;
	
	//propagate carry
	for(i = off8 + 2; i < op1->block_count; ++i) {
		sum = (unsigned long)op1->blocks[i] + overflow;
		rop->blocks[i] = sum & 0xFFFFFFFF;
		overflow = sum >> 32;
	}
}

void square(mpz *rop, const mpz *op) {
	int i, j;
	unsigned long product;
	
	zero(rop); //needed?

	for(i = 0; i < op->block_count; ++i) {
		for(j = 0; j < op->block_count; ++j) {
			printf("Block %d (loff %d)\n", j, i);
			product = op->blocks[i] * op->blocks[j];
			add_lu_off(rop, rop, product, i + j);
		}
	}		
}

int main() {
	mpz *x, *y, *z;
	x = create_mpz(4);
	y = create_mpz(4);
	
	load_ui(x, 4, 0);
	load_ui(x, 5, 1);
	print(x,0);
	print(x,1);
	
	square(y, x);
	print(y, 0);
	print(y, 1);
	

	destroy_mpz(x);
	destroy_mpz(y);
	return 0;
}








