#pragma once

#include <mbgl/shader/shader.hpp>
#include <mbgl/gl/gl.hpp>

#include <array>

namespace mbgl {

template <typename T>
class Uniform {
public:
    using Type = T;
    using ArgType = T;

    Uniform(const GLchar* name_, const Shader& shader) : name(name_), current() {
        location = MBGL_CHECK_ERROR(glGetUniformLocation(shader.getID(), name));
    }

    Uniform(const GLchar* name_) : name(name_), current() {
    }

    void operator=(const ArgType& t) {
        if (current != t) {
            current = t;
            bind();
        }
    }

    void set(const ArgType& t) {
        operator=(t);
    }

    void bind();

    const char* const name;
    Type current;
    GLint location = -1;
};

template <size_t C, size_t R = C>
class UniformMatrix {
public:
    using Type = std::array<float, C*R>;
    using ArgType = std::array<double, C*R>;

    UniformMatrix(const GLchar* name_, const Shader& shader) : name(name_), current() {
        location = MBGL_CHECK_ERROR(glGetUniformLocation(shader.getID(), name));
    }

    UniformMatrix(const GLchar* name_) : name(name_), current() {
    }

    void operator=(const ArgType& t) {
        bool dirty = false;
        for (unsigned int i = 0; i < C*R; i++) {
            if (current[i] != t[i]) {
                current[i] = t[i];
                dirty = true;
            }
        }
        if (dirty) {
            bind();
        }
    }

    void set(const ArgType& t) {
        operator=(t);
    }

    void bind();

    const char* const name;
    Type current;
    GLint location = -1;
};

} // namespace mbgl
