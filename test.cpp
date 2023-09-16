#include <cstdint>
#include <string>
#include <libos/fileio.h>
//start of runtime
typedef float float32_t;
typedef double float64_t;

namespace runtime
{
struct vec2
{
    float32_t x, y;
};

struct vec3
{
    float32_t x, y, z;
};

struct vec4
{
    float32_t x, y, z, w;
};

struct quat
{
    float32_t x, y, z, w;
};

struct matrix4
{
    float32_t mat[16];
};

template<typename T>
T de_ptr(T *ptr)
{
    return *ptr;
}

} // namespace runtime

//end of runtime
namespace test
{
const int8_t z = 6;
[[nodiscard]] const int8_t sub_test(const int8_t x,const int8_t y)
{
return x - y;
}
[[nodiscard]] const int8_t add_test(const int8_t x,const int8_t y)
{
const int8_t w = 5 * y / 7;
return x + w;
}
void test_two(int8_t* x,const int8_t& y)
{
const int32_t o = add_test(x,y);

}
int32_t main()
{
return add_test(1,2);
}

}
