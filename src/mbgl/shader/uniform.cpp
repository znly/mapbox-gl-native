#include <mbgl/shader/uniform.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {

template <>
void Uniform<GLfloat>::bind() {
    MBGL_CHECK_ERROR(glUniform1f(location, current));
}

template <>
void Uniform<GLint>::bind() {
    MBGL_CHECK_ERROR(glUniform1i(location, current));
}

template <>
void Uniform<std::array<GLfloat, 2>>::bind() {
    MBGL_CHECK_ERROR(glUniform2fv(location, 1, current.data()));
}

template <>
void Uniform<std::array<GLfloat, 3>>::bind() {
    MBGL_CHECK_ERROR(glUniform3fv(location, 1, current.data()));
}

template <>
void Uniform<std::array<GLfloat, 4>>::bind() {
    MBGL_CHECK_ERROR(glUniform4fv(location, 1, current.data()));
}

template <>
void Uniform<Color>::bind() {
    MBGL_CHECK_ERROR(glUniform4f(location, current.r, current.g, current.b, current.a));
}

template <>
void UniformMatrix<2>::bind() {
    MBGL_CHECK_ERROR(glUniformMatrix2fv(location, 1, GL_FALSE, current.data()));
}

template <>
void UniformMatrix<3>::bind() {
    MBGL_CHECK_ERROR(glUniformMatrix3fv(location, 1, GL_FALSE, current.data()));
}

template <>
void UniformMatrix<4>::bind() {
    MBGL_CHECK_ERROR(glUniformMatrix4fv(location, 1, GL_FALSE, current.data()));
}

// Add more as needed.

} // namespace mbgl
