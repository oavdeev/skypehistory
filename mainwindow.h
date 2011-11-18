#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

#include "skypedbb.h"
#include "chatlistmodel.h"
#include "chatlogview.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
protected:
    void changeEvent(QEvent *e);

private:
    SkypeDbb::Database database;
    Ui::MainWindow *ui;
    QStandardItemModel *itemModel;
    ChatListModel *chatListModel;

    QMap<QString, QPair<QWidget *, ChatLogView *> > tabs;
public slots:
    void contactListClicked ( const QModelIndex & index );
    void searchResultDoubleClicked ( const QModelIndex & index );
    void searchTextChanged ( const QString& text );
    void tabCloseRequested (int index );
};

#endif // MAINWINDOW_H
