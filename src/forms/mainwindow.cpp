#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QSignalMapper>

#include "globals.h"
#include "version.h"

#include "maintab_items.h"
#include "maintab_chars.h"
#include "maintab_tools.h"
#include "maintab_log.h"
#include "subdlg_taskprogress.h"
#include "dlg_settings.h"
#include "dlg_profileclient_options.h"
#include "dlg_profilescripts_options.h"

#include "../spherescript/scriptparser.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setting version in the title bar
    #define _STRINGIFY(x) #x
    #define TOSTRING(x) _STRINGIFY(x)
    QString version = TOSTRING(LEVIATHAN_VERSION);
    #undef _STRINGIFY
    #undef TOSTRING
    #ifndef BUILD_NOT_AUTOMATIC
        version += "(automated)";
    #endif
    this->setWindowTitle(this->windowTitle() + " - version " + version);


    // Load settings and profiles
    g_settings.updateFromJson();
    g_clientProfiles = ClientProfile::createFromJson();
    g_scriptsProfiles = ScriptsProfile::createFromJson();

    /*  Setting up the menubar */

    // Generate Client Profiles menu entries
    QAction *actionEditClientProfiles = new QAction("Edit Client Profiles", nullptr);
    ui->menuProfiles->addAction(actionEditClientProfiles);
    connect(actionEditClientProfiles, SIGNAL(triggered(bool)), this, SLOT(onManual_actionEditClientProfiles_triggered()));

    QAction *actionLoadDefaultClientProfile = new QAction("Load default Client Profile", nullptr);
    ui->menuProfiles->addAction(actionLoadDefaultClientProfile);
    connect(actionLoadDefaultClientProfile, SIGNAL(triggered(bool)), this, SLOT(onManual_actionLoadDefaultClientProfile_triggered()));

    if (!g_clientProfiles.empty())
    {
        QMenu *menuLoadClientProfile = new QMenu("Load Client Profile...", nullptr);
        QSignalMapper* clientProfilesSignalMapper = new QSignalMapper();
        std::vector<QAction*> clientProfileActions;
        for (size_t i = 0; i < g_clientProfiles.size(); ++i)
        {
            clientProfileActions.emplace_back(new QAction(g_clientProfiles[i].m_name.c_str(), nullptr));
            menuLoadClientProfile->addAction(clientProfileActions[i]);
            connect(clientProfileActions[i], SIGNAL(triggered()), clientProfilesSignalMapper, SLOT(map()));
            clientProfilesSignalMapper->setMapping(clientProfileActions[i], (int)i);
        }
        connect(clientProfilesSignalMapper, SIGNAL(mapped(int)), this, SLOT(onManual_actionLoadClientProfile_mapped(int)));
        ui->menuProfiles->addMenu(menuLoadClientProfile);
    }

    ui->menuProfiles->addSeparator();

    // Generate Scripts Profiles menu entries
    QAction *actionEditScriptsProfiles = new QAction("Edit Scripts Profiles", nullptr);
    ui->menuProfiles->addAction(actionEditScriptsProfiles);
    connect(actionEditScriptsProfiles, SIGNAL(triggered(bool)), this, SLOT(onManual_actionEditScriptsProfiles_triggered()));

    QAction *actionLoadDefaultScriptsProfile = new QAction("Load default Scripts Profile", nullptr);
    ui->menuProfiles->addAction(actionLoadDefaultScriptsProfile);
    connect(actionLoadDefaultScriptsProfile, SIGNAL(triggered(bool)), this, SLOT(onManual_actionLoadDefaultScriptsProfile_triggered()));

    if (!g_scriptsProfiles.empty())
    {
        QMenu *menuLoadScriptsProfile = new QMenu("Load Scripts Profile...", nullptr);
        QSignalMapper* scriptsProfilesSignalMapper = new QSignalMapper();
        std::vector<QAction*> scriptsProfileActions;
        for (size_t i = 0; i < g_scriptsProfiles.size(); ++i)
        {
            scriptsProfileActions.emplace_back(new QAction(g_scriptsProfiles[i].m_name.c_str(), nullptr));
            menuLoadScriptsProfile->addAction(scriptsProfileActions[i]);
            connect(scriptsProfileActions[i], SIGNAL(triggered()), scriptsProfilesSignalMapper, SLOT(map()));
            scriptsProfilesSignalMapper->setMapping(scriptsProfileActions[i], (int)i);
        }
        connect(scriptsProfilesSignalMapper, SIGNAL(mapped(int)), this, SLOT(onManual_actionLoadScriptsProfile_mapped(int)));
        ui->menuProfiles->addMenu(menuLoadScriptsProfile);
    }

    // TODO: add in a submenu the actions to load every single, stored profile

    // Generate Settings entry
    //ui->menuProfiles->addSeparator();
    QAction *actionSettings = new QAction("Settings", nullptr);
    //ui->menuProfiles->addAction(actionSettings);
    ui->menuBar->addAction(actionSettings);
    connect(actionSettings, SIGNAL(triggered(bool)), this, SLOT(onManual_actionSettings_triggered()));


    /* Setting up the tabs in this form */

    m_MainTab_Items_inst = new MainTab_Items();
    ui->tabWidget->insertTab(0, m_MainTab_Items_inst, "Items");
    m_MainTab_Chars_inst = new MainTab_Chars();
    ui->tabWidget->insertTab(1, m_MainTab_Chars_inst, "Chars");
    m_MainTab_Tools_inst = new MainTab_Tools();
    ui->tabWidget->insertTab(2, m_MainTab_Tools_inst, "Tools");
    g_MainTab_Log_inst = new MainTab_Log();
    ui->tabWidget->insertTab(3, g_MainTab_Log_inst, "Log");          // this is a global class


    /* Startup-time operations */

    if (g_settings.m_loadDefaultProfilesAtStartup)
        QTimer::singleShot(50, this, SLOT(loadDefaultProfiles()));
}

MainWindow::~MainWindow()
{
    delete m_MainTab_Items_inst;
    delete m_MainTab_Chars_inst;
    delete g_MainTab_Log_inst;
    delete ui;
}


void MainWindow::on_tabWidget_currentChanged(int /* UNUSED: index */)
{
    ui->tabWidget->currentWidget()->setFocus();     // i need the keyboard focus to be able to use CTRL + F to start a search
}

/* Menu bar actions */

void MainWindow::onManual_actionEditClientProfiles_triggered()
{
    Dlg_ProfileClient_Options dlg(this);
    dlg.exec();
}

void MainWindow::onManual_actionLoadDefaultClientProfile_triggered()
{
    int clientProfileIdx = getDefaultClientProfile();
    if (clientProfileIdx != -1)
        loadClientProfile(clientProfileIdx);
}


void MainWindow::onManual_actionEditScriptsProfiles_triggered()
{
    Dlg_ProfileScripts_Options dlg(this);
    dlg.exec();
}

void MainWindow::onManual_actionLoadDefaultScriptsProfile_triggered()
{
    int scriptsProfileIdx = getDefaultScriptsProfile();
    if (scriptsProfileIdx != -1)
        loadScriptProfile(scriptsProfileIdx);
}

void MainWindow::onManual_actionSettings_triggered()
{
    Dlg_Settings dlg(this);
    dlg.exec();
}

void MainWindow::onManual_actionLoadClientProfile_mapped(int index)
{
    loadClientProfile(index);
}

void MainWindow::onManual_actionLoadScriptsProfile_mapped(int index)
{
    loadScriptProfile(index);
}

void MainWindow::on_checkBox_onTop_toggled(bool checked)
{
    if (checked)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);

    // From Qt Wiki:
    // Note: This function calls setParent() when changing the flags for a window, causing the widget
    // to be hidden. You must call show() to make the widget visible again..
    show();
}

void MainWindow::on_checkBox_focus_toggled(bool checked)
{
    g_sendKeystrokeAndFocusClient = checked;
}

int MainWindow::getDefaultClientProfile()
{
    // Check which Client Profile is the default one and return its index

    for (size_t i = 0; i < g_clientProfiles.size(); ++i)
    {
        if (g_clientProfiles[i].m_defaultProfile)
            return (int)i;
    }
    appendToLog("No default Client Profile found!");
    return -1;
}

int MainWindow::getDefaultScriptsProfile()
{
    // Check which Scripts Profile is the default one and return its index

    for (size_t i = 0; i < g_scriptsProfiles.size(); ++i)
    {
        if (g_scriptsProfiles[i].m_defaultProfile)
            return (int)i;
    }
    appendToLog("No default Scripts Profile found!");
    return -1;
}

void MainWindow::loadDefaultProfiles()
{
    int clientProfileIdx = getDefaultClientProfile();
    if (clientProfileIdx != -1)
        loadClientProfile(clientProfileIdx);

    int scriptsProfileIdx = getDefaultScriptsProfile();
    if (scriptsProfileIdx != -1)
        loadScriptProfile(scriptsProfileIdx);
}

void MainWindow::loadClientProfile(int index)
{
    g_loadedClientProfile = index;

    // The Ui must be built only in the main thread...
    SubDlg_TaskProgress progressDlg(window());   // Do not set a parent? The object cannot be moved to another thread if it has a parent?
    progressDlg.move(window()->rect().center() - progressDlg.rect().center());
    progressDlg.show();

    // Setting the progress bar to "pulse"
    progressDlg.setProgressMax(0);
    progressDlg.setProgressVal(0);

    // Loading stuff
    progressDlg.setLabelText("Loading client files...");
    auto setProgress = [](int /*i*/) {QApplication::processEvents();}; //{ progressDlg.setProgressVal(i); };
    loadClientFiles(setProgress);  // in common.cpp

    progressDlg.close();
}

void MainWindow::loadScriptProfile(int index)
{
    // The Ui must be built only in the main thread...
    SubDlg_TaskProgress progressDlg(window());   // Do not set a parent? The object cannot be moved to another thread if it has a parent?
    progressDlg.move(window()->rect().center() - progressDlg.rect().center());
    progressDlg.show();

    // Set up the parser and the progress window.
    ScriptParser parser(index);

    //connect(thread, SIGNAL(started()), parser, SLOT(start()), Qt::DirectConnection);
    connect(&parser, SIGNAL(notifyTPProgressMax(int)), &progressDlg, SLOT(setProgressMax(int)));
    connect(&parser, SIGNAL(notifyTPProgressVal(int)), &progressDlg, SLOT(setProgressVal(int)));
    connect(&parser, SIGNAL(notifyTPMessage(QString)), &progressDlg, SLOT(setLabelText(QString)));
    parser.run();

    progressDlg.close();

    m_MainTab_Chars_inst->updateViews();
    m_MainTab_Items_inst->updateViews();
}

