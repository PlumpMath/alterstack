### Source dependances

1. Catch (test framework) https://github.com/philsquared/Catch
   test/include/catch.hpp
2. scontext (stack switch lib, part of boost::context)
   https://github.com/masterspline/scontext.git
   ext/scontext/
3. crash_log (log using mmap'ed file as temp buffer, which flushed to file
   when filled. This ensures, that in case of application crash, buffer 
   will be stored on disk by kernel. 
   Also this algorithm fast due to rare file writes)
   https://github.com/masterspline/crash_log.git
   ext/crash_log git submodule
4. cpu_utils (rdtsc() and other CPU-specific finctions)
   https://github.com/masterspline/cpu_utils.git
   ext/cpu_utils/ (as git submodule)
