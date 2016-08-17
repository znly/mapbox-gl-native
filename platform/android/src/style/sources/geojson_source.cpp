#include "geojson_source.hpp"

#include <mbgl/style/conversion.hpp>
#include "../conversion/geojson.hpp"

#include <string>

namespace mbgl {
namespace android {

    GeoJSONSource::GeoJSONSource(jni::JNIEnv& env, jni::String sourceId)
        : Source(env, std::make_unique<mbgl::style::GeoJSONSource>(jni::Make<std::string>(env, sourceId))) {
    }

    GeoJSONSource::GeoJSONSource(mbgl::Map& map, mbgl::style::GeoJSONSource& coreSource)
        : Source(map, coreSource) {
    }

    GeoJSONSource::~GeoJSONSource() = default;

    void GeoJSONSource::setGeoJSON(jni::JNIEnv& env, jni::Object<> json) {
        using namespace mbgl::style::conversion;

        //Convert the jni object
        Result<GeoJSON> converted = convert<GeoJSON>(Value(env, json));
        if(!converted) {
            mbgl::Log::Error(mbgl::Event::JNI, "Error setting geo json: " + converted.error().message);
            return;
        }

        //Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setGeoJSON(*converted);
    }

    void GeoJSONSource::setURL(jni::JNIEnv& env, jni::String url) {
        //Update the core source
        source.as<mbgl::style::GeoJSONSource>()->GeoJSONSource::setURL(jni::Make<std::string>(env, url));
    }

    jni::Class<GeoJSONSource> GeoJSONSource::javaClass;

    jni::jobject* GeoJSONSource::createJavaPeer(jni::JNIEnv& env) {
        static auto constructor = GeoJSONSource::javaClass.template GetConstructor<jni::jlong>(env);
        return GeoJSONSource::javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(this));
    }

    void GeoJSONSource::registerNative(jni::JNIEnv& env) {
        //Lookup the class
        GeoJSONSource::javaClass = *jni::Class<GeoJSONSource>::Find(env).NewGlobalRef(env).release();

        #define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

        //Register the peer
        jni::RegisterNativePeer<GeoJSONSource>(
            env, GeoJSONSource::javaClass, "nativePtr",
            std::make_unique<GeoJSONSource, JNIEnv&, jni::String>,
            "initialize",
            "finalize",
            METHOD(&GeoJSONSource::setGeoJSON, "nativeSetGeoJson"),
            METHOD(&GeoJSONSource::setGeoJSON, "nativeSetUrl")
        );
    }

} // namespace android
} // namespace mbgl
