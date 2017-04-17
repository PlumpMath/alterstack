### Source dependances

1. Catch (test framework) https://github.com/philsquared/Catch
   test/include/catch.hpp
2. scontext (stack switch lib, part of boost::context)
   https://github.com/masterspline/scontext.git
   ext/scontext/
3. cpu_utils (rdtsc() and other CPU-specific finctions)
   https://github.com/masterspline/cpu_utils.git
   ext/cpu_utils/ (as git submodule)
4. hwloc (detect cpu caches topology) https://github.com/open-mpi/hwloc
   https://github.com/masterspline/hwloc.git
   ext/hwloc (git submodule)
