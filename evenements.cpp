#include "evenements.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>
#include <QSqlError>
#include <QDate>
#include <QMessageBox>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QLabel>
#include <QScreen>
#include <QApplication>
#include <QRect>
evenement::evenement() {
    nom = "";
    date_debut = QDate();
    date_fin = QDate();
    lieu = "";
    capacite = 0;
    sponsors = "";
    budget = 0.0f;
}

evenement::evenement(QString nom, QDate date_debut, QDate date_fin, QString lieu, int capacite, QString sponsors, float budget) {
    this->nom = nom;
    this->date_debut = date_debut;
    this->date_fin = date_fin;
    this->lieu = lieu;
    this->capacite = capacite;
    this->sponsors = sponsors;
    this->budget = budget;
}

bool evenement::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO IMENE.EVENEMENTS (NOM, DATE_DEBUT, DATE_FIN, LIEU, CAPACITE, SPONSORS, BUDGET)"
                  "VALUES (:nom, :date_debut, :date_fin, :lieu, :capacite, :sponsors, :budget)");

    query.bindValue(":nom", nom);
    query.bindValue(":date_debut", date_debut);
    query.bindValue(":date_fin", date_fin);
    query.bindValue(":lieu", lieu);
    query.bindValue(":capacite", capacite);
    query.bindValue(":sponsors", sponsors);
    query.bindValue(":budget", budget);

    bool success = query.exec();
    if (!success) {
        qDebug() << "Insert query failed:" << query.lastError().text();
        // Optional: Show error to user
        QMessageBox::critical(nullptr, "Erreur", "Échec de l'ajout: " + query.lastError().text());
    }
    return success;
}

bool evenement::modifier(QString old_nom) {
    QSqlQuery query;
    query.prepare("UPDATE IMENE.EVENEMENTS SET NOM=:nom, DATE_DEBUT=:date_debut, DATE_FIN=:date_fin, "
                  "BUDGET=:budget, CAPACITE=:capacite, LIEU=:lieu, SPONSORS=:sponsors WHERE NOM=:old_nom");

    query.bindValue(":old_nom", old_nom);
    query.bindValue(":nom", nom);
    query.bindValue(":date_debut", date_debut);
    query.bindValue(":date_fin", date_fin);
    query.bindValue(":budget", budget);
    query.bindValue(":capacite", capacite);
    query.bindValue(":lieu", lieu);
    query.bindValue(":sponsors", sponsors);

    bool success = query.exec();
    if (!success) {
        qDebug() << "Update query failed:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Erreur", "Échec de la modification: " + query.lastError().text());
    }
    return success;
}

QSqlQueryModel* evenement::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM IMENE.EVENEMENTS");

    if (model->lastError().isValid()) {
        qDebug() << "Display query failed:" << model->lastError().text();
        return nullptr;
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID_EV"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("NOM"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("DATE_DEBUT"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("DATE_FIN"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("LIEU"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("CAPACITE"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("SPONSORS"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("BUDGET"));

    return model;
}

bool evenement::supprimer(QString nom) {
    QSqlQuery query;
    query.prepare("DELETE FROM IMENE.EVENEMENTS WHERE NOM = :nom");
    query.bindValue(":nom", nom);

    bool success = query.exec();
    if (!success) {
        qDebug() << "Delete query failed:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Erreur", "Échec de la suppression: " + query.lastError().text());
    }
    return success;
}

void evenement::afficherStatistiques(QWidget *parent) {
    // Create a new dialog window
    QDialog *statsDialog = new QDialog(parent);
    statsDialog->setWindowTitle("Statistiques des Événements");
    statsDialog->setMinimumSize(1250, 450);  // Adjust size as needed

    // Create layout for the dialog
    QHBoxLayout *layout = new QHBoxLayout(statsDialog);

    // First chart: Distribution by Location
    QSqlQuery locationQuery;
    locationQuery.prepare("SELECT LIEU, COUNT(*) FROM IMENE.EVENEMENTS GROUP BY LIEU");

    if (!locationQuery.exec()) {
        qDebug() << "Location statistics query failed:" << locationQuery.lastError().text();
        QMessageBox::critical(parent, "Erreur", "Échec du chargement des statistiques de lieux: " + locationQuery.lastError().text());
        delete statsDialog;
        return;
    }

    QPieSeries *locationSeries = new QPieSeries();
    locationSeries->setName("Distribution par Lieu");
    while (locationQuery.next()) {
        QString location = locationQuery.value(0).toString();
        int count = locationQuery.value(1).toInt();
        locationSeries->append(location, count);
    }

    // Second chart: Distribution by Sponsors
    QSqlQuery sponsorQuery;
    sponsorQuery.prepare("SELECT SPONSORS, COUNT(*) FROM IMENE.EVENEMENTS GROUP BY SPONSORS");

    if (!sponsorQuery.exec()) {
        qDebug() << "Sponsor statistics query failed:" << sponsorQuery.lastError().text();
        QMessageBox::critical(parent, "Erreur", "Échec du chargement des statistiques de sponsors: " + sponsorQuery.lastError().text());
        delete statsDialog;
        return;
    }

    QPieSeries *sponsorSeries = new QPieSeries();
    sponsorSeries->setName("Distribution par Sponsors");
    while (sponsorQuery.next()) {
        QString sponsor = sponsorQuery.value(0).toString();
        int count = sponsorQuery.value(1).toInt();
        sponsorSeries->append(sponsor, count);
    }

    // Customize both series
    for (QPieSlice *slice : locationSeries->slices()) {
        slice->setLabel(slice->label() + " (" + QString::number(slice->percentage() * 100, 'f', 1) + "%)");
    }

    for (QPieSlice *slice : sponsorSeries->slices()) {
        slice->setLabel(slice->label() + " (" + QString::number(slice->percentage() * 100, 'f', 1) + "%)");
    }

    // Create charts
    QChart *locationChart = new QChart();
    locationChart->addSeries(locationSeries);
    locationChart->setTitle("Répartition par Lieu");
    locationChart->legend()->setVisible(true);
    locationChart->legend()->setAlignment(Qt::AlignRight);

    QChart *sponsorChart = new QChart();
    sponsorChart->addSeries(sponsorSeries);
    sponsorChart->setTitle("Répartition par Sponsors");
    sponsorChart->legend()->setVisible(true);
    sponsorChart->legend()->setAlignment(Qt::AlignRight);

    // Create chart views
    QChartView *locationChartView = new QChartView(locationChart);
    locationChartView->setRenderHint(QPainter::Antialiasing);
    locationChartView->setMinimumSize(600, 400);

    QChartView *sponsorChartView = new QChartView(sponsorChart);
    sponsorChartView->setRenderHint(QPainter::Antialiasing);
    sponsorChartView->setMinimumSize(600, 400);

    // Add charts to layout
    layout->addWidget(locationChartView);
    layout->addWidget(sponsorChartView);

    // Set the layout and show the dialog
    statsDialog->setLayout(layout);
    statsDialog->show();
}

QSqlQueryModel* evenement::searchByName(const QString &name)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT * FROM evenements WHERE nom LIKE :name");
    query.bindValue(":name", "%" + name + "%");  // Use LIKE for partial matches
    query.exec();
    model->setQuery(query);
    return model;
}

/*
QSqlQueryModel* evenement::getPastEvents() {
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare("SELECT * FROM IMENE.EVENEMENTS WHERE DATE_FIN < :currentDate");
    query.bindValue(":currentDate", QDate::currentDate());

    if (!query.exec()) {
        qDebug() << "Error retrieving past events:" << query.lastError().text();
        delete model;  // Prevent memory leak
        return nullptr;
    }

    model->setQuery(std::move(query));  // ✅ Fix the deprecated setQuery()
    return model;
}
*/


QSqlQueryModel* evenement::trier(const QString& critere, const QString& ordre) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QString queryStr = "SELECT * FROM IMENE.evenements ORDER BY " + critere + " " + ordre;
    model->setQuery(queryStr);

    if (model->lastError().isValid()) {
        qCritical() << "Error executing query:" << model->lastError().text();
        delete model;
        return nullptr;
    }

    return model;
}

