#include "EvaVM.h"
#include "EvaValue.h"

int main() {
    EvaVM vm;

    auto result = vm.exec(R"(
        (var x 5)
        (set x (+ x 10))
        x
        (begin
            (var z 100)
            (set x 1000)
            (begin
                (var x 200)
                x)
            x)
        x
    )");

    log(result);

    std::cout << "All done!\n";

    return 0;
}
