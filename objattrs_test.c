

/* test for struct return */
typedef struct {
  short a, b;
} SmallStruct;

SmallStruct testStructReturn()
{
SmallStruct rval;
	rval.a=44; rval.b=55;
	return rval;
}

/* test for floating-point ABI */
double
testFloatABI(double arg)
{
}


#if __ALTIVEC__ > 0
/* test for vector ABI */
typedef int INT_VEC __attribute__(( vector_size(16) ));

extern void __testVectorABI(INT_VEC a1, INT_VEC a2);

void
testVectorABI(INT_VEC a1, INT_VEC a2)
{
	__testVectorABI(a1, a2);
}
#endif
