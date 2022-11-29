#include <memory>
#include <string>

namespace clanguml {
namespace t20009 {
template <typename T> struct A {
    void a(T arg) { }
};

template <typename T> struct B {
    void b(T arg) { a->a(arg); }

    std::unique_ptr<A<T>> a;
};

void tmain()
{
    std::shared_ptr<B<std::string>> bstring;
    auto bint = std::make_unique<B<int>>();

    bstring->b("b");
    bint.get()->b(42);
}
}
}