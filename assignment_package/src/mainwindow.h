#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "component.h"
#include <QMainWindow>
#include "smartpointerhelp.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

    void slot_addHeItems(std::vector<QListWidgetItem*>&);
    void slot_addVertexItems(std::vector<QListWidgetItem*>&);
    void slot_addFaceItems(std::vector<QListWidgetItem*>&);

    void slot_addRootToTreeWidget(QTreeWidgetItem *i);

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
