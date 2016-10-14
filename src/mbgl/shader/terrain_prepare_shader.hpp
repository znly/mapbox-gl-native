#pragma once

#include <mbgl/gl/shader.hpp>
#include <mbgl/gl/attribute.hpp>
#include <mbgl/gl/uniform.hpp>

namespace mbgl {

class RasterVertex;

class TerrainPrepareShader : public gl::Shader {
public:
    TerrainPrepareShader(gl::Context&, Defines defines = None);

    using VertexType = RasterVertex;

    gl::Attribute<int16_t, 2>  a_pos         = { "a_pos",         *this };
    gl::Attribute<uint16_t, 2> a_texture_pos = { "a_texture_pos", *this };

    gl::UniformMatrix<4>              u_matrix    = {"u_matrix",    *this};
    gl::Uniform<int32_t>              u_image     = {"u_image",     *this};
    gl::Uniform<std::array<float, 2>> u_dimension = {"u_dimension", *this};
    gl::Uniform<float>                u_zoom      = {"u_zoom",      *this};
};

} // namespace mbgl
