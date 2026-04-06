# to be implemented

```cpp
import std;

int main(){
    int age {16};

    try {
        if (age < 18){
            throw std::runtime_error("u gotta be adult!");
        }
    } catch (const std::exception &e) {
        std::println("Error: {}", e.what());
    }

    return 0;
}
```