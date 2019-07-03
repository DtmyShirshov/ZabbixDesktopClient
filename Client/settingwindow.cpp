#include "settingwindow.h"
#include "ui_settingwindow.h"
#include "json.h"
#include <QProcess>
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include <QFlags>

SettingWindow::SettingWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/new/ZabbixIco.ico"));
    QSettings settings("config.ini", QSettings::IniFormat);
    /* Если файл есть(при запуске настроек из основного окна), то подставить сохраненные значения */
    if(QFile("config.ini").exists())
    {
        QString log = settings.value("login").toString();
        QString pass = settings.value("password").toString();
        QString IP = settings.value("IP").toString();
        int timerInterval = settings.value("timer").toInt();

        this->ui->log->setText(log);
        this->ui->pass->setText(pass);
        this->ui->IP->setText(IP);

        switch (timerInterval)
        {
        case 10:
            this->ui->comboBox->setCurrentText("10");
            break;
        case 15:
            this->ui->comboBox->setCurrentText("15");
            break;
        case 20:
            this->ui->comboBox->setCurrentText("20");
            break;
        case 25:
            this->ui->comboBox->setCurrentText("25");
            break;
        case 30:
            this->ui->comboBox->setCurrentText("30");
            break;
        }
    }

}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::on_pushButton_2_clicked()
{
    this->close();
}
/* сохранение настроек в файл */
void SettingWindow::on_pushButton_clicked()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    json j;
    QString log = ui->log->text();
    QString pass = ui->pass->text();
    QString IP = ui->IP->text();

    /* защита от пустых полей */
    if(log != "" && pass != "" && IP != "")
    {
        QJsonObject jAuth = j.Authorization(IP, log, pass);
        /* если авторизация прошла без ошибок, то сохранить и перезапустить,
         * если ошибка есть, то вывести её на экран */

        if(jAuth.contains("error"))
        {
            QJsonObject error = jAuth["error"].toObject();
            QMessageBox* msg = new QMessageBox(this);
            msg->setWindowTitle("Ошибка авторизации!");
            msg->setText(error["data"].toString());
            msg->open();
        }
        else
        {
           if(jAuth["result"] == QJsonValue())
           {
               QMessageBox* msg = new QMessageBox(this);
               msg->setWindowTitle("Ошибка ввода!");
               msg->setText("Проверьте соединение или введенный URL!");
               msg->setModal(true);
               msg->open();
           }
           else
           {
               QMessageBox* msg = new QMessageBox(this);
               msg->setWindowTitle("Подтвердите действие!");
               msg->setText("Приложение будет перезапущено!");
               msg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
               msg->setButtonText(QMessageBox::Yes, tr("Ок"));
               msg->setButtonText(QMessageBox::Cancel, tr("Отмена"));
               msg->open();

               if(msg->exec() == QMessageBox::Yes)
               {
                   /* сохранение логина пароля и url*/
                   settings.setValue("login", log);
                   settings.setValue("password", pass);
                   settings.setValue("IP", IP);
                   /* сохранение интервала таймера */
                   switch (ui->comboBox->currentIndex())
                   {
                   case 0:
                       settings.setValue("timer", 10);
                       break;
                   case 1:
                       settings.setValue("timer", 15);
                       break;
                   case 2:
                       settings.setValue("timer", 20);
                       break;
                   case 3:
                       settings.setValue("timer", 25);
                       break;
                   case 4:
                       settings.setValue("timer", 30);
                       break;
                   default:
                       break;
                   }

                   QThread::sleep(1);


                   qDebug("Сохранил настройки");

                   //перезапуск приложения!
                   qApp->quit();
                   QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
               }
               else
               {
                  msg->close();
               }
           }
        }
    }
    else
    {
        QMessageBox* msg = new QMessageBox(this);
        msg->setWindowTitle("Ошибка ввода!");
        msg->setText("Обязательно введите логин, пароль и URL!");
        msg->setModal(true);
        msg->open();
    }
}
