//
//  mANetworkManager.h
//  miniAudicle
//
//  Created by Spencer Salazar on 4/13/14.
//
//

#import <Foundation/Foundation.h>

@class mANetworkAction;

@interface mANetworkRoom : NSObject

@property (strong, nonatomic) NSString *roomId;
@property (strong, nonatomic) NSString *name;

@end

@interface mANetworkManager : NSObject

@property (copy, nonatomic) NSString *serverHost;
@property (nonatomic) NSInteger serverPort;

- (NSString *)userId;
- (NSURL *)makeURL:(NSString *)path;
- (void)listRooms:(void (^)(NSArray *))listHandler // array of mANetworkRoom
     errorHandler:(void (^)(NSError *))errorHandler;
- (void)joinRoom:(NSString *)roomId
         handler:(void (^)(mANetworkAction *))updateHandler
    errorHandler:(void (^)(NSError *))errorHandler;
- (void)leaveCurrentRoom;

@end