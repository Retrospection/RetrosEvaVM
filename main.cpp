#include "EvaVM.h"
#include "EvaValue.h"

int main() {
    EvaVM vm;

    auto result = vm.exec(R"(
        "hello,world!"
    )");

    log(result);

    std::cout << "All done!\n";

    return 0;
}
