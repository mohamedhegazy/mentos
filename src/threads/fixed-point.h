/* p.q format to represent fixed point numbers */

#define p 17	
#define q 14
#define f (1 << 14)

int convert_to_fp(int);
int convert_to_integer_zero_round(int);
int convert_to_integer_nearest(int);
int add_fp(int,int);
int subtract_fp(int,int);
int add_fp_integer(int,int);
int subtract_fp_integer(int,int);
int mult_fp(int,int);
int mult_fp_integer(int,int);
int divide_fp(int,int);
int divide_fp_integer(int,int);

int convert_to_fp(int n){
return n * f;
}

int convert_to_integer_zero_round(int x){
return x / f;
}

int convert_to_integer_nearest(int x){
if(x>=0){
return (x + f / 2 ) / f;
}
return (x - f / 2 ) / f;
}

int add_fp(int x,int y){
return x + y;
}

int subtract_fp(int x,int y){
return x - y;
}

int add_fp_integer(int x,int n){
return x + n*f;
}

int subtract_fp_integer(int x,int n){
return x - n*f;
}

int mult_fp(int x,int y){
return ((uint64_t) x) * y / f; 
}
int mult_fp_integer(int x,int n){
return x * n;
}
int divide_fp(int x,int y){
return ((uint64_t) x) * f / y;
}

int divide_fp_integer(int x,int n){
return x / n;
}

