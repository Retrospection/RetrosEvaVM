#include "EvaVM.h"
#include "EvaValue.h"

int main() {
    EvaVM vm;

    auto result = vm.exec(R"(
        (var i 10)
        (var count 0)
        (while (> i 0)
            (begin
                (set i (- i 1))
                (set count (+ count 1))))
        count
    )");

    log(result);

    std::cout << "All done!\n";

    return 0;
}
