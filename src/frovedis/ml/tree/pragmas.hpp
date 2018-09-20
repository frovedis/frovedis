#ifndef __PRAGMAS__
#define __PRAGMAS__

#define DO_QUOTE(arg) #arg

#ifndef UNROLL_LEVEL
#define UNROLL_LEVEL 4
#endif

#if defined(__ve__)
#define __ivdep__ DO_QUOTE(_NEC ivdep)
#define __novector__ DO_QUOTE(_NEC novector)
#define __outerloop_unroll__ DO_QUOTE(_NEC outerloop_unroll(UNROLL_LEVEL))
#elif defined(_SX)
#define __ivdep__ DO_QUOTE(cdir nodep)
#define __novector__ DO_QUOTE(cdir novector)
#define __outerloop_unroll__ DO_QUOTE(cdir outerunroll=UNROLL_LEVEL)
#elif defined(__INTEL_COMPILER)
#define __ivdep__ DO_QUOTE(ivdep)
#define __novector__ DO_QUOTE(novector)
#define __outerloop_unroll__ DO_QUOTE(unroll(UNROLL_LEVEL))
#else
#define __ivdep__ DO_QUOTE(message(""))
#define __novector__ DO_QUOTE(message(""))
#define __outerloop_unroll__ DO_QUOTE(message(""))
#endif

#endif
