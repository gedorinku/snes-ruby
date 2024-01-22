#undef assert

#ifdef NDEBUG
#define	assert(x) (void)0
#else
#define assert(x) ((void)((x) || (hal_write(0, #x, strlen(#x)) && exit(1))))
#endif
