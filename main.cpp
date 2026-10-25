#include "connection.h"
#include "mainwindow.h"
#include "theme.h"

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    a.setStyleSheet(Theme::feuilleDeStyle());

    // On récupère l'instance unique du singleton
    Connection& c = Connection::createInstance();

    // On tente d'établir la connexion AVANT de créer la fenêtre
    // (le constructeur de la vue appelle afficher())
    bool test = c.createConnection();
    QString erreurOracle;
    if (!test) {
        erreurOracle = QSqlDatabase::database(QSqlDatabase::defaultConnection, false).lastError().text();
        c.createDemoConnection();   // secours : base de démonstration en mémoire
    }

    MainWindow W;
    W.show();

    //************************Test Connection BD**************************//
    if (test) {
        QMessageBox::information(&W, QObject::tr("Base de données"),
                                 QObject::tr("Connexion réussie."));
    } else {
        QMessageBox::warning(&W, QObject::tr("Base de données"),
                             QObject::tr("Échec de la connexion à Oracle.\n"
                                         "L'application fonctionne avec une base de démonstration "
                                         "(les modifications ne sont pas enregistrées dans Oracle).\n\n")
                                 + erreurOracle);
    }

    return a.exec();
}
