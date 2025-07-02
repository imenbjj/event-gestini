#ifndef EVENEMENTS_H
#define EVENEMENTS_H
#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include<QDate>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QDialog>
#include <QHBoxLayout>
#include <QSystemTrayIcon>
#include <QTimer>

class evenement
{
    int id_ev;
    QString nom;
    QDate date_debut;
    QDate date_fin;
    QString lieu;
    int capacite;
    QString sponsors;
    float budget;

public:
    evenement();  // Default constructor
    evenement(QString, QDate, QDate, QString, int, QString, float);  // Correct constructor
    bool modifier(QString old_nom);
    QSqlQueryModel* searchByName(const QString &name);
    QSqlQueryModel* getPastEvents();
    QSqlQueryModel* afficher();
    QSqlQueryModel* trier(const QString& critere, const QString& ordre);

    bool ajouter();
    bool supprimer(QString nom);
    void afficherStatistiques(QWidget *parent);
    // In evenements.h
    //void notifierProchainEvenement(QWidget *parent, QSystemTrayIcon *trayIcon);
    void notifierProchainEvenement(QWidget *parent);
    // Getters and Setters
    int Getid() { return id_ev; }
    void Setid(int val) { id_ev = val; }
    QString Getnom() { return nom; }
    void Setnom(QString val) { nom = val; }
    QDate Getdate_debut() { return date_debut; }
    void Setdate_debut(QDate val) { date_debut = val; }
    QDate Getdate_fin() { return date_fin; }
    void Setdate_fin(QDate val) { date_fin = val; }
    QString Getlieu() { return lieu; }
    void Setlieu(QString val) { lieu = val; }
    int Getcapacite() { return capacite; }
    void Setcapacite(int val) { capacite = val; }
    QString Getsponsors() { return sponsors; }
    void Setsponsors(QString val) { sponsors = val; }
    float Getbudget() { return budget; }
    void Setbudget(float val) { budget = val; }

    //bool supprimer(int);

};

#endif // EVENEMENTS_H
