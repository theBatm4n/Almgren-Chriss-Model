#include <iostream>
#include <chrono>

int main() {
    int clock[11] = {};

    auto start = std::chrono::high_resolution_clock::now();
    /*
    clock[0] = 0;
    clock[1] = 0;
    clock[2] = 0;
    clock[3] = 0;
    clock[4] = 0;
    clock[5] = 0;
    */
    for(int i= 0; i<=10; i++){
        clock[i] = 0;
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start);

    std::cout << "Execution time: " << duration.count() << " nanoseconds" << std::endl;
    
    return 0;

}