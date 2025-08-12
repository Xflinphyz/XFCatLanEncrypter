#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_label_input_linkActivated(const QString &link);

    void on_textEdit_strInput_textChanged();

    void on_textEdit_keyInput_textChanged();

    void on_textEdit_strOutput_textChanged();

    void on_modeChange_clicked();

    void on_goWork_clicked();

    void on_CatLanModeChange_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
