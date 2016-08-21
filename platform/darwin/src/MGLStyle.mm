#import "MGLStyle.h"

#import "MGLMapView_Private.h"
#import "MGLStyleLayer.h"
#import "MGLFillStyleLayer.h"
#import "MGLLineStyleLayer.h"
#import "MGLCircleStyleLayer.h"
#import "MGLSymbolStyleLayer.h"
#import "MGLRasterStyleLayer.h"
#import "MGLBackgroundStyleLayer.h"

#import "MGLStyle_Private.h"
#import "MGLStyleLayer_Private.h"
#import "MGLSource_Private.h"

#import "NSDate+MGLAdditions.h"

#import "MGLSource.h"
#import "MGLVectorSource.h"
#import "MGLRasterSource.h"
#import "MGLGeoJSONSource.h"

#include <mbgl/util/default_styles.hpp>
#include <mbgl/sprite/sprite_image.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/sources/geojson_source.hpp>
#include <mbgl/style/sources/vector_source.hpp>
#include <mbgl/style/sources/raster_source.hpp>
#include <mbgl/mbgl.hpp>

#if TARGET_OS_IPHONE
    #import "UIImage+MGLAdditions.h"
#else
    #import "NSImage+MGLAdditions.h"
#endif

@interface MGLStyle()

@property (nonatomic, readwrite, weak) MGLMapView *mapView;
@property (readonly, copy, nullable) NSURL *URL;

@end

@implementation MGLStyle

#pragma mark Default style URLs

static_assert(mbgl::util::default_styles::currentVersion == MGLStyleDefaultVersion, "mbgl::util::default_styles::currentVersion and MGLStyleDefaultVersion disagree.");

/// @param name The style’s marketing name, written in lower camelCase.
/// @param fileName The last path component in the style’s URL, excluding the version suffix.
#define MGL_DEFINE_STYLE(name, fileName) \
    static NSURL *MGLStyleURL_##name; \
    + (NSURL *)name##StyleURL { \
        static dispatch_once_t onceToken; \
        dispatch_once(&onceToken, ^{ \
            MGLStyleURL_##name = [self name##StyleURLWithVersion:8]; \
        }); \
        return MGLStyleURL_##name; \
    } \
    \
    + (NSURL *)name##StyleURL##WithVersion:(NSInteger)version { \
        return [NSURL URLWithString:[@"mapbox://styles/mapbox/" #fileName "-v" stringByAppendingFormat:@"%li", (long)version]]; \
    }

MGL_DEFINE_STYLE(streets, streets)
MGL_DEFINE_STYLE(outdoors, outdoors)
MGL_DEFINE_STYLE(light, light)
MGL_DEFINE_STYLE(dark, dark)
MGL_DEFINE_STYLE(satellite, satellite)
MGL_DEFINE_STYLE(satelliteStreets, satellite-streets)

// Make sure all the styles listed in mbgl::util::default_styles::orderedStyles
// are defined above and also declared in MGLStyle.h.
static_assert(6 == mbgl::util::default_styles::numOrderedStyles,
              "mbgl::util::default_styles::orderedStyles and MGLStyle have different numbers of styles.");

// Hybrid has been renamed Satellite Streets, so the last Hybrid version is hard-coded here.
static NSURL *MGLStyleURL_hybrid;
+ (NSURL *)hybridStyleURL {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        MGLStyleURL_hybrid = [NSURL URLWithString:@"mapbox://styles/mapbox/satellite-hybrid-v8"];
    });
    return MGLStyleURL_hybrid;
}

// Emerald is no longer getting new versions as a default style, so the current version is hard-coded here.
static NSURL *MGLStyleURL_emerald;
+ (NSURL *)emeraldStyleURL {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        MGLStyleURL_emerald = [NSURL URLWithString:@"mapbox://styles/mapbox/emerald-v8"];
    });
    return MGLStyleURL_emerald;
}

#pragma mark -

- (instancetype)initWithMapView:(MGLMapView *)mapView {
    if (self = [super init]) {
        _mapView = mapView;
    }
    return self;
}

- (NSURL *)URL {
    return [NSURL URLWithString:@(self.mapView.mbglMap->getStyleURL().c_str())];
}

- (NSString *)name {
    return @(self.mapView.mbglMap->getStyleName().c_str());
}

#pragma mark Sources

- (MGLSource *)sourceWithIdentifier:(NSString *)identifier
{
    auto mbglSource = self.mapView.mbglMap->getSource(identifier.UTF8String);
    if (!mbglSource) {
        return nil;
    }

    // TODO: Fill in options specific to the respective source classes
    // https://github.com/mapbox/mapbox-gl-native/issues/6584
    MGLSource *source;
    if (mbglSource->is<mbgl::style::VectorSource>()) {
        source = [[MGLVectorSource alloc] initWithIdentifier:identifier];
    } else if (mbglSource->is<mbgl::style::GeoJSONSource>()) {
        source = [[MGLGeoJSONSource alloc] initWithIdentifier:identifier];
    } else if (mbglSource->is<mbgl::style::RasterSource>()) {
        source = [[MGLRasterSource alloc] initWithIdentifier:identifier];
    } else {
        source = [[MGLSource alloc] initWithIdentifier:identifier];
    }

    source.source = mbglSource;

    return source;
}

- (void)addSource:(MGLSource *)source
{
    self.mapView.mbglMap->addSource(source.mbglSource);
}

- (void)removeSource:(MGLSource *)source
{
    self.mapView.mbglMap->removeSource(source.identifier.UTF8String);
}

#pragma mark Style layers

- (NS_MUTABLE_ARRAY_OF(MGLStyleLayer *) *)layers
{
    auto layers = self.mapView.mbglMap->getLayers();
    NSMutableArray *styleLayers = [NSMutableArray arrayWithCapacity:layers.size()];
    for (auto &layer : layers) {
        MGLStyleLayer *styleLayer = [self layerFromMBGLLayer:layer];
        [styleLayers addObject:styleLayer];
    }
    return styleLayers;
}

- (void)setLayers:(NS_MUTABLE_ARRAY_OF(MGLStyleLayer *) *)layers {
    std::vector<mbgl::style::Layer *> rawLayers;
    rawLayers.reserve(layers.count);
    for (MGLStyleLayer *layer in layers) {
        rawLayers.push_back(layer.layer);
    }
    self.mapView.mbglMap->setLayers(rawLayers);
}

- (NSUInteger)countOfLayers
{
    return self.mapView.mbglMap->getLayers().size();
}

- (MGLStyleLayer *)objectInLayersAtIndex:(NSUInteger)index
{
    auto layers = self.mapView.mbglMap->getLayers();
    auto layer = layers.at(index);
    if (layer) {
        return [self layerFromMBGLLayer:layer];
    } else {
        [NSException raise:NSRangeException
                    format:@"No style layer at index %lu", (unsigned long)index];
        return nil;
    }
}

- (void)getLayers:(MGLStyleLayer **)buffer range:(NSRange)inRange
{
    auto layers = self.mapView.mbglMap->getLayers();
    NSUInteger i = 0;
    for (auto layer = *(layers.begin() + inRange.location); i < inRange.length; ++layer, ++i) {
        MGLStyleLayer *styleLayer = [self layerFromMBGLLayer:layer];
        buffer[i] = styleLayer;
    }
}

- (void)insertObject:(MGLStyleLayer *)styleLayer inLayersAtIndex:(NSUInteger)index
{
    auto layers = self.mapView.mbglMap->getLayers();
    if (index == layers.size()) {
        [self addLayer:styleLayer];
    } else {
        auto layerAbove = layers.at(index);
        self.mapView.mbglMap->addLayer(std::unique_ptr<mbgl::style::Layer>(styleLayer.layer), layerAbove->getID());
    }
}

- (void)removeObjectFromLayersAtIndex:(NSUInteger)index
{
    auto layers = self.mapView.mbglMap->getLayers();
    auto layer = layers.at(index);
    self.mapView.mbglMap->removeLayer(layer->getID());
}

- (MGLStyleLayer *)layerFromMBGLLayer:(mbgl::style::Layer *)mbglLayer
{
    NSParameterAssert(mbglLayer);
    
    NSString *identifier = @(mbglLayer->getID().c_str());
    MGLStyleLayer *styleLayer;
    if (auto fillLayer = mbglLayer->as<mbgl::style::FillLayer>()) {
        MGLSource *source = [self sourceWithIdentifier:@(fillLayer->getSourceID().c_str())];
        styleLayer = [[MGLFillStyleLayer alloc] initWithIdentifier:identifier source:source];
    } else if (auto lineLayer = mbglLayer->as<mbgl::style::LineLayer>()) {
        MGLSource *source = [self sourceWithIdentifier:@(lineLayer->getSourceID().c_str())];
        styleLayer = [[MGLLineStyleLayer alloc] initWithIdentifier:identifier source:source];
    } else if (auto symbolLayer = mbglLayer->as<mbgl::style::SymbolLayer>()) {
        MGLSource *source = [self sourceWithIdentifier:@(symbolLayer->getSourceID().c_str())];
        styleLayer = [[MGLSymbolStyleLayer alloc] initWithIdentifier:identifier source:source];
    } else if (auto rasterLayer = mbglLayer->as<mbgl::style::RasterLayer>()) {
        MGLSource *source = [self sourceWithIdentifier:@(rasterLayer->getSourceID().c_str())];
        styleLayer = [[MGLRasterStyleLayer alloc] initWithIdentifier:identifier source:source];
    } else if (auto circleLayer = mbglLayer->as<mbgl::style::CircleLayer>()) {
        MGLSource *source = [self sourceWithIdentifier:@(circleLayer->getSourceID().c_str())];
        styleLayer = [[MGLCircleStyleLayer alloc] initWithIdentifier:identifier source:source];
    } else if (mbglLayer->as<mbgl::style::BackgroundLayer>()) {
        styleLayer = [[MGLBackgroundStyleLayer alloc] initWithIdentifier:identifier];
    } else {
        NSAssert(NO, @"Unrecognized layer type");
        return nil;
    }
    
    styleLayer.layer = mbglLayer;
    
    return styleLayer;
}

- (MGLStyleLayer *)layerWithIdentifier:(NSString *)identifier
{
    auto mbglLayer = self.mapView.mbglMap->getLayer(identifier.UTF8String);
    return mbglLayer ? [self layerFromMBGLLayer:mbglLayer] : nil;
}

- (void)removeLayer:(MGLStyleLayer *)layer
{
    [self willChangeValueForKey:@"layers"];
    self.mapView.mbglMap->removeLayer(layer.identifier.UTF8String);
    [self didChangeValueForKey:@"layers"];
}

- (void)addLayer:(MGLStyleLayer *)layer
{
    if (!layer.layer) {
        [NSException raise:NSInvalidArgumentException format:
         @"The style layer %@ cannot be added to the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MGLStyleLayer.",
         layer];
    }

    [self willChangeValueForKey:@"layers"];
    self.mapView.mbglMap->addLayer(std::unique_ptr<mbgl::style::Layer>(layer.layer));
    [self didChangeValueForKey:@"layers"];
}

- (void)insertLayer:(MGLStyleLayer *)layer belowLayer:(MGLStyleLayer *)otherLayer
{
    if (!layer.layer) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"The style layer %@ cannot be added to the style. "
         @"Make sure the style layer was created as a member of a concrete subclass of MGLStyleLayer.",
         layer];
    }
    if (!otherLayer.layer) {
        [NSException raise:NSInvalidArgumentException
                    format:
         @"A style layer cannot be placed before %@ in the style. "
         @"Make sure otherLayer was obtained using -[MGLStyle layerWithIdentifier:].",
         otherLayer];
    }

    [self willChangeValueForKey:@"layers"];
    const mbgl::optional<std::string> belowLayerId { otherLayer.identifier.UTF8String };
    self.mapView.mbglMap->addLayer(std::unique_ptr<mbgl::style::Layer>(layer.layer), belowLayerId);
    [self didChangeValueForKey:@"layers"];
}

#pragma mark Style classes

- (NS_ARRAY_OF(NSString *) *)styleClasses
{
    const std::vector<std::string> &appliedClasses = self.mapView.mbglMap->getClasses();

    NSMutableArray *returnArray = [NSMutableArray arrayWithCapacity:appliedClasses.size()];

    for (auto appliedClass : appliedClasses) {
       [returnArray addObject:@(appliedClass.c_str())];
    }

    return returnArray;
}

- (void)setStyleClasses:(NS_ARRAY_OF(NSString *) *)appliedClasses
{
    [self setStyleClasses:appliedClasses transitionDuration:0];
}

- (void)setStyleClasses:(NS_ARRAY_OF(NSString *) *)appliedClasses transitionDuration:(NSTimeInterval)transitionDuration
{
    std::vector<std::string> newAppliedClasses;

    for (NSString *appliedClass in appliedClasses)
    {
        newAppliedClasses.push_back([appliedClass UTF8String]);
    }

    mbgl::style::TransitionOptions transition { { MGLDurationInSeconds(transitionDuration) } };
    self.mapView.mbglMap->setTransitionOptions(transition);
    self.mapView.mbglMap->setClasses(newAppliedClasses);
}

- (BOOL)hasStyleClass:(NSString *)styleClass
{
    return styleClass && self.mapView.mbglMap->hasClass([styleClass UTF8String]);
}

- (void)addStyleClass:(NSString *)styleClass
{
    if (styleClass)
    {
        self.mapView.mbglMap->addClass([styleClass UTF8String]);
    }
}

- (void)removeStyleClass:(NSString *)styleClass
{
    if (styleClass)
    {
        self.mapView.mbglMap->removeClass([styleClass UTF8String]);
    }
}

#pragma mark Style images

- (void)setImage:(MGLImage *)image forName:(NSString *)name
{
    NSAssert(image, @"image is null");
    NSAssert(name, @"name is null");

    self.mapView.mbglMap->addImage([name UTF8String], image.mgl_spriteImage);
}

- (void)removeImageForName:(NSString *)name
{
    NSAssert(name, @"name is null");

    self.mapView.mbglMap->removeImage([name UTF8String]);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; name = %@, URL = %@>",
            NSStringFromClass([self class]), (void *)self,
            self.name ? [NSString stringWithFormat:@"\"%@\"", self.name] : self.name,
            self.URL ? [NSString stringWithFormat:@"\"%@\"", self.URL] : self.URL];
}

@end
