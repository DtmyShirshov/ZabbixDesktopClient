#include "mainwindow.h"
#include "settingwindow.h"
#include <QApplication>
#include <QFile>



#include <QtDebug>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>
#include <QTime>
#include <QDir>

static QTextCodec *logCodec = NULL;
static FILE *logStream = NULL;
QString g_logFilePath = "";

/** @brief For convenient parsing log files, messages have to be formatted as:
 *      level: message (`placeInSource`)
 *  where:
 *      level - Debug, Warning, Critical, Fatal
 *      message - log message
 *      placeInSource - point, where message was emited in format: (`filename:line, function_signature`)
 */
void logging(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = logCodec->fromUnicode(msg);

    QString fileName(context.file);
    fileName.remove(0, fileName.lastIndexOf("\\") + 1);
    fileName.remove(0, fileName.lastIndexOf("/") + 1);
    QByteArray file = logCodec->fromUnicode(fileName);

    QTime time = QTime::currentTime();
    QString formatedTime = time.toString("hh:mm:ss.zzz");
    fprintf(logStream, "%s ", qPrintable(formatedTime));

    switch (type) {
    case QtDebugMsg:
        fprintf(logStream, "Debug: %s \n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(logStream, "Warning: %s \n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(logStream, "Critical: %s \n", localMsg.constData(), file.constData(), context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(logStream, "Fatal: %s \n", localMsg.constData(), file.constData(), context.line, context.function);
        abort();
        break;
    }
    fflush(logStream);
}

int main(int argc, char *argv[])
{
    QByteArray envVar = qgetenv("QTDIR");   //  this variable is only set when run application in QtCreator
       if (envVar.isEmpty()) {
           g_logFilePath = QDir::currentPath() + QDir::separator() + "debug.txt";
           logStream = _wfopen(g_logFilePath.toStdWString().c_str(), L"w");
       } else {
           logStream = stderr;
       }
       logCodec = QTextCodec::codecForName("Windows-1251");
       qInstallMessageHandler(logging);

    QApplication a(argc, argv);
    /* проверка на наличие файла с настройками.
       если файла нет, то запустить окно настроек,
       а если файл есть, то запустить основное окно.
    */
    if(QFile("config.ini").exists())
    {
        qDebug("Файл с настройками найден. Запускаю главное окно.");
        MainWindow w;
        QSettings settings("config.ini", QSettings::IniFormat);
        if(settings.contains("height") && settings.contains("width"))
        {
            int he = settings.value("height").toInt();
            int wi = settings.value("width").toInt();
            w.resize(wi, he);
        }
        w.show();
        return a.exec();
    }
    else
    {
        qDebug("Файла нет");
        SettingWindow sw;
        sw.setModal(true);
        sw.setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        sw.show();
        return a.exec();
    }
}
