#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "globals.h"

#include "maintab_items.h"
#include "maintab_chars.h"
#include "maintab_log.h"

#include "dlg_parseprogress.h"
#include "dlg_profilescripts_options.h"

#include "../spherescript/scriptparser.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Load scripts profiles
    g_scriptsProfiles = ScriptsProfile::readJsonData();

    // Generate "Profiles" menu entries
    QAction *actionLoadDefaultProfile = new QAction("Load default profile", this);
    ui->menuProfiles->addAction(actionLoadDefaultProfile);
    connect(actionLoadDefaultProfile, SIGNAL(triggered(bool)), this, SLOT(on_actionLoadDefaultProfile_triggered()));
    // TODO: vedi perchè il connect dell'azione dà errore in log
    // TODO: aggiungi un'azione per caricare ogni singolo profilo

    // Create tabs instances
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

    delete m_Profile_Options_inst;

    delete ui;
}

void MainWindow::on_actionLoadDefaultProfile_triggered()
{
    // Check which Scripts Profile is the default one
    int defaultProfileIndex = -1;
    for (size_t i = 0; i < g_scriptsProfiles.size(); i++)
    {
        if (g_scriptsProfiles[i].m_defaultProfile)  // qDebugga se è davvero true
        {
            defaultProfileIndex = (int)i;
            break;
        }
    }
    if (defaultProfileIndex == -1)
    {
        appendToLog("No default profile found!");
        return;
    }

    // The Ui must be built only in the main thread...
    Dlg_ParseProgress progressDlg(window());   // Do not set a parent? The object cannot be moved to another thread if it has a parent?
    progressDlg.move(window()->rect().center() - progressDlg.rect().center());
    progressDlg.show();

    // Set up the parser and the progress window.
    ScriptParser parser(defaultProfileIndex);

    //connect(thread, SIGNAL(started()), parser, SLOT(start()), Qt::DirectConnection);
    connect(&parser, SIGNAL(notifyPPProgressMax(int)), &progressDlg, SLOT(setProgressMax(int)));
    connect(&parser, SIGNAL(notifyPPProgressVal(int)), &progressDlg, SLOT(setProgressVal(int)));
    connect(&parser, SIGNAL(notifyPPMessage(QString)), &progressDlg, SLOT(setLabelText(QString)));
    parser.run();

    progressDlg.close();

    m_MainTab_Chars_inst->updateViews();
    m_MainTab_Items_inst->updateViews();
}

void MainWindow::on_actionOptions_triggered()
{
    // TODO: farla una istanza senza puntatore?
    delete m_Profile_Options_inst;
    m_Profile_Options_inst = new Dlg_ProfileScripts_Options(this);
    m_Profile_Options_inst->show();
}
