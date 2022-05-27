#ifndef MP_H
#define MP_H

typedef struct {
	unsigned int *blocks; //little endian
	int block_count;
} mpz;

mpz *create_mpz(unsigned int block_ct);
void destroy_mpz(mpz *this);
void load(mpz *rop, const mpz *op);
void load_ui(mpz *rop, unsigned int op, unsigned int off8);

//BOE >  0: Create with BOE blocks
//BOE =  0: Create with exactly enough blocks to repr
//BOE = -1: Create with DOUBLE  enough blocks to repr
void load_create(mpz **rop, const char *string,
                            int block_or_enum);

int compare(const mpz *op1, const mpz *op2);
int compare0(const mpz *op);
int compare1(const mpz *op);

void swap(mpz *op1, mpz *op2);

void zero(mpz *op);

void add(mpz *rop, const mpz *op1, const mpz *op2);
void add_ui(mpz *rop, const mpz *op1, unsigned int op2);
void add_lu_off(mpz *rop, const mpz *op1, unsigned long op2,
                                          int off8);
void subtract(mpz *rop, const mpz *op1, const mpz *op2);
void difference(mpz *rop, const mpz *op1, const mpz *op2);
void mul(mpz *rop, const mpz *op1, const mpz *op2);
void idiv(mpz *q, mpz *r, const mpz *n, const mpz *d);

int get_bit(const mpz *a, unsigned int x);
unsigned int ctz(const mpz *op);
void ls(mpz *rop, const mpz *op, unsigned int bits);
void ls2(mpz *rop, const mpz *op, int off8);
void ls3(mpz *rop, const mpz *op, unsigned int bits);
void rs(mpz *rop, const mpz *op, unsigned int bits);
void rs2(mpz *rop, const mpz *op, unsigned int off8);
void rs3(mpz *rop, const mpz *op, unsigned int bits);

void gcd(mpz *u, mpz *v, mpz *scratch);

void print(const mpz *x, int decimal);
void print_ui(unsigned int x, char *buf);

#endif