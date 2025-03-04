#include <app/Counter.h>

#include <app/print.h>
#include <fmt/format.h>

#include <ostream>
#include <stdexcept>
#include <unordered_set>

auto singletonConstructedObjects() -> std::unordered_set<Counter::Obj const*>& {
    static std::unordered_set<Counter::Obj const*> data{};
    return data;
}

Counter::Obj::Obj()
    : mData(0)
    , mCounts(nullptr) {
    if (!singletonConstructedObjects().emplace(this).second) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    ++staticDefaultCtor;
}

Counter::Obj::Obj(const size_t& data, Counter& counts)
    : mData(data)
    , mCounts(&counts) {
    if (!singletonConstructedObjects().emplace(this).second) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    ++mCounts->ctor;
}

Counter::Obj::Obj(const Counter::Obj& o)
    : mData(o.mData)
    , mCounts(o.mCounts) {
    if (1 != singletonConstructedObjects().count(&o)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (!singletonConstructedObjects().emplace(this).second) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (nullptr != mCounts) {
        ++mCounts->copyCtor;
    }
}

Counter::Obj::Obj(Counter::Obj&& o) noexcept
    : mData(o.mData)
    , mCounts(o.mCounts) {
    if (1 != singletonConstructedObjects().count(&o)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (!singletonConstructedObjects().emplace(this).second) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (nullptr != mCounts) {
        ++mCounts->moveCtor;
    }
}

Counter::Obj::~Obj() {
    if (1 != singletonConstructedObjects().erase(this)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (nullptr != mCounts) {
        ++mCounts->dtor;
    } else {
        ++staticDtor;
    }
}

auto Counter::Obj::operator==(const Counter::Obj& o) const -> bool {
    if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (nullptr != mCounts) {
        ++mCounts->equals;
    }
    return mData == o.mData;
}

auto Counter::Obj::operator<(const Obj& o) const -> bool {
    if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (nullptr != mCounts) {
        ++mCounts->less;
    }
    return mData < o.mData;
}

// NOLINTNEXTLINE(bugprone-unhandled-self-assignment,cert-oop54-cpp)
auto Counter::Obj::operator=(const Counter::Obj& o) -> Counter::Obj& {
    if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    mCounts = o.mCounts;
    if (nullptr != mCounts) {
        ++mCounts->assign;
    }
    mData = o.mData;
    return *this;
}

auto Counter::Obj::operator=(Counter::Obj&& o) noexcept -> Counter::Obj& {
    if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    if (nullptr != o.mCounts) {
        mCounts = o.mCounts;
    }
    mData = o.mData;
    if (nullptr != mCounts) {
        ++mCounts->moveAssign;
    }
    return *this;
}

auto Counter::Obj::get() const -> size_t const& {
    if (nullptr != mCounts) {
        ++mCounts->constGet;
    }
    return mData;
}

auto Counter::Obj::get() -> size_t& {
    if (nullptr != mCounts) {
        ++mCounts->get;
    }
    return mData;
}

void Counter::Obj::swap(Obj& other) {
    if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&other)) {
        test::print("ERROR at {}({}): {}\n", __FILE__, __LINE__, __func__);
        std::abort();
    }
    using std::swap;
    swap(mData, other.mData);
    swap(mCounts, other.mCounts);
    if (nullptr != mCounts) {
        ++mCounts->swaps;
    }
}

auto Counter::Obj::getForHash() const -> size_t {
    if (nullptr != mCounts) {
        ++mCounts->hash;
    }
    return mData;
}

Counter::Counter() {
    Counter::staticDefaultCtor = 0;
    Counter::staticDtor = 0;
}

Counter::~Counter() {
    // check that all are destructed
    if (!singletonConstructedObjects().empty()) {
        test::print("ERROR at ~Counter(): got {} objects still alive!", singletonConstructedObjects().size());
        std::abort();
    }
}

auto Counter::total() const -> size_t {
    return ctor + staticDefaultCtor + copyCtor + (dtor + staticDtor) + equals + less + assign + swaps + get + constGet + hash +
           moveCtor + moveAssign;
}

void Counter::operator()(std::string_view title) {
    m_records += fmt::format("{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}|{:9}| {}\n",
                             ctor,
                             staticDefaultCtor,
                             copyCtor,
                             dtor + staticDtor,
                             assign,
                             swaps,
                             get,
                             constGet,
                             hash,
                             equals,
                             less,
                             moveCtor,
                             moveAssign,
                             total(),
                             title);
}

auto operator<<(std::ostream& os, Counter const& c) -> std::ostream& {
    return os << c.m_records;
}

auto operator new(size_t /*unused*/, Counter::Obj* /*unused*/) -> void* {
    throw std::runtime_error("operator new overload is taken! Cast to void* to ensure the void pointer overload is taken.");
}
size_t Counter::staticDefaultCtor = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
size_t Counter::staticDtor = 0;        // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
