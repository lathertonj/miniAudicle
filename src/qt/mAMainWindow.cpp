/*----------------------------------------------------------------------------
miniAudicle
GUI to ChucK audio programming environment

Copyright (c) 2005-2013 Spencer Salazar.  All rights reserved.
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

#include "mAMainWindow.h"
#include "ui_mAMainWindow.h"

#include "madocumentview.h"
#include "mAConsoleMonitor.h"
#include "mAVMMonitor.h"
#include "mAPreferencesWindow.h"

#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>
#include <QCloseEvent>
#include <QPushButton>

#include <list>


extern const char MA_VERSION[];
extern const char MA_ABOUT[];
extern const char MA_HELP[];
extern const char CK_VERSION[];


const int MAX_RECENT_FILES = 10;


mAMainWindow::mAMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mAMainWindow),
    ma(new miniAudicle)
{
    ui->setupUi(this);
    
    vm_on = false;

    QCoreApplication::setOrganizationName("Stanford CCRMA");
    QCoreApplication::setOrganizationDomain("ccrma.stanford.edu");
    QCoreApplication::setApplicationName("miniAudicle");
    
    mAPreferencesWindow::configureDefaults();
    
    m_consoleMonitor = new mAConsoleMonitor(NULL);
    m_vmMonitor = new mAVMMonitor(NULL, this, ma);
    m_preferencesWindow = NULL;

    m_docid = ma->allocate_document_id();

    ui->actionAdd_Shred->setEnabled(false);
    ui->actionRemove_Shred->setEnabled(false);
    ui->actionReplace_Shred->setEnabled(false);
    ui->actionRemove_Last_Shred->setEnabled(false);
    ui->actionRemove_All_Shreds->setEnabled(false);

    updateRecentFilesMenu();
    
    QSettings settings;
    ma->set_log_level(settings.value("/ChucK/LogLevel", (int) 2).toInt());
    switch(ma->get_log_level())
    {
    case 0: ui->actionLogNone->setChecked(true); break;
    case 1: ui->actionLogCore->setChecked(true); break;
    case 2: ui->actionLogSystem->setChecked(true); break;
    case 3: ui->actionLogSevere->setChecked(true); break;
    case 4: ui->actionLogWarning->setChecked(true); break;
    case 5: ui->actionLogInfo->setChecked(true); break;
    case 6: ui->actionLogConfig->setChecked(true); break;
    case 7: ui->actionLogFine->setChecked(true); break;
    case 8: ui->actionLogFiner->setChecked(true); break;
    case 9: ui->actionLogFinest->setChecked(true); break;
    case 10: ui->actionLogCrazy->setChecked(true); break;
    }
    
    QWidget * expandingSpace = new QWidget;
    expandingSpace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->mainToolBar->insertWidget(ui->actionRemove_Last_Shred, expandingSpace);

    QDesktopWidget * desktopWidget = QApplication::desktop();
    QRect available = desktopWidget->availableGeometry(this);

    // position left edge at 0.2, bottom edge at 0.8
    m_consoleMonitor->move(available.left() + available.width()*0.2,
                           available.top() + available.height()*0.8 - m_consoleMonitor->frameGeometry().height());
    m_consoleMonitor->setAttribute(Qt::WA_QuitOnClose, false);
    m_consoleMonitor->show();

    // position right edge at 0.8, center vertically
    m_vmMonitor->move(available.left() + available.width()*0.8 - m_vmMonitor->frameGeometry().width(),
                      available.top() + available.height()*0.2);
    m_vmMonitor->setAttribute(Qt::WA_QuitOnClose, false);
    m_vmMonitor->show();
    
    m_preferencesWindow = new mAPreferencesWindow(NULL, ma);
    m_preferencesWindow->move(this->pos().x() + this->frameGeometry().width()/2 - m_preferencesWindow->frameGeometry().width()/2,
                              this->pos().y() + this->frameGeometry().height()/2 - m_preferencesWindow->frameGeometry().height()/2);    
    m_preferencesWindow->setAttribute(Qt::WA_QuitOnClose, false);

    // center horizontally, top 100px down from top
    this->move(available.left() + available.width()*0.5 - this->frameGeometry().width()/2,
               100);

    newFile();
}

mAMainWindow::~mAMainWindow()
{
    if(vm_on)
    {
        ma->stop_vm();
    }

    ma->free_document_id(m_docid);

    // manually detach
    // each window has a pointer to the miniAudicle object
    // so we have to detach that object before deleting the miniAudicle
    for(int i = ui->tabWidget->count()-1; i >= 0; i--)
    {
        mADocumentView * view = (mADocumentView *) ui->tabWidget->widget(i);
        view->detach();
    }

    delete m_vmMonitor;
    m_vmMonitor = NULL;
    delete m_consoleMonitor;
    m_consoleMonitor = NULL;
    delete m_preferencesWindow;
    m_preferencesWindow = NULL;
    delete ui;
    ui = NULL;
    delete ma;
    ma = NULL;
}


void mAMainWindow::exit()
{
    if(shouldCloseOrQuit())
        qApp->exit(0);
}

void mAMainWindow::closeEvent(QCloseEvent * event)
{
    if(shouldCloseOrQuit())
        event->accept();
    else
        event->ignore();
}

bool mAMainWindow::shouldCloseOrQuit()
{
    list<mADocumentView *> unsavedViews;
    
    for(int i = 0; i < ui->tabWidget->count(); i++)
    {
        mADocumentView * view = (mADocumentView *) ui->tabWidget->widget(i);
        if(view->isDocumentModified())
            unsavedViews.push_back(view);
    }
    
    bool review = false;
    
    if(unsavedViews.size() == 1)
    {
        review = true;
    }
    else if(unsavedViews.size() > 1)
    {
        QMessageBox messageBox(this);
        
        messageBox.window()->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        messageBox.window()->setWindowIcon(this->windowIcon());
        
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(QString("<b>You have %1 documents with unsaved changes. Do you want to review these changes before quitting?</b>").arg(unsavedViews.size()));
        messageBox.setInformativeText("If you don’t review your documents, all your changes will be lost.");
        
        messageBox.setDefaultButton(messageBox.addButton("Review Changes", QMessageBox::AcceptRole));
        messageBox.setEscapeButton(messageBox.addButton("Cancel", QMessageBox::RejectRole));
        messageBox.addButton("Discard Changes", QMessageBox::DestructiveRole);
        
        int ret = messageBox.exec();
        
        QMessageBox::ButtonRole role = messageBox.buttonRole(messageBox.clickedButton());
        if(role == QMessageBox::AcceptRole)
        {
            review = true;
        }
        else if(ret == QMessageBox::DestructiveRole)
        {
        }
        else
        {
            return false;
        }
    }
    
    if(review)
    {
        for(int i = ui->tabWidget->count()-1; i >= 0; i--)
        {
            if(!performClose(i))
            {
                return false;
            }
        }
    }
    
    return true;
}

void mAMainWindow::about()
{
    char buf[256];
    snprintf(buf, 256, MA_ABOUT, MA_VERSION, CK_VERSION, sizeof(void*)*8);
    QString body = QString("<h3>miniAudicle</h3>\n") + buf;
    body.replace(QRegExp("\n"), "<br />");
    QMessageBox::about(this, "About miniAudicle", body);
}

void mAMainWindow::newFile()
{
    mADocumentView * documentView = new mADocumentView(0, "untitled", NULL, ma);
    documentView->setTabWidget(ui->tabWidget);
    QObject::connect(m_preferencesWindow, SIGNAL(preferencesChanged()),
                     documentView, SLOT(preferencesChanged()));

    ui->tabWidget->addTab(documentView, QIcon(), "untitled");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

    documentView->show();
}

void mAMainWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull())
        fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), "", "ChucK Scripts (*.ck)");

    if (!fileName.isEmpty())
    {
        QFile * file = new QFile(fileName);
        if (file->open(QFile::ReadWrite))
        {
            // close -- will be reopened as needed by document view
            file->close();            
            QFileInfo fileInfo(fileName);
            mADocumentView * documentView = new mADocumentView(0, fileInfo.fileName().toStdString(), file, ma);
            documentView->setTabWidget(ui->tabWidget);
            QObject::connect(m_preferencesWindow, SIGNAL(preferencesChanged()),
                             documentView, SLOT(preferencesChanged()));

            ui->tabWidget->addTab(documentView, QIcon(), fileInfo.fileName());
            ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

            documentView->show();
            
            QString path = documentView->filePath();
            addRecentFile(path);
            updateRecentFilesMenu();            
        }
    }
}

void mAMainWindow::openExample()
{
    QString examplesDir;
#ifdef __PLATFORM_WIN32__
    examplesDir = QCoreApplication::applicationDirPath() + "/examples";
#endif
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Example"), examplesDir, "ChucK Scripts (*.ck)");

    if(!fileName.isEmpty())
    {
        QFile * file = new QFile(fileName);
        if (file->open(QFile::ReadWrite))
        {
            // close -- will be reopened as needed by document view
            file->close();
            QFileInfo fileInfo(fileName);
            mADocumentView * documentView = new mADocumentView(0, fileInfo.fileName().toStdString(), file, ma);
            documentView->setTabWidget(ui->tabWidget);
            QObject::connect(m_preferencesWindow, SIGNAL(preferencesChanged()),
                             documentView, SLOT(preferencesChanged()));
            documentView->setReadOnly(true);

            ui->tabWidget->addTab(documentView, QIcon(), fileInfo.fileName());
            ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);

            documentView->show();
            
            QString path = documentView->filePath();
            addRecentFile(path);
            updateRecentFilesMenu();            
        }
    }
}

void mAMainWindow::openRecent()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action)
        openFile(action->data().toString());
}

void mAMainWindow::closeFile()
{
    closeFile(ui->tabWidget->currentIndex());
}

void mAMainWindow::closeFile(int i)
{
    performClose(i);
}

bool mAMainWindow::performClose(int i)
{
    mADocumentView * view = (mADocumentView *) ui->tabWidget->widget(i);

    if(view == NULL)
        return true;

    if(view->isDocumentModified())
    {
        QMessageBox msgBox;
        msgBox.setText(QString("<b>The document '%1' has been modified.</b>").arg(view->title().c_str()));
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();

        if(ret == QMessageBox::Save)
        {
            view->save();
        }
        else if(ret == QMessageBox::Cancel)
        {
            return false;
        }
    }

    ui->tabWidget->removeTab(i);
    delete view;
    
    return true;
}

void mAMainWindow::saveFile()
{
    mADocumentView * view = (mADocumentView *) ui->tabWidget->currentWidget();

    if(view == NULL)
        return;

    view->save();

    QString path = view->filePath();
    if(path.length())
    {
        addRecentFile(path);
        updateRecentFilesMenu();
    }
}

void mAMainWindow::saveAs()
{
    mADocumentView * view = (mADocumentView *) ui->tabWidget->currentWidget();

    if(view == NULL)
        return;
    
    // TODO: have saveAs() and save() return true if save was successful
    // then update recent files according to that
    view->saveAs();

    QString path = view->filePath();
    if(path.length())
    {
        addRecentFile(path);
        updateRecentFilesMenu();
    }
}

void mAMainWindow::tabSelected(int index)
{
    mADocumentView * currentView = (mADocumentView *) ui->tabWidget->currentWidget();
    
    if(currentView == NULL)
    {
        statusBar()->clearMessage();
        return;
    }
    
    string result = currentView->lastResult();
    if(result.size())
        statusBar()->showMessage(QString(result.c_str()));
    else
        statusBar()->clearMessage();
}

#pragma mark

void mAMainWindow::addCurrentDocument()
{
    mADocumentView *currentDocument = ((mADocumentView *) ui->tabWidget->currentWidget());
    
    currentDocument->add();
    
    string result = currentDocument->lastResult();
    if(result.size())
        statusBar()->showMessage(QString(result.c_str()));
    else
        statusBar()->clearMessage();
}

void mAMainWindow::replaceCurrentDocument()
{
    mADocumentView *currentDocument = ((mADocumentView *) ui->tabWidget->currentWidget());
    
    currentDocument->replace();
    
    string result = currentDocument->lastResult();
    if(result.size())
        statusBar()->showMessage(QString(result.c_str()));
    else
        statusBar()->clearMessage();
}

void mAMainWindow::removeCurrentDocument()
{
    mADocumentView *currentDocument = ((mADocumentView *) ui->tabWidget->currentWidget());
    
    currentDocument->remove();
    
    string result = currentDocument->lastResult();
    if(result.size())
        statusBar()->showMessage(QString(result.c_str()));
    else
        statusBar()->clearMessage();
}

void mAMainWindow::removeLastShred()
{
    string result;
    ma->removelast(m_docid, result);
}

void mAMainWindow::removeAllShreds()
{
    string result;
    ma->removeall(m_docid, result);
}

void mAMainWindow::toggleVM()
{
    QSettings settings;
    
    if(!vm_on)
    {
        ma->set_enable_audio(settings.value(mAPreferencesEnableAudio).toBool());
        ma->set_enable_network_thread(settings.value(mAPreferencesEnableNetwork).toBool());
        ma->set_adc(settings.value(mAPreferencesAudioInput).toInt());
        ma->set_dac(settings.value(mAPreferencesAudioOutput).toInt());
        ma->set_num_inputs(settings.value(mAPreferencesInputChannels).toInt());
        ma->set_num_outputs(settings.value(mAPreferencesOutputChannels).toInt());
        ma->set_sample_rate(settings.value(mAPreferencesSampleRate).toInt());
        ma->set_buffer_size(settings.value(mAPreferencesBufferSize).toInt());
        
        list<string> chuginDirs;
        list<string> chuginFiles;
        
        if(settings.value(mAPreferencesEnableChuGins).toBool())
        {
            QStringList chuginPaths = settings.value(mAPreferencesChuGinPaths).toStringList();
            for(int i = 0; i < chuginPaths.length(); i++)
            {
                QString path = chuginPaths.at(i);
                QFileInfo fileInfo(path);
                if(fileInfo.isDir())
                    chuginDirs.push_back(path.toStdString());
                else
                    chuginFiles.push_back(path.toStdString());
            }
        }
        
        ma->set_library_paths(chuginDirs);
        ma->set_named_chugins(chuginFiles);
        
        if(ma->start_vm())
        {
            ui->actionStart_Virtual_Machine->setText("Stop Virtual Machine");

            m_vmMonitor->vmChangedToState(true);

            vm_on = true;
        }
    }
    else
    {
        ma->stop_vm();

        ui->actionStart_Virtual_Machine->setText("Start Virtual Machine");

        m_vmMonitor->vmChangedToState(false);

        vm_on = false;
    }

    ui->actionAdd_Shred->setEnabled(vm_on);
    ui->actionRemove_Shred->setEnabled(vm_on);
    ui->actionReplace_Shred->setEnabled(vm_on);
    ui->actionRemove_Last_Shred->setEnabled(vm_on);
    ui->actionRemove_All_Shreds->setEnabled(vm_on);
}


void mAMainWindow::setLogLevel()
{
    ui->actionLogNone->setChecked(false);
    ui->actionLogCore->setChecked(false);
    ui->actionLogSystem->setChecked(false);
    ui->actionLogSevere->setChecked(false);
    ui->actionLogWarning->setChecked(false);
    ui->actionLogInfo->setChecked(false);
    ui->actionLogConfig->setChecked(false);
    ui->actionLogFine->setChecked(false);
    ui->actionLogFiner->setChecked(false);
    ui->actionLogFinest->setChecked(false);
    ui->actionLogCrazy->setChecked(false);
    
    QObject *sender = this->sender();
    if(sender == ui->actionLogNone)         ma->set_log_level(0);
    else if(sender == ui->actionLogCore)    ma->set_log_level(1);
    else if(sender == ui->actionLogSystem)  ma->set_log_level(2);
    else if(sender == ui->actionLogSevere)  ma->set_log_level(3);
    else if(sender == ui->actionLogWarning) ma->set_log_level(4);
    else if(sender == ui->actionLogInfo)    ma->set_log_level(5);
    else if(sender == ui->actionLogConfig)  ma->set_log_level(6);
    else if(sender == ui->actionLogFine)    ma->set_log_level(7);
    else if(sender == ui->actionLogFiner)   ma->set_log_level(8);
    else if(sender == ui->actionLogFinest)  ma->set_log_level(9);
    else if(sender == ui->actionLogCrazy)   ma->set_log_level(10);
    
    QAction * action = qobject_cast<QAction *>(sender);
    action->setChecked(true);
    
    QSettings settings;
    settings.setValue("/ChucK/LogLevel", ma->get_log_level());
}


void mAMainWindow::showPreferences()
{
    if(m_preferencesWindow == NULL)
    {
        m_preferencesWindow = new mAPreferencesWindow(NULL, ma);
        m_preferencesWindow->move(this->pos().x() + this->frameGeometry().width()/2 - m_preferencesWindow->frameGeometry().width()/2,
                                  this->pos().y() + this->frameGeometry().height()/2 - m_preferencesWindow->frameGeometry().height()/2);
    }
    
    m_preferencesWindow->show();
    m_preferencesWindow->raise();
    m_preferencesWindow->activateWindow();
}


void mAMainWindow::showConsoleMonitor()
{
    m_consoleMonitor->show();
    m_consoleMonitor->raise();
    m_consoleMonitor->activateWindow();
}

void mAMainWindow::showVirtualMachineMonitor()
{
    m_vmMonitor->show();
    m_vmMonitor->raise();
    m_vmMonitor->activateWindow();
}

void mAMainWindow::addRecentFile(QString &path)
{
    QSettings settings;
    QStringList recentFiles = settings.value("/GUI/RecentFiles").toStringList();
    
    recentFiles.removeAll(path);
    while(recentFiles.length() > MAX_RECENT_FILES-1)
        recentFiles.removeLast();
    recentFiles.prepend(path);
    
    settings.setValue("/GUI/RecentFiles", recentFiles);
}

void mAMainWindow::updateRecentFilesMenu()
{
    ui->menuRecent_Files->clear();
    
    QSettings settings;
    
    QStringList recentFiles = settings.value("/GUI/RecentFiles").toStringList();
    
    for(int i = 0; i < recentFiles.length(); i++)
    {
        QString path = recentFiles.at(i);
        QAction * action = new QAction(this);
        action->setText(QString("&%1   %2")
                        .arg(i+1)
                        .arg(QFileInfo(path).fileName()));
        action->setData(path);
        connect(action, SIGNAL(triggered()), SLOT(openRecent()));
        
        ui->menuRecent_Files->addAction(action);
    }
}
