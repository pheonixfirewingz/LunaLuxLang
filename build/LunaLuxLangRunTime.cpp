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