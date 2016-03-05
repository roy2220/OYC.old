#pragma once


#include <functional>
#include <utility>


namespace OYC {

class ScopeGuard final
{
    ScopeGuard(const ScopeGuard &) = delete;
    ScopeGuard &operator=(const ScopeGuard &) = delete;

public:
    inline explicit ScopeGuard(const std::function<void ()> &);
    inline explicit ScopeGuard(std::function<void ()> &&);
    inline ~ScopeGuard();

    inline void commit() noexcept;
    inline void dismiss() noexcept;

private:
    const std::function<void ()> rollback_;
    bool isEngaged_;
};


ScopeGuard::ScopeGuard(const std::function<void ()> &rollback)
  : rollback_(rollback),
    isEngaged_(false)
{
}


ScopeGuard::ScopeGuard(std::function<void ()> &&rollback)
  : rollback_(std::move(rollback)),
    isEngaged_(false)
{
}


ScopeGuard::~ScopeGuard()
{
    if (isEngaged_) {
        rollback_();
    }
}


void
ScopeGuard::commit() noexcept
{
    isEngaged_ = true;
}


void
ScopeGuard::dismiss() noexcept
{
    isEngaged_ = false;
}

} // namespace OYC
