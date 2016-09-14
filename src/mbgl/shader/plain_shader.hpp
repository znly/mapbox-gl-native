#pragma once

#include <mbgl/shader/shader.hpp>
#include <mbgl/shader/uniform.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/traits.hpp>

namespace mbgl {

template <typename U>
struct UniformValue {
    const typename U::ArgType& value;
};

class PlainShader : public Shader {
public:
    PlainShader(gl::ObjectStore&, Defines defines = None);

    void bind(GLbyte *offset) final;

    struct matrix : public UniformMatrix<4> {
        matrix() : UniformMatrix{ "u_matrix" } {};
    };
    struct color : public Uniform<Color> {
        color() : Uniform{ "u_color" } {};
    };
    struct opacity : public Uniform<GLfloat> {
        opacity() : Uniform{ "u_opacity" } {};
    };

    template <typename... Args>
    void setUniforms(Args&&... args) {
        auto x = std::make_tuple(std::forward<Args>(args)...);
        apply(uniforms, [this, &x](auto& uniform) {
            using U = std::remove_cv_t<std::remove_reference_t<decltype(uniform)>>;
            uniform.set(get_tuple_type<UniformValue<U>>({ {} }, std::forward<decltype(x)>(x)).value);
        });
    }

private:
    std::tuple<color, opacity, matrix> uniforms;
};

} // namespace mbgl
