#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "../spherescript/scriptparser.h"
#include "maintab_items.h"
#include "maintab_chars.h"
#include "maintab_log.h"
#include "subdlg_taskprogress.h"
#include "dlg_profileclient_options.h"
#include "dlg_profilescripts_options.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Load profiles
    g_clientProfiles = ClientProfile::readJsonData();
    g_scriptsProfiles = ScriptsProfile::readJsonData();


    /*  Setting up the menubar */

    // Generate Client Profiles menu entries
    QAction *actionEditClientProfiles = new QAction("Edit Client Profiles", this);
    ui->menuProfiles->addAction(actionEditClientProfiles);
    connect(actionEditClientProfiles, SIGNAL(triggered(bool)), this, SLOT(onCustom_actionEditClientProfiles_triggered()));

    QAction *actionLoadDefaultClientProfile = new QAction("Load default Client Profile", this);
    ui->menuProfiles->addAction(actionLoadDefaultClientProfile);
    connect(actionLoadDefaultClientProfile, SIGNAL(triggered(bool)), this, SLOT(onCustom_actionLoadDefaultClientProfile_triggered()));

    ui->menuProfiles->addSeparator();

    // Generate Scripts Profiles menu entries
    QAction *actionEditScriptsProfiles = new QAction("Edit Scripts Profiles", this);
    ui->menuProfiles->addAction(actionEditScriptsProfiles);
    connect(actionEditScriptsProfiles, SIGNAL(triggered(bool)), this, SLOT(onCustom_actionEditScriptsProfiles_triggered()));

    QAction *actionLoadDefaultScriptsProfile = new QAction("Load default Scripts Profile", this);
    ui->menuProfiles->addAction(actionLoadDefaultScriptsProfile);
    connect(actionLoadDefaultScriptsProfile, SIGNAL(triggered(bool)), this, SLOT(onCustom_actionLoadDefaultScriptsProfile_triggered()));

    // TODO: aggiungi un'azione per caricare ogni singolo profilo (in un submenu?)


    /* Setting up the tabs in this form */

    m_MainTab_Items_inst = new MainTab_Items();
    ui->tabWidget->insertTab(0, m_MainTab_Items_inst, "Items");
    m_MainTab_Chars_inst = new MainTab_Chars();
    ui->tabWidget->insertTab(1, m_MainTab_Chars_inst, "Chars");
    g_MainTab_Log_inst = new MainTab_Log();
    ui->tabWidget->insertTab(2, g_MainTab_Log_inst, "Log");          // this is a global class
}

MainWindow::~MainWindow()
{
    delete m_MainTab_Items_inst;
    delete m_MainTab_Chars_inst;
    delete g_MainTab_Log_inst;
    delete ui;
}


/* Menu bar actions */

void MainWindow::onCustom_actionEditClientProfiles_triggered()
{
    Dlg_ProfileClient_Options dlg(this);
    dlg.exec();
}

void MainWindow::onCustom_actionLoadDefaultClientProfile_triggered()
{
    // Check which Client Profile is the default one
    int defaultProfileIndex = -1;
    for (size_t i = 0; i < g_clientProfiles.size(); i++)
    {
        if (g_clientProfiles[i].m_defaultProfile)
        {
            defaultProfileIndex = (int)i;
            break;
        }
    }
    if (defaultProfileIndex == -1)
    {
        appendToLog("No default Client Profile found!");
        return;
    }
    g_loadedClientProfile = defaultProfileIndex;

    // The Ui must be built only in the main thread...
    SubDlg_TaskProgress progressDlg(window());   // Do not set a parent? The object cannot be moved to another thread if it has a parent?
    progressDlg.move(window()->rect().center() - progressDlg.rect().center());
    progressDlg.show();

    // Setting the progress bar to "pulse"
    progressDlg.setProgressMax(0);
    progressDlg.setProgressVal(0);

    // Loading stuff
    progressDlg.setLabelText("Loading client files...");
    loadClientFiles();

    progressDlg.close();
}


void MainWindow::onCustom_actionEditScriptsProfiles_triggered()
{
    Dlg_ProfileScripts_Options dlg(this);
    dlg.exec();
}

void MainWindow::onCustom_actionLoadDefaultScriptsProfile_triggered()
{
    // Check which Scripts Profile is the default one
    int defaultProfileIndex = -1;
    for (size_t i = 0; i < g_scriptsProfiles.size(); i++)
    {
        if (g_scriptsProfiles[i].m_defaultProfile)
        {
            defaultProfileIndex = (int)i;
            break;
        }
    }
    if (defaultProfileIndex == -1)
    {
        appendToLog("No default Scripts Profile found!");
        return;
    }

    // The Ui must be built only in the main thread...
    SubDlg_TaskProgress progressDlg(window());   // Do not set a parent? The object cannot be moved to another thread if it has a parent?
    progressDlg.move(window()->rect().center() - progressDlg.rect().center());
    progressDlg.show();

    // Set up the parser and the progress window.
    ScriptParser parser(defaultProfileIndex);

    //connect(thread, SIGNAL(started()), parser, SLOT(start()), Qt::DirectConnection);
    connect(&parser, SIGNAL(notifyTPProgressMax(int)), &progressDlg, SLOT(setProgressMax(int)));
    connect(&parser, SIGNAL(notifyTPProgressVal(int)), &progressDlg, SLOT(setProgressVal(int)));
    connect(&parser, SIGNAL(notifyTPMessage(QString)), &progressDlg, SLOT(setLabelText(QString)));
    parser.run();

    progressDlg.close();

    m_MainTab_Chars_inst->updateViews();
    m_MainTab_Items_inst->updateViews();
}

