#include <executor.h>
#include <coroutine>
#include <memory>

class System {
private:
    std::shared_ptr<Executor::Executor> executor_;
public:
    System(std::shared_ptr<Executor::Executor> executor)
        : executor_(std::move(executor)) {}
    void tick();
};