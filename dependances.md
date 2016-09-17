### Source dependances

1. Catch (test framework) https://github.com/philsquared/Catch
   test/include/catch.hpp
2. scontext (stack switch lib, part of boost::context)
   https://github.com/masterspline/scontext.git
   ext/scontext/
3. crash_log (журнал, который пишет в mmap'ed на файл область памяти и сбрасывает
   содержимое при заполнении, что дает гарантию сохраниния данных при падении приложения
   и работает быстро, т.к. запись в файл происходит редко)
   https://github.com/masterspline/crash_log.git
   ext/crash_log git submodule
