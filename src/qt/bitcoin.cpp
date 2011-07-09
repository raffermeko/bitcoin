/*
 * W.J. van der Laan 2011
 */
#include "bitcoingui.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "qtwin.h"

#include "headers.h"
#include "init.h"

#include <QApplication>
#include <QMessageBox>
#include <QThread>
#include <QLocale>
#include <QTranslator>

// Need a global reference for the notifications to find the GUI
BitcoinGUI *guiref;

int MyMessageBox(const std::string& message, const std::string& caption, int style, wxWindow* parent, int x, int y)
{
    // Message from main thread
    if(guiref)
    {
        guiref->error(QString::fromStdString(caption),
                      QString::fromStdString(message));
    }
    else
    {
        QMessageBox::critical(0, QString::fromStdString(caption),
            QString::fromStdString(message),
            QMessageBox::Ok, QMessageBox::Ok);
    }
    return 4;
}

int ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style, wxWindow* parent, int x, int y)
{
    // Message from network thread
    if(guiref)
    {
        QMetaObject::invokeMethod(guiref, "error", Qt::QueuedConnection,
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(message)));
    }
    else
    {
        printf("%s: %s\n", caption.c_str(), message.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
    }
    return 4;
}

bool ThreadSafeAskFee(int64 nFeeRequired, const std::string& strCaption, wxWindow* parent)
{
    if(!guiref)
        return false;
    if(nFeeRequired < MIN_TX_FEE || nFeeRequired <= nTransactionFee || fDaemon)
        return true;
    bool payFee = false;

    // Call slot on GUI thread.
    // If called from another thread, use a blocking QueuedConnection.
    Qt::ConnectionType connectionType = Qt::DirectConnection;
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        connectionType = Qt::BlockingQueuedConnection;
    }

    QMetaObject::invokeMethod(guiref, "askFee", connectionType,
                               Q_ARG(qint64, nFeeRequired),
                               Q_ARG(bool*, &payFee));

    return payFee;
}

void CalledSetStatusBar(const std::string& strText, int nField)
{
    // Only used for built-in mining, which is disabled, simple ignore
}

void UIThreadCall(boost::function0<void> fn)
{
    // Only used for built-in mining, which is disabled, simple ignore
}

void MainFrameRepaint()
{
}

/*
   Translate string to current locale using Qt.
 */
std::string _(const char* psz)
{
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(bitcoin);
    QApplication app(argc, argv);

    // Load language file for system locale
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load("bitcoin_"+locale);
    app.installTranslator(&translator);

    app.setQuitOnLastWindowClosed(false);

    try
    {
        if(AppInit2(argc, argv))
        {
            {
                // Put this in a block, so that BitcoinGUI is cleaned up properly before
                // calling shutdown.
                BitcoinGUI window;
                ClientModel clientModel(pwalletMain);
                WalletModel walletModel(pwalletMain);

                guiref = &window;
                window.setClientModel(&clientModel);
                window.setWalletModel(&walletModel);

#ifdef Q_WS_WIN32
                // Windows-specific customization
                window.setAttribute(Qt::WA_TranslucentBackground);
                window.setAttribute(Qt::WA_NoSystemBackground, false);
                QPalette pal = window.palette();
                QColor bg = pal.window().color();
                bg.setAlpha(0);
                pal.setColor(QPalette::Window, bg);
                window.setPalette(pal);
                window.ensurePolished();
                window.setAttribute(Qt::WA_StyledBackground, false);
#endif
                if (QtWin::isCompositionEnabled())
                {
                    QtWin::extendFrameIntoClientArea(&window);
                    window.setContentsMargins(0, 0, 0, 0);
                }

                window.show();

                app.exec();

                guiref = 0;
            }
            Shutdown(NULL);
        }
        else
        {
            return 1;
        }
    } catch (std::exception& e) {
        PrintException(&e, "Runaway exception");
    } catch (...) {
        PrintException(NULL, "Runaway exception");
    }
    return 0;
}
