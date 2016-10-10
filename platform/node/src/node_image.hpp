#pragma once

#include <mbgl/util/image.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <nan.h>
#pragma GCC diagnostic pop

namespace node_mbgl {

class NodeImage : public Nan::ObjectWrap {
public:
    NodeImage(v8::Local<v8::Object>);
    ~NodeImage();

    static Nan::Persistent<v8::Function> constructor;

    static void Init(v8::Local<v8::Object> exports);

    static void New(const Nan::FunctionCallbackInfo<v8::Value>&);

    std::unique_ptr<mbgl::PremultipliedImage> image;
};

}
