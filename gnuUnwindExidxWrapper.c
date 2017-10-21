void *
__real___gnu_Unwind_Find_exidx(void *pc, int *pNumEntries);

void *
__cexp_Unwind_Find_exidx(void *pc, int *pNumEntries);

void *
__wrap___gnu_Unwind_Find_exidx(void *pc, int *pNumEntries)
{
void *rval = __cexp_Unwind_Find_exidx(pc, pNumEntries);

	return rval ? rval : __real___gnu_Unwind_Find_exidx(pc, pNumEntries);
}
