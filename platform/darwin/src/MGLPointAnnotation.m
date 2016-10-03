#import "MGLPointAnnotation.h"

@implementation MGLPointAnnotation

@synthesize coordinate;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if (self = [super initWithCoder:coder]) {
        self.coordinate = CLLocationCoordinate2DMake([coder decodeDoubleForKey:@"coordinateLatitude"],
                                                     [coder decodeDoubleForKey:@"coordinateLongitude"]);
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeDouble:coordinate.latitude forKey:@"coordinateLatitude"];
    [coder encodeDouble:coordinate.longitude forKey:@"coordinateLongitude"];
}

- (BOOL)isEqual:(id)other
{
    if ( ! [other isKindOfClass:[self class]])
    {
        return NO;
    }
    if (other == self)
    {
        return YES;
    }
    
    MGLPointAnnotation *otherAnnotation = other;
    return (self.coordinate.latitude == otherAnnotation.coordinate.latitude
            && self.coordinate.longitude == otherAnnotation.coordinate.longitude
            && [self.title isEqualToString:otherAnnotation.title]
            && [self.subtitle isEqualToString:otherAnnotation.subtitle]);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; title = %@; subtitle = %@; coordinate = %f, %f>",
            NSStringFromClass([self class]), (void *)self,
            self.title ? [NSString stringWithFormat:@"\"%@\"", self.title] : self.title,
            self.subtitle ? [NSString stringWithFormat:@"\"%@\"", self.subtitle] : self.subtitle,
            coordinate.latitude, coordinate.longitude];
}

@end
