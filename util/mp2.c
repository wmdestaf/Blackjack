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

int clz(const mpz *op1) {
	
}

int ctz(const mpz *op2) {
	
}

void ls(unsigned int bits) {
	
}

void ls2(mpz *rop, const mpz *op, unsigned int off8) {
	int i;
	for(i = op->block_count - 1; i >= off8; --i) {
		rop->blocks[i] = op->blocks[i - off8];
	}
	
	for(i = 0; i < off8; ++i) {
		rop->blocks[i] = 0x00;
	}
}

void rs(unsigned int bits) {
	
}

void rs2(unsigned int off8) {
	int i;

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

void inv(mpz *rop, const mpz *op1) {
	int i;
	for(i = 0; i < op1->block_count; ++i) {
		rop->blocks[i] = ~op1->blocks[i];
	}
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
		sum = (long)op1->blocks[i] + (long)op2->blocks[i] 
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
	sum = (long)*op1->blocks + op2;
	*rop->blocks = sum & 0xFFFFFFFF;
	overflow = sum >> 32;
	
	//propagate carry
	for(i = 1; i < op1->block_count; ++i) {
		sum = (long)op1->blocks[i] + overflow;
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
		diff = (long)op1->blocks[i] - (long)op2->blocks[i]
		                            - borrow;
		//calculate borrow
		borrow = (diff & 0x80000000) >> 31;	
		
		//if borrow, offset. this is compiled to a cmove
		if(borrow) {
			diff += 0x200000000;
		}
		rop->blocks[i] = diff;
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



int main() {
	mpz *x, *y, *z;
	x = create_mpz(4);
	y = create_mpz(4);
	z = create_mpz(4);
	
	load_ui(x, 87,     3);
	load_ui(x, 99999,  2);
	load_ui(x, 198,    1);
	load_ui(x, 201302, 0);
	load_ui(y, 80,     3);
	load_ui(y, 100001, 2);
	load_ui(y, 200002, 1);
	load_ui(y, 33334,  0);

	print(x, 0);
	print(x, 1);
	print(y, 0);
	print(y, 1);
	
	printf("\n\n");
	
	subtract(z, x, y);

	print(z, 0);
	print(z, 1);
	

	return 0;
}








