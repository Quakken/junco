#include "foo.hpp"

int foo::bar::baz() noexcept {
    #if defined(JUNCO_BUILD_DEBUG)
        return -1;
    #elif defined(JUNCO_BUILD_RELEASE)
        return 1;
    #endif
}