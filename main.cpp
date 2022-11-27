#include "EvaVM.h"
#include "EvaValue.h"

int main() {
    EvaVM vm;

    auto result = vm.exec(R"(
        (if (> 5 10) 1 2)
    )");

    log(result);

    std::cout << "All done!\n";

    return 0;
}
