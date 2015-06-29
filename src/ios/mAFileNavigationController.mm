//
//  mAFileNavigationController.m
//  miniAudicle
//
//  Created by Spencer Salazar on 1/7/15.
//
//

#import "mAFileNavigationController.h"
#import "mADocumentManager.h"
#import "mAFileViewController.h"
#import "mADocumentManager.h"


@interface mAFileNavigationController ()
{
    IBOutlet UISegmentedControl *segmentedControl;
}

@property (strong, nonatomic) IBOutlet UINavigationController *childNavigationController;

@property (strong, nonatomic) mAFileViewController *myScriptsViewController;
@property (strong, nonatomic) mAFileViewController *recentViewController;
@property (strong, nonatomic) mAFileViewController *examplesViewController;

@property (strong, nonatomic) IBOutlet UIView *contentView;

- (void)setup;
- (void)adjustNavigationBar:(UIViewController *)targetViewController animated:(BOOL)animated;
- (IBAction)selectedMode:(id)sender;

@end

@implementation mAFileNavigationController

- (void)setDetailViewController:(mADetailViewController *)detailViewController
{
    _detailViewController = detailViewController;
    
    self.myScriptsViewController.detailViewController = self.detailViewController;
    self.recentViewController.detailViewController = self.detailViewController;
    self.examplesViewController.detailViewController = self.detailViewController;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.myScriptsViewController = [[mAFileViewController alloc] initWithNibName:@"mAFileViewController" bundle:nil];
    self.myScriptsViewController.scripts = [[mADocumentManager manager] userScripts];
    self.myScriptsViewController.editable = YES;
    
    self.examplesViewController = [[mAFileViewController alloc] initWithNibName:@"mAFileViewController" bundle:nil];
    self.examplesViewController.scripts = [[mADocumentManager manager] exampleScripts];
    self.examplesViewController.editable = NO;
    
    self.recentViewController = [[mAFileViewController alloc] initWithNibName:@"mAFileViewController" bundle:nil];
    self.recentViewController.scripts = [[mADocumentManager manager] recentFiles];
    self.recentViewController.editable = NO;
    
    self.myScriptsViewController.detailViewController = self.detailViewController;
    self.recentViewController.detailViewController = self.detailViewController;
    self.examplesViewController.detailViewController = self.detailViewController;
    
    [self.navigationController pushViewController:self.myScriptsViewController animated:NO];
    self.navigationController.navigationBar.translucent = NO;

    // reset to add subview to contentView
    _childNavigationController.view.frame = self.contentView.bounds;
    [self.contentView addSubview:_childNavigationController.view];
    [self.childNavigationController setViewControllers:@[self.myScriptsViewController] animated:NO];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(recentFilesChanged:)
                                                 name:mADocumentManagerRecentFilesChanged
                                               object:[mADocumentManager manager]];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)selectedMode:(id)sender
{
    switch([segmentedControl selectedSegmentIndex])
    {
        case 0:
            [self.childNavigationController setViewControllers:@[self.myScriptsViewController] animated:NO];
            break;
        case 1:
            [self.childNavigationController setViewControllers:@[self.recentViewController] animated:NO];
            break;
        case 2:
            [self.childNavigationController setViewControllers:@[self.examplesViewController] animated:NO];
            break;
        default: ; // uhh
    }
}

- (void)adjustNavigationBar:(UIViewController *)targetViewController animated:(BOOL)animated
{
    if(targetViewController == _childNavigationController.viewControllers[0] &&
//       !(targetViewController.navigationItem.leftBarButtonItems.count || targetViewController.navigationItem.leftBarButtonItem ||
//         targetViewController.navigationItem.rightBarButtonItems.count || targetViewController.navigationItem.rightBarButtonItem))
       !(targetViewController.navigationItem.leftBarButtonItems.count || targetViewController.navigationItem.rightBarButtonItems.count ))
        [_childNavigationController setNavigationBarHidden:YES animated:animated];
    else
        [_childNavigationController setNavigationBarHidden:NO animated:animated];
}

- (void)recentFilesChanged:(NSNotification *)n
{
    [self.recentViewController scriptsChanged];
}


#pragma mark - UINavigationControllerDelegate

- (void)navigationController:(UINavigationController *)navigationController
      willShowViewController:(UIViewController *)viewController
                    animated:(BOOL)animated
{
    [self adjustNavigationBar:viewController animated:animated];
}


@end