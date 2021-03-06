#include "../../frozen/meta/string.hpp"
#include "../../frozen/string.hpp"

#include <type_traits>

namespace meta = frozen::meta;

using fizz = FROZEN_FROM_STRING_LITERAL("fizz");
using buzz = FROZEN_FROM_STRING_LITERAL("buzz");

template< size_t Start,
          size_t Last,
          class Acc = meta::string<>,
          size_t Mod3 = Start%3,
          size_t Mod5 = Start%5,
          bool Finish = (Start>=Last) >
struct fizzbuzz{
    typedef Acc type;
};

template< size_t Start,
          size_t Last,
          class Acc >
struct fizzbuzz<Start, Last, Acc, 0, 0, false>
        : fizzbuzz<Start+1, Last, meta::add_newline<meta::concat<meta::concat<Acc, fizz>, buzz>> >{};

template< size_t Start,
          size_t Last,
          class Acc,
          size_t Mod5 >
struct fizzbuzz<Start, Last, Acc, 0, Mod5, false>
        : fizzbuzz<Start+1, Last, meta::add_newline<meta::concat<Acc, fizz>>>{};

template< size_t Start,
          size_t Last,
          class Acc,
          size_t Mod3 >
struct fizzbuzz<Start, Last, Acc, Mod3, 0, false>
        : fizzbuzz<Start+1, Last, meta::add_newline<meta::concat<Acc, buzz>> >{};

template< size_t Start,
          size_t Last,
          class Acc,
          size_t Mod3,
          size_t Mod5 >
struct fizzbuzz<Start, Last, Acc, Mod3, Mod5, false>
        : fizzbuzz<Start+1, Last, meta::add_newline<meta::concat<Acc, FROZEN_TO_STRING(Start)>> >{};

using result = FROZEN_FROM_STRING_LITERAL("1\n2\nfizz\n4\nbuzz\nfizz\n7\n8\nfizz\nbuzz\n11\nfizz\n13\n14\nfizzbuzz\n");

static_assert(std::is_same<typename fizzbuzz<1, 16>::type, result>::value, "");

#include <iostream>
int main()
{
    std::cout << fizzbuzz<1, 15>::type::value << '\n';
    return 0;
}
