#ifndef MP_H
#define MP_H

typedef struct {
	unsigned char *blocks; //little endian
	int block_count;
} mpz;

//basic
mpz *create_mpz(int bits, int block);
void destroy_mpz(mpz *op);
void clear_mpz(mpz *rop);

//load
void load_mpz_str(mpz *rop, char *in);
void load_ui(mpz *rop, unsigned int in);
void load(mpz *rop, const mpz *op1);

//bitwise
int  safe_bit_ct(char *in);
int  bit_count(const mpz *a);
void push_bit(mpz *rop, int bit);
void clear_bit(mpz *a, unsigned int x);
void set_bit(mpz *a, unsigned int x);
int get_bit(const mpz *a, unsigned int x);
void lower(mpz *a, int n);
int inv(mpz *rop);

//bit shift
void ls2(mpz *rop); //shift 1 left in-place
//shift 0<=n<8 bits left out-of-place
void ls3(mpz *rop, const mpz *op, unsigned int n);
//shift off8 blocks left, out-of-place
void ls4(mpz *rop, const mpz *op, int off8);

//unary
int inc(mpz *rop);
int dec(mpz *rop);

//binary arithmetic
int add(mpz *rop, const mpz *op, const mpz *op2);
void abs_sub(mpz *rop, const mpz *op1, const mpz *op2);
void mul(mpz *rop, const mpz *op1, const mpz *op2, mpz *tmp);
void idiv(mpz *rop, const mpz *op1, const mpz *op2, 
                                     mpz *rem, mpz *tmp);


int compare(const mpz *a, const mpz *b);
int compare1(const mpz *a);
int compare0(const mpz *a);

void swap(mpz *op1, mpz *op2);

void print_mpz(const mpz *rop);

#endif