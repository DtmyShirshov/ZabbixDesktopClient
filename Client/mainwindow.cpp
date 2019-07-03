#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingwindow.h"
#include "json.h"
#include <QtWidgets>
#include <QTimer>
#include <QApplication>
#include <QString>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QApplication::setQuitOnLastWindowClosed(false);

    QSettings settings("config.ini", QSettings::IniFormat);

    int timerInterval = settings.value("timer").toInt() * 1000;

    /* запоминаем время запуска приложения */
    launchDateTime = (int)QDateTime::currentDateTime().toTime_t();

    jsn.Authorization(settings.value("IP").toString(), settings.value("login").toString(), settings.value("password").toString());

    ui->setupUi(this);
    this->setWindowTitle("ZabbixDesktopClient");
    this->setWindowIcon(QIcon(":/new/ZabbixIco.ico"));

    /* Задаем иконку и подсказку при наведении на неё */
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/new/ZabbixIco.ico"));
    trayIcon->setToolTip("ZabbixDesktopClient");

    /* Создаем контекстное меню */
    QMenu* menu = new QMenu(this);

    QAction* hideWindow = new QAction("Свернуть окно", this);
    QAction* viewWindow = new QAction("Развернуть окно", this);
    QAction* closeMsg = new QAction("Закрыть все оповещения", this);
    QAction* quitAction = new QAction("Выход", this);

    /* подключаем сигналы нажатий на пункты меню к соответсвующим слотам.
       Первый пункт меню разворачивает приложение из трея,
       второй пункт сворачивает приложение в трей,
       третий пункт меню завершает приложение.
    */
    connect(viewWindow, SIGNAL(triggered()), this, SLOT(show()));
    connect(hideWindow, SIGNAL(triggered()), this, SLOT(HideApp()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(ExitApp()));
    connect(closeMsg, SIGNAL(triggered()), this, SLOT(CloseAllMsg()));

    /* Добавляем кнопки в контекстное меню */
    menu->addAction(viewWindow);
    menu->addAction(hideWindow);
    menu->addAction(closeMsg);
    menu->addAction(quitAction);

    /* Задаем контекстное меню кнопке в трее и показываем её */
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    /* прячем 1 и 4 колонку */
    ui->tableWidget->setColumnHidden(1, true);
    ui->tableWidget->setColumnHidden(4, true);
    /* задаем ширину стобцов и из заголовки, запрещаем изменять ширину */
    QStringList name_table;
    name_table << "Дата/время" << "" << "Узел сети" << "Проблема";
    ui->tableWidget->setHorizontalHeaderLabels(name_table);
    ui->tableWidget->setColumnWidth(0, 140);
    ui->tableWidget->setColumnWidth(2, 250);
    //ui->tableWidget->setColumnWidth(3, 850);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);



    sorting = "time";

    sound = new QSound(":\\new\\alarm_02.wav");

    /* Объявляем таймер, задаем ему интервал, подключаем сигнал таймаута к слоту*/
    QTimer* timerGetProblems = new QTimer(this);
    timerGetProblems->start(timerInterval);
    connect(timerGetProblems, SIGNAL(timeout()), this, SLOT(GetProblems()));

    QTimer* timerDeleteProblems = new QTimer(this);
    timerDeleteProblems->start(timerInterval * 2);
    connect(timerDeleteProblems, SIGNAL(timeout()), this, SLOT(DeleteResolvedProblems()));

    connect(ui->tableWidget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(resizeRows()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeRows()
{
    ui->tableWidget->resizeRowsToContents();
}

void MainWindow::GetProblems()
{
    QJsonArray problems = jsn.GetProblems(launchDateTime);

    if(!problems.isEmpty())
    {
        for(auto i = problems.begin(); i != problems.end(); ++i)
        {
            QJsonObject jObj = i->toObject();
            SetItems(jObj);
        }
    }
}

void MainWindow::SetItems(QJsonObject problem)
{
    if(!alreadyExists.contains(problem["triggerid"]))
    {
        QList<QTableWidgetItem*> item = ui->tableWidget->findItems(problem["triggerid"].toString(), Qt::MatchExactly);
        if(!item.isEmpty())
        {
            alreadyExists.append(problem["triggerid"]);
        }
        else
        {
            QString description  = problem["description"].toString();

            QString priority = problem["priority"].toString();

            QString time = QDateTime::fromTime_t(problem["lastchange"].toString().toLongLong()).toString("dd.MM.yy / hh:mm:ss");

            QJsonArray hostArr = problem["hosts"].toArray();
            QString host = hostArr.at(0)["name"].toString();

            if(description.contains("{HOST.NAME}"))
            {
                int index = description.indexOf("{HOST.NAME}");
                description.replace(index, 11 , host);
            }

            if(description.contains("{ITEM.LASTVALUE}"))
            {
                int index = description.indexOf("{ITEM.LASTVALUE}");
                description.replace(index, 17 , problem["items"].toArray().at(0)["lastvalue"].toString());
            }

            QTableWidgetItem* itemSeveritity = new QTableWidgetItem();
            itemSeveritity->setText(priority);

            QTableWidgetItem* itemDescription = new QTableWidgetItem();
            itemDescription->setText(description);

            QColor color;
            if(priority == "1")
            {
                color.setRgb(30,144,255,255);
                itemDescription->setBackgroundColor(color);
            }
            if(priority == "2")
            {
                color.setRgb(255,215,0,255);
                itemDescription->setBackgroundColor(color);
            }
            if(priority == "3")
            {
                color.setRgb(255,165,0,255);
                itemDescription->setBackgroundColor(color);
            }
            if(priority == "4")
            {
                color.setRgb(255,99,71,255);
                itemDescription->setBackgroundColor(color);
            }
            if(priority == "5")
            {
                color.setRgb(255,0,0,255);
                itemDescription->setBackgroundColor(color);
            }

            QTableWidgetItem* itemHost = new QTableWidgetItem();
            itemHost->setText(host);

            QTableWidgetItem* itemDateTime = new QTableWidgetItem();
            itemDateTime->setText(time);

            QTableWidgetItem* itemTrigger_id = new QTableWidgetItem();
            itemTrigger_id->setText(problem["triggerid"].toString());

            /* вывод проблемы в таблицу */
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
            ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()+1, 80);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, itemHost);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, itemDescription);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, itemSeveritity);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, itemDateTime);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 4, itemTrigger_id);
            resizeRows();



            /* сортировка записей по времени в соответствии с настроеным фильтром */
            if(sorting == "time")
            {
                ui->tableWidget->sortItems(0, Qt::DescendingOrder);
            }
            if(sorting == "severitity")
            {
                ui->tableWidget->sortItems(0, Qt::DescendingOrder);
                ui->tableWidget->sortItems(1 , Qt::DescendingOrder);
            }

            int closeTimeOut;

            switch (ui->comboBox->currentIndex())
            {
            case 0:
                closeTimeOut = 10 * 60000;
                break;
            case 1:
                closeTimeOut = 30 * 60000;
                break;
            case 2:
                closeTimeOut = 60 * 60000;
                break;
            case 3:
                closeTimeOut = 1 * 60000;
                break;
            default:
                closeTimeOut = 30 * 60000;
                break;
            }

            QMessageBox* msg = new QMessageBox(this);
            msg->setStandardButtons(QMessageBox::Ok);
            msg->button(QMessageBox::Ok)->animateClick(closeTimeOut);
            msg->setWindowFlag(Qt::WindowStaysOnTopHint);

            /* вывод оповещения в соответствии с настроеным фильтром */
            if(ui->checkBox->isChecked() == true && priority == "1")
            {
                msg->setWindowTitle("Информация!");
                msg->setText(time + "\n" + host + "\n" + description);
                msg->show();
            }
            if(ui->checkBox_2->isChecked() == true && priority == "2")
            {
                msg->setWindowTitle("Предупреждение!");
                msg->setText(time + "\n" + host + "\n" + description);
                msg->show();
            }
            if(ui->checkBox_3->isChecked() == true && priority == "3")
            {
                msg->setWindowTitle("Средняя!");
                msg->setText(time + "\n" + host + "\n" + description);
                msg->show();
            }
            if(ui->checkBox_4->isChecked() == true && priority == "4")
            {
                msg->setWindowTitle("Высокая!");
                msg->setText(time + "\n" + host + "\n" + description);
                msg->show();
            }
            if(ui->checkBox_5->isChecked() == true && priority == "5")
            {
                msg->setWindowTitle("Чрезвычайная!");
                msg->setText(time + "\n" + host + "\n" + description);
                msg->show();
            }
            msglist.append(msg);
            sound->play();
        }
    }
    ui->label_2->setText("Проблем показано: " + QString::number(ui->tableWidget->rowCount()));
}

void MainWindow::DeleteResolvedProblems()
{
    if(!alreadyExists.isEmpty())
    {
        QJsonArray problems = jsn.GetProblems(launchDateTime);
        QJsonArray triggerIDs;
        for(auto i = problems.begin(); i != problems.end(); ++i)
        {
            QJsonObject jObj = i->toObject();
            triggerIDs.append(jObj["triggerid"]);
        }

        for(int i = 0; i < alreadyExists.count(); ++i)
        {
            if(!triggerIDs.contains(alreadyExists.at(i)))
            {
                QList<QTableWidgetItem*> item = ui->tableWidget->findItems(alreadyExists.at(i).toString(), Qt::MatchExactly);

                if(!item.empty())
                {
                    for(int j = 0; j < item.count(); j++)
                    {
                     int row = item.at(j)->row();
                     ui->tableWidget->removeRow(row);
                    }
                    alreadyExists.removeAt(i);
                }
            }
        }
        ui->label_2->setText("Проблем показано: " + QString::number(ui->tableWidget->rowCount()));
    }
}



/* Метод, который обрабатывает событие закрытия окна приложения */
void MainWindow::closeEvent(QCloseEvent * event)
{
    /* Если окно видимо, то завершение приложения игнорируется, а окно просто скрывается */
    if(this->isVisible())
    {
        event->ignore();
        this->HideApp();
    }
}

/* Метод, который обрабатывает нажатие на иконку приложения в трее */
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
    /* если окно видимо, то оно скрывается, и наоборот, если скрыто, то разворачивается на экран */
        if(!this->isVisible())
        {
            this->show();
        }
        else
        {
            hide();
        }
        break;
    default:
        break;
    }
}

void MainWindow::on_Settings_triggered()
{
    SettingWindow sw;
    sw.setModal(true);
    sw.setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    sw.exec();
}

void MainWindow::on_HideApp_triggered()
{
    HideApp();
}

void MainWindow::on_Exit_triggered()
{
    ExitApp();
}

/* Выход из приложения с подтверждением действия */
void MainWindow::ExitApp()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    /* проверка, нажимал ли пользователь на "Больше не спрашивать?"
       если да, то выйти из приложения без подтверждения действия,
       если нет, то запросить подтверждение.
    */
    settings.setValue("height", QString::number(this->height()));
    settings.setValue("width", QString::number(this->width()));

    if(settings.value("dontShowExitMSG").toBool() == true)
    {
        QApplication::quit();
    }
    else
    {
        QCheckBox* cb = new QCheckBox("Больше не спрашивать");

        QMessageBox* msgE = new QMessageBox(this);
        msgE->setWindowTitle("Подвердите дейстиве");
        msgE->setText("Вы уверены, что хотите выйти?");
        msgE->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgE->setButtonText(QMessageBox::Yes, tr("Да"));
        msgE->setButtonText(QMessageBox::No, tr("Нет"));
        msgE->setCheckBox(cb);
        msgE->show();

        if(msgE->exec() == QMessageBox::Yes)
        {
           QApplication::quit();
        }

        if(cb->isChecked() == true)
        {
            settings.setValue("dontShowExitMSG", true);
        }
     }
}

void MainWindow::CloseAllMsg()
{
    for(int i = 0; i < msglist.count()-1; ++i)
    {
        try
        {
            msglist.at(i)->close();
        }
        catch (const std::exception& e)
        {
            qDebug() << e.what();
        }

    }
    msglist.clear();
}

/* Сворачивание приложения в трей с подтверждением действия */
void MainWindow::HideApp()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    /* проверка, нажимал ли пользователь на "Больше не спрашивать?"
       если да, то свернуть приложение без подтверждения действия,
       если нет, то запросить подтверждение.
    */
    if(settings.value("dontShowHideMSG").toBool() == true)
    {
        hide();
    }
    else
    {
        if(this->isVisible())
        {
            QCheckBox* cb = new QCheckBox("Больше не спрашивать");

            QMessageBox *msgH = new QMessageBox(this);
            msgH->setWindowTitle("Подвердите действие");
            msgH->setText("Приложение будет свернуто в трей!");
            msgH->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgH->setButtonText(QMessageBox::Yes, tr("Да"));
            msgH->setButtonText(QMessageBox::No, tr("Нет"));
            msgH->setCheckBox(cb);
            msgH->show();

            if(msgH->exec() == QMessageBox::Yes)
            {
               hide();
            }

            if(cb->isChecked() == true)
            {
                settings.setValue("dontShowHideMSG", true);
            }
        }
    }
}

void MainWindow::on_radioButton_clicked()
{
    sorting = "time";
    ui->tableWidget->sortItems(0,Qt::DescendingOrder);
}

void MainWindow::on_radioButton_2_clicked()
{
    sorting = "severitity";
    ui->tableWidget->sortItems(1, Qt::DescendingOrder);
}


