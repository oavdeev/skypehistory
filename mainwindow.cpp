#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatlogview.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::init()
{
    itemModel = new QStandardItemModel(this);
    database.loadItems();

    this->ui->listView->setModel(itemModel);
    itemModel->setColumnCount(1);
    Q_FOREACH(QString name, database.contacts())
    {
        itemModel->appendRow(new QStandardItem(name));
    }

    chatListModel = new ChatListModel(&database, this);
    chatListModel->filter();
    this->ui->treeView->setModel(chatListModel);

    ui->treeView->setColumnWidth(0, 190);

    connect(ui->listView, SIGNAL(clicked(const QModelIndex&)), SLOT(contactListClicked(const QModelIndex&)));
    connect(ui->searchTextEdit, SIGNAL(textChanged(const QString&)), SLOT(searchTextChanged(const QString&)));

    connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(searchResultDoubleClicked(const QModelIndex&)));

    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(tabCloseRequested(int)));
    QList<int> spSizes;
    spSizes.append(0);
    spSizes.append(ui->splitterV->height());
    ui->splitterH->setSizes(spSizes);
    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(0);
}

void MainWindow::tabCloseRequested (int index )
{
    tabs.remove(ui->tabWidget->tabText(index));
    ui->tabWidget->removeTab(index);

    if (ui->tabWidget->count() == 0)
    {
        QList<int> spSizes;
        spSizes.append(0);
        spSizes.append(ui->splitterV->height());
        ui->splitterH->setSizes(spSizes);
    }
}

void MainWindow::searchTextChanged( const QString& text )
{
    chatListModel->setTextFilter(text);
    chatListModel->filter();
}

void MainWindow::searchResultDoubleClicked ( const QModelIndex & index )
{
    if (ui->splitterH->sizes()[0] == 0)
    {
        QList<int> spSizes;
        spSizes.append(ui->splitterV->height() * 2 /3);
        spSizes.append(ui->splitterV->height() / 3);
        ui->splitterH->setSizes(spSizes);
    }


    QSharedPointer<SkypeDbb::Chat> chat = chatListModel->getChat(index);
    QSharedPointer<Snippet> snippet = chatListModel->getSnippet(index);

    ChatLogView *chatView = 0;
    if (tabs.contains(chat->name()))
    {        
        int idx = ui->tabWidget->indexOf(tabs[chat->name()].first);
        if (idx != -1)
        {
            ui->tabWidget->setCurrentIndex(idx);
            chatView = tabs[chat->name()].second;
        } else
            tabs.remove(chat->name());
    }

    if (chatView == 0)
    {
        QWidget *tab = new QWidget(ui->tabWidget, Qt::SubWindow);
        QVBoxLayout *layout = new QVBoxLayout(tab);

        chatView = new ChatLogView(tab, chat);

        layout->addWidget(chatView);
        tab->setLayout(layout);
        int idx = ui->tabWidget->addTab(tab, chat->name());
        tabs[chat->name()] = qMakePair(tab, chatView);

        ui->tabWidget->setCurrentIndex(idx);
    }

    if (snippet)
        chatView->setPosition(snippet->item);

}

void MainWindow::contactListClicked ( const QModelIndex & index )
{
    QStringList selected;
    foreach (const QModelIndex& idx, ui->listView->selectionModel()->selectedIndexes())
        selected.append(itemModel->data(idx, Qt::DisplayRole).toString());

    chatListModel->setParticipantFilter( selected );
    chatListModel->filter();
}
