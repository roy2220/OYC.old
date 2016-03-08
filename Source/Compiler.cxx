#include "Compiler.h"

#include <string>
#include <vector>


namespace OYC {

class CompilationContext final
{
    CompilationContext(const CompilationContext &) = delete;
    CompilationContext &operator=(const CompilationContext &) = delete;

public:
    explicit CompilationContext(CompilationContext *);

    int getSuperRegisterID(const std::string *) const;
    int getNumberOfRegisters() const;
    void addRegister(const std::string *);
    void deleteRegisters(int);
    void pushRegisterID(const std::string *);
    void pushRegisterID();
    int popRegisterID();

private:
    CompilationContext *const super_;
    std::vector<const std::string *> registerIDToName_;
    std::vector<int> registerIDs_;

    int getRegisterID(const std::string *) const;
};


CompilationContext::CompilationContext(CompilationContext *super)
  : super_(super)
{
}


int
CompilationContext::getSuperRegisterID(const std::string *superRegisterName) const
{
    return super_->getRegisterID(superRegisterName);
}


int
CompilationContext::getNumberOfRegisters() const
{
    return static_cast<int>(registerIDToName_.size());
}


void
CompilationContext::addRegister(const std::string *registerName)
{
    registerIDToName_.push_back(registerName);
}


void
CompilationContext::deleteRegisters(int numberOfRegisters)
{
    registerIDToName_.erase(registerIDToName_.begin() + numberOfRegisters, registerIDToName_.end());
}


void
CompilationContext::pushRegisterID(const std::string *registerName)
{
    registerIDs_.push_back(getRegisterID(registerName));
}


void
CompilationContext::pushRegisterID()
{
    addRegister(nullptr);
    registerIDs_.push_back(static_cast<int>(registerIDToName_.size()) - 1);
}


int
CompilationContext::popRegisterID()
{
    int registerID = registerIDs_.back();
    registerIDs_.pop_back();

    if (registerIDToName_[registerID] == nullptr) {
        registerIDToName_.pop_back();
    }

    return registerID;
}


int
CompilationContext::getRegisterID(const std::string *registerName) const
{
    for (int registerID = static_cast<int>(registerIDToName_.size()) - 1;; --registerID) {
        if (registerIDToName_[registerID] == registerName) {
            return registerID;
        }
    }
}

} // namespace OYC
