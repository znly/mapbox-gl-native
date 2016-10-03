#import "MGLShape.h"

@implementation MGLShape

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if (self = [super init]) {
        _title = [coder decodeObjectOfClass:[NSString class] forKey:@"title"];
        _subtitle = [coder decodeObjectOfClass:[NSString class] forKey:@"subtitle"];
#if !TARGET_OS_IPHONE
        _toolTip = [coder decodeObjectOfClass:[NSString class] forKey:@"toolTip"];
#endif
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_title forKey:@"title"];
    [coder encodeObject:_subtitle forKey:@"subtitle"];
#if !TARGET_OS_IPHONE
    [coder encodeObject:_toolTip forKey:@"toolTip"];
#endif
}

- (CLLocationCoordinate2D)coordinate
{
    [[NSException exceptionWithName:@"MGLAbstractClassException"
                             reason:@"MGLShape is an abstract class"
                           userInfo:nil] raise];

    return CLLocationCoordinate2DMake(MAXFLOAT, MAXFLOAT);
}

@end
