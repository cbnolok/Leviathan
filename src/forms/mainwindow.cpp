#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "version.h"
#include "../spherescript/scriptparser.h"
#include "maintab_items.h"
#include "maintab_chars.h"
#include "maintab_log.h"
#include "subdlg_taskprogress.h"
#include "dlg_settings.h"
#include "dlg_profileclient_options.h"
#include "dlg_profilescripts_options.h"
#include <QTimer>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setting version in the title bar
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    QString version = TOSTRING(LEVIATHAN_VERSION);
    #undef STRINGIFY
    #undef TOSTRING
    #ifndef BUILD_NOT_AUTOMATIC
        version += "(automated)";
    #endif
    this->setWindowTitle(this->windowTitle() + " - version " + version);


    // Load settings and profiles
    g_settings = Settings::readJsonData();
    g_clientProfiles = ClientProfile::readJsonData();
    g_scriptsProfiles = ScriptsProfile::readJsonData();

    /*  Setting up the menubar */

    // Generate Client Profiles menu entries
    QAction *actionEditClientProfiles = new QAction("Edit Client Profiles", this);
    ui->menuProfiles->addAction(actionEditClientProfiles);
    connect(actionEditClientProfiles, SIGNAL(triggered(bool)), this, SLOT(onManual_actionEditClientProfiles_triggered()));

    QAction *actionLoadDefaultClientProfile = new QAction("Load default Client Profile", this);
    ui->menuProfiles->addAction(actionLoadDefaultClientProfile);
    connect(actionLoadDefaultClientProfile, SIGNAL(triggered(bool)), this, SLOT(onManual_actionLoadDefaultClientProfile_triggered()));

    ui->menuProfiles->addSeparator();

    // Generate Scripts Profiles menu entries
    QAction *actionEditScriptsProfiles = new QAction("Edit Scripts Profiles", this);
    ui->menuProfiles->addAction(actionEditScriptsProfiles);
    connect(actionEditScriptsProfiles, SIGNAL(triggered(bool)), this, SLOT(onManual_actionEditScriptsProfiles_triggered()));

    QAction *actionLoadDefaultScriptsProfile = new QAction("Load default Scripts Profile", this);
    ui->menuProfiles->addAction(actionLoadDefaultScriptsProfile);
    connect(actionLoadDefaultScriptsProfile, SIGNAL(triggered(bool)), this, SLOT(onManual_actionLoadDefaultScriptsProfile_triggered()));

    // TODO: aggiungi un'azione per caricare ogni singolo profilo (in un submenu?)

    // Generate Settings entry
    ui->menuProfiles->addSeparator();
    QAction *actionSettings = new QAction("Settings", this);
    ui->menuProfiles->addAction(actionSettings);
    connect(actionSettings, SIGNAL(triggered(bool)), this, SLOT(onManual_actionSettings_triggered()));


    /* Setting up the tabs in this form */

    m_MainTab_Items_inst = new MainTab_Items();
    ui->tabWidget->insertTab(0, m_MainTab_Items_inst, "Items");
    m_MainTab_Chars_inst = new MainTab_Chars();
    ui->tabWidget->insertTab(1, m_MainTab_Chars_inst, "Chars");
    g_MainTab_Log_inst = new MainTab_Log();
    ui->tabWidget->insertTab(2, g_MainTab_Log_inst, "Log");          // this is a global class


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
    loadClientFiles();  // in common.cpp

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


