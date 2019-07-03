#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::SettingWindow *ui;
};

#endif // SETTINGWINDOW_H
