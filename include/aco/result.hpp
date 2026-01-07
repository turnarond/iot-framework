// include/aco/result.hpp
#include <string>
namespace aco {
template<typename T>
struct Result {
    bool success;
    T value;
    std::string error_msg;

    static Result<T> ok(T&& v) { return {true, std::move(v), {}}; }
    static Result<T> error(std::string msg) { return {false, {}, std::move(msg)}; }

    explicit operator bool() const { return success; }
};
}