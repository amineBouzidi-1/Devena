#include "connection.h"
#include "basedemo.h"

// Ctor : initialise l'attribut `db` avec le driver ODBC (stocké dans le Singleton)
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor : ferme la connexion si elle est encore ouverte
Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

// Accès à l'instance unique du Singleton (Meyers Singleton)
Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    db.setDatabaseName("Source_Projet2A"); // nom de la source de données ODBC (64 bits)
    db.setUserName("amine");               // nom d'utilisateur Oracle
    db.setPassword("amine123");            // mot de passe Oracle

    // Les NUMBER Oracle sont lus directement en double (évite « 4,6 » / « 4.6 »)
    db.setNumericalPrecisionPolicy(QSql::LowPrecisionDouble);

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

// ---------------------------------------------------------------------------
// Mode démonstration : remplace la connexion ODBC par une base SQLite en
// mémoire contenant les mêmes tables (EQUIPE, EMPLOYE, EVALUATION).
// Le code de la classe Evaluation reste identique : seules les données changent.
// ---------------------------------------------------------------------------
bool Connection::createDemoConnection() {
    const QString nom = db.connectionName();
    if (db.isOpen())
        db.close();
    db = QSqlDatabase();                    // libérer la référence avant removeDatabase
    QSqlDatabase::removeDatabase(nom);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        qDebug() << "Base de démonstration impossible :" << db.lastError().text();
        return false;
    }
    return BaseDemo::initialiser();
}

bool Connection::estModeDemo() {
    return QSqlDatabase::database(QSqlDatabase::defaultConnection, false).driverName() == "QSQLITE";
}
