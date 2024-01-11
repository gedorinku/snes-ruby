#undef assert

#ifdef NDEBUG
#define	assert(x) (void)0
#else
// FIXME: assert is not supported
#define	assert(x) (void)0
// #define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__),0)))
#endif
