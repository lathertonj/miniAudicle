/*----------------------------------------------------------------------------
miniAudicle
Cocoa GUI to chuck audio programming environment

Copyright (c) 2005 Spencer Salazar.  All rights reserved.
http://chuck.cs.princeton.edu/
http://soundlab.cs.princeton.edu/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: miniAudicleDocument.h
// desc: Document class, creates a new window for each new document and manages
//       document-level connections to miniAudicle
//
// author: Spencer Salazar (ssalazar@princeton.edu)
// date: Autumn 2005
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import "chuck_def.h"

class miniAudicle;
@class NumberedTextView;
@class RBSplitSubview;
@class mAArgumentsTableView;

@interface miniAudicleDocument : NSDocument <NSToolbarDelegate>
{
    IBOutlet NSWindow * window;
    IBOutlet NumberedTextView * text_view;
    IBOutlet NSTextField * status_text;
    IBOutlet RBSplitSubview * argument_subview;
    IBOutlet NSTextField * argument_text;
    IBOutlet NSButton * toggle_argument_subview;
    IBOutlet NSButton * button_bar;
    IBOutlet NSTableView * argument_table;
    
    NSMutableArray * arguments;
    BOOL reject_argument_edits;

    NSToolbar * toolbar;
    NSString * data;
        
    //NSDrawer * shred_drawer;
    //NSDrawer * document_drawer;
    NSDrawer * argument_drawer;
    
    NSMenu * remove_menu;
    NSMenu * replace_menu;
        
    miniAudicle * ma;
    t_CKUINT docid;
    BOOL vm_on;
    
    BOOL shows_arguments;
    BOOL shows_toolbar;
    BOOL shows_line_numbers;
    BOOL shows_status_bar;
    
    BOOL has_customized_appearance;
}

- (id)init;

- (void)userDefaultsDidChange:(NSNotification *)n;

- (NSData *)dataRepresentationOfType:(NSString *)type;
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)type;
- (BOOL)isEmpty;

- (void)add:(id)sender;
- (void)remove:(id)sender;
- (void)replace:(id)sender;
- (void)removeall:(id)sender;
- (void)removelast:(id)sender;

- (void)removeShred:(id)sender;
- (void)replaceShred:(id)sender;

- (void)setLockEditing:(BOOL)lock;
- (BOOL)lockEditing;

- (void)commentOut:(id)sender;

- (void)saveBackup:(id)sender;

- (void)setMiniAudicle:(miniAudicle *)t_ma;
- (void)setVMOn:(BOOL)t_vm_on;
- (void)vm_on;
- (void)vm_off;

- (void)setShowsArguments:(BOOL)_shows_arguments;
- (BOOL)showsArguments;

- (void)setShowsToolbar:(BOOL)_shows_toolbar;
- (BOOL)showsToolbar;

- (void)setShowsLineNumbers:(BOOL)_shows_line_numbers;
- (BOOL)showsLineNumbers;

- (void)setShowsStatusBar:(BOOL)_shows_status_bar;
- (BOOL)showsStatusBar;

- (void)handleArgumentText:(id)sender;

- (void)argumentsTableView:(mAArgumentsTableView *)atv 
                deleteRows:(NSIndexSet *)is;
@end
