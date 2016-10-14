#pragma once

#include <mbgl/gl/shader.hpp>
#include <mbgl/gl/attribute.hpp>
#include <mbgl/gl/uniform.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {

class RasterVertex;

class TerrainShader : public gl::Shader {
public:
    TerrainShader(gl::Context&, Defines defines = None);

    using VertexType = RasterVertex;

    gl::Attribute<int16_t, 2> a_pos = { "a_pos", *this };
    gl::Attribute<uint16_t, 2> a_texture_pos = { "a_texture_pos", *this };

    gl::UniformMatrix<4> u_matrix = { "u_matrix", *this };
    gl::Uniform<int32_t> u_image = { "u_image", *this };

    gl::Uniform<int32_t> u_mode = { "u_mode", *this };
    gl::Uniform<std::array<float, 2>> u_dimension = { "u_dimension", *this };
    gl::Uniform<float> u_zoom = { "u_zoom", *this };
    gl::Uniform<float> u_azimuth = { "u_azimuth", *this };
    gl::Uniform<float> u_zenith = { "u_zenith", *this };
    gl::Uniform<float> u_mipmap = { "u_mipmap", *this };
    gl::Uniform<float> u_exaggeration = { "u_exaggeration", *this };
    gl::Uniform<Color> u_shadow = { "u_shadow", *this };
    gl::Uniform<Color> u_highlight = { "u_highlight", *this };
    gl::Uniform<Color> u_accent = { "u_accent", *this };
};

} // namespace mbgl
