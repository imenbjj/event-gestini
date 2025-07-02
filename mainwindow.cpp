#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "evenements.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include "HistoryDialog.h"
#include <QIcon>
#include <QSize>
#include <QDate>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    evenement e;

    QSqlQueryModel* model = e.afficher();

    connect(ui->comboBox_2, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onComboBox2IndexChanged);

    if (model) {
        ui->tableView4->setModel(model);
        ui->tableView4->setSortingEnabled(true);  //
    } else {
        QMessageBox::critical(this, "Erreur", "Échec du chargement des données initiales");
    }

    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);
    //checkUpcomingEvents();
    ui->pushButtonNotification->setIcon(QIcon(":/images/cloche.png"));
    ui->pushButtonNotification->setIconSize(QSize(24, 24)); // Ajuste la taille
    ui->pushButtonNotification->setFlat(true);
    connect(ui->pushButtonNotification, &QPushButton::clicked, this, [this]() {
        checkUpcomingEvents(); // Check for upcoming events when the button is clicked
    });

}

MainWindow::~MainWindow() {
    delete ui;
}

// Definition of the onRowSelected slot
void MainWindow::onRowSelected()
{
    QModelIndexList selectedRows = ui->tableView4->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) return;

    QModelIndex index = selectedRows.first();

    QString nom = ui->tableView4->model()->data(index.sibling(index.row(), 1)).toString();
    QDate dateDebut = ui->tableView4->model()->data(index.sibling(index.row(), 2)).toDate();
    QDate dateFin = ui->tableView4->model()->data(index.sibling(index.row(), 3)).toDate();
    QString lieu = ui->tableView4->model()->data(index.sibling(index.row(), 4)).toString();
    int capacite = ui->tableView4->model()->data(index.sibling(index.row(), 5)).toInt();
    QString sponsors = ui->tableView4->model()->data(index.sibling(index.row(), 6)).toString();
    float budget = ui->tableView4->model()->data(index.sibling(index.row(), 7)).toFloat();

    // Remplir les champs du formulaire
    ui->lineedit_nom->setText(nom);
    ui->lineedit_date_debut->setDate(dateDebut);
    ui->lineedit_date_fin->setDate(dateFin);
    ui->comboBox_lieu->setCurrentText(lieu);
    ui->lineedit_capacite->setText(QString::number(capacite));
    ui->comboBox_sponsors->setCurrentText(sponsors);
    ui->lineedit_budget->setText(QString::number(budget));

    // Facultatif : Préremplir le champ de suppression avec le nom
    ui->lineedit_old_nom->setText(nom);
}

void MainWindow::on_pushbutton_ajouter_2_clicked()
{
    QString eventName = ui->lineedit_nom->text();
    QDate startDate = ui->lineedit_date_debut->date();
    QDate endDate = ui->lineedit_date_fin->date();
    float budget = ui->lineedit_budget->text().toFloat();
    int capacity = ui->lineedit_capacite->text().toInt();
    QString venue = ui->comboBox_lieu->currentText();
    QString sponsors = ui->comboBox_sponsors->currentText();

    // Input validation
    if (eventName.isEmpty() || venue.isEmpty() || sponsors.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs obligatoires doivent être remplis.");
        return;
    }

    if (!venue.contains("bloc", Qt::CaseInsensitive)) {
        QMessageBox::warning(this, "Erreur", "Le lieu doit inclure 'bloc' (ex: 'bloc A').");
        return;
    }

    if (capacity < 0) {
        QMessageBox::warning(this, "Erreur", "La capacité ne peut pas être négative.");
        return;
    }

    if (budget < 0) {
        QMessageBox::warning(this, "Erreur", "Le budget ne peut pas être négatif.");
        return;
    }

    if (ui->lineedit_date_debut->date()>ui->lineedit_date_fin->date()) {  // Fixed date comparison logic
        QMessageBox::warning(this, "Erreur", "La date de début doit être antérieure ou égale à la date de fin.");
        return;
    }

    evenement e;
    e.Setnom(eventName);
    e.Setdate_debut(startDate);
    e.Setdate_fin(endDate);
    e.Setbudget(budget);
    e.Setcapacite(capacity);
    e.Setlieu(venue);
    e.Setsponsors(sponsors);

    bool test = e.ajouter();
    if (test) {
        QMessageBox::information(this, "Succès", "Événement ajouté avec succès.");
        ui->tableView4->setModel(e.afficher());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout de l'événement.");
    }
    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);

}

void MainWindow::on_pushButton_supprimer_clicked()
{
    QString eventName = ui->lineedit_old_nom->text();

    if (eventName.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez spécifier le nom de l'événement à supprimer.");
        return;
    }

    evenement e;
    bool test = e.supprimer(eventName);
    if (test) {
        QMessageBox::information(this, "Succès", "Événement supprimé avec succès.");
        ui->tableView4->setModel(e.afficher());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression de l'événement.");
    }
    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);

}

void MainWindow::on_pushButton_modifier_clicked()
{
    QModelIndexList selectedIndexes = ui->tableView4->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un événement à modifier.");
        return;
    }

    QString oldNom = ui->tableView4->model()->data(selectedIndexes.first().sibling(selectedIndexes.first().row(), 1)).toString();

    QString eventName = ui->lineedit_nom->text();
    QDate startDate = ui->lineedit_date_debut->date();
    QDate endDate = ui->lineedit_date_fin->date();
    float budget = ui->lineedit_budget->text().toFloat();
    int capacity = ui->lineedit_capacite->text().toInt();
    QString venue = ui->comboBox_lieu->currentText();
    QString sponsors = ui->comboBox_sponsors->currentText();

    // Input validation (same as ajouter)
    if (eventName.isEmpty() || venue.isEmpty() || sponsors.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs obligatoires doivent être remplis.");
        return;
    }

    if (!venue.contains("bloc", Qt::CaseInsensitive)) {
        QMessageBox::warning(this, "Erreur", "Le lieu doit inclure 'bloc' (ex: 'bloc A').");
        return;
    }

    if (capacity < 0) {
        QMessageBox::warning(this, "Erreur", "La capacité ne peut pas être négative.");
        return;
    }

    if (budget < 0) {
        QMessageBox::warning(this, "Erreur", "Le budget ne peut pas être négatif.");
        return;
    }

    if (startDate > endDate) {
        QMessageBox::warning(this, "Erreur", "La date de début doit être antérieure ou égale à la date de fin.");
        return;
    }

    evenement e;
    e.Setnom(eventName);
    e.Setdate_debut(startDate);
    e.Setdate_fin(endDate);
    e.Setbudget(budget);
    e.Setcapacite(capacity);
    e.Setlieu(venue);
    e.Setsponsors(sponsors);

    bool test = e.modifier(oldNom);
    if (test) {
        QMessageBox::information(this, "Succès", "Événement modifié avec succès.");
        ui->tableView4->setModel(e.afficher());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification de l'événement.");
    }
    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);

}

// Moved to MainWindow class instead of evenement
void MainWindow::on_pushButton_stats_clicked()  // Add this slot in header too
{
    evenement e;
    e.afficherStatistiques(this);
}

void MainWindow::on_pushButton_export_pdf_clicked()
{
    // Get selected row
    QModelIndexList selectedIndexes = ui->tableView4->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un événement à exporter.");
        return;
    }

    // Get data from selected row with QDateTime to include time
    QModelIndex index = selectedIndexes.first();
    QString eventName = ui->tableView4->model()->data(index.sibling(index.row(), 1)).toString();
    QDateTime startDateTime = ui->tableView4->model()->data(index.sibling(index.row(), 2)).toDateTime();  // Include time
    QDateTime endDateTime = ui->tableView4->model()->data(index.sibling(index.row(), 3)).toDateTime();    // Include time
    QString venue = ui->tableView4->model()->data(index.sibling(index.row(), 4)).toString();
    int capacity = ui->tableView4->model()->data(index.sibling(index.row(), 5)).toInt();
    QString sponsors = ui->tableView4->model()->data(index.sibling(index.row(), 6)).toString();
    float budget = ui->tableView4->model()->data(index.sibling(index.row(), 7)).toFloat();

    // Open file dialog for PDF save location
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter l'événement en PDF",
                                                    QDir::homePath() + "/" + eventName + "_event.pdf",
                                                    "PDF Files (*.pdf)");
    if (fileName.isEmpty())
        return;

    // Set up PDF printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(false);

    QPageLayout layout(QPageSize(QPageSize::A4), QPageLayout::Landscape, QMarginsF(60, 60, 60, 60));
    printer.setPageLayout(layout);

    // Start painting on PDF
    QPainter painter;
    if (!painter.begin(&printer)) {
        QMessageBox::warning(this, "Erreur", "Échec de l'ouverture du fichier PDF pour l'écriture");
        return;
    }

    // Set up fonts with enhanced styling
    QFont nameFont("Montserrat", 48, QFont::Bold);  // Larger and bolder for emphasis
    QFont textFont("Montserrat", 18);              // Slightly larger for readability
    QFont footerFont("Montserrat", 14);
    footerFont.setItalic(true);                    // Set italic style after construction

    // Get page dimensions
    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    int pageWidth = pageRect.width();
    int pageHeight = pageRect.height();
    int margin = 150;  // Increased margin for a more spacious look (from previous design)
    int contentWidth = pageWidth - 2 * margin;
    int yPos = margin;

    // Increase header dimensions by 1000 (matching original placement)
    int baseHeaderWidth = contentWidth;
    int baseHeaderHeight = 150;
    int headerWidth = baseHeaderWidth + 1000;
    int headerHeight = baseHeaderHeight + 1000;

    // Cap header width to page boundaries
    if (headerWidth > pageWidth - margin) {
        headerWidth = pageWidth - margin - 20;
    }

    // Draw radial gradient background for header
    QRadialGradient radialGradient(pageWidth / 2, yPos, 400, pageWidth / 2, yPos);
    radialGradient.setColorAt(0, QColor(230, 240, 250));  // Light teal center
    radialGradient.setColorAt(1, QColor(163, 191, 250));  // Soft cyan edge
    painter.setBrush(QBrush(radialGradient));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(margin, yPos - 75, headerWidth, headerHeight, 40, 40);

    // Draw event name with shadow effect
    painter.setFont(nameFont);
    painter.setPen(QColor(0, 109, 119));  // Deep teal
    QRect nameRect(margin, yPos - 25, headerWidth, headerHeight - 50);
    painter.drawText(nameRect, Qt::AlignCenter, eventName);
    painter.setPen(Qt::gray);  // Shadow effect
    painter.drawText(nameRect.translated(3, 3), Qt::AlignCenter, eventName);  // Enhanced shadow
    yPos += headerHeight;

    // Draw modern underline with gradient
    QLinearGradient underlineGradient(margin + 100, yPos, margin + headerWidth - 100, yPos);
    underlineGradient.setColorAt(0, QColor(0, 109, 119));
    underlineGradient.setColorAt(1, QColor(163, 191, 250));
    painter.setPen(QPen(QBrush(underlineGradient), 6));
    painter.drawLine(margin + 100, yPos, margin + headerWidth - 100, yPos);
    yPos += 50;  // Matching original placement

    // Event details with aesthetic styling, including time
    QStringList details;
    details << "Date et heure de début: " + startDateTime.toString("dd/MM/yyyy HH:mm")
            << "Date et heure de fin: " + endDateTime.toString("dd/MM/yyyy HH:mm")
            << "Lieu: " + venue
            << "Capacité: " + QString::number(capacity)
            << "Sponsors: " + sponsors
            << "Budget: " + QString::number(budget, 'f', 2) + " TND";

    painter.setFont(textFont);
    QFontMetrics fm(textFont);
    int lineHeight = fm.height() * 1.5;  // Increased line height for better spacing

    // Detail box dimensions (increased by 1000, matching original placement)
    int baseBoxWidth = contentWidth - 40;
    int baseBoxHeight = lineHeight * 3 + 40;
    int boxWidth = baseBoxWidth + 1000;
    int boxHeight = baseBoxHeight + 1000;

    if (boxWidth > pageWidth - margin) {
        boxWidth = pageWidth - margin - 20;
    }

    painter.setPen(QColor(74, 74, 74));  // Warm gray
    for (const QString& detail : details) {
        QRect detailRect(margin, yPos, boxWidth, boxHeight);
        painter.setBrush(QBrush(QColor(245, 248, 250, 80)));  // Light background with more opacity
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(detailRect, 20, 20);
        painter.setPen(QColor(74, 74, 74));
        painter.drawText(detailRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, detail);

        if (yPos + boxHeight > pageHeight - 150) {
            printer.newPage();
            yPos = margin;
            // Redraw larger header on new page
            painter.setBrush(QBrush(radialGradient));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(margin, yPos - 75, headerWidth, headerHeight, 40, 40);
            painter.setFont(nameFont);
            painter.setPen(QColor(0, 109, 119));
            painter.drawText(nameRect, Qt::AlignCenter, eventName);
            painter.setPen(Qt::gray);
            painter.drawText(nameRect.translated(3, 3), Qt::AlignCenter, eventName);  // Shadow
            yPos += headerHeight;
            painter.setPen(QPen(QBrush(underlineGradient), 6));
            painter.drawLine(margin + 100, yPos, margin + headerWidth - 100, yPos);
            yPos += 50;
            painter.setFont(textFont);
            painter.setPen(QColor(74, 74, 74));
        }

        yPos += boxHeight + 30;  // Matching original placement
    }

    // Draw decorative footer
    painter.setBrush(QBrush(QColor(230, 240, 250, 120)));  // Enhanced footer background
    painter.setPen(Qt::NoPen);
    painter.drawRect(margin, pageHeight - 150, contentWidth, 100);
    painter.setFont(footerFont);
    painter.setPen(Qt::gray);
    QString footer = "Généré le " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    painter.drawText(margin + 20, pageHeight - 120, contentWidth - 40, 50, Qt::AlignLeft, footer);

    // Draw enhanced watermark
    QFont watermarkFont("Montserrat", 60, QFont::Light);
    painter.setFont(watermarkFont);
    QLinearGradient watermarkGradient(0, 0, pageWidth, pageHeight);
    watermarkGradient.setColorAt(0, QColor(0, 109, 119, 15));
    watermarkGradient.setColorAt(1, QColor(163, 191, 250, 5));
    painter.setPen(QPen(watermarkGradient, 2));
    painter.save();
    painter.translate(pageWidth / 2, pageHeight / 2);
    painter.rotate(-45);
    painter.drawText(-pageWidth / 2, -70, pageWidth, 140, Qt::AlignCenter, "Événement");
    painter.restore();

    painter.end();
    QMessageBox::information(this, "Succès", "L'événement a été exporté en PDF avec succès");
    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);

}


void MainWindow::on_pushButton_search_clicked()
{
    QString searchText = ui->lineEdit_search->text().trimmed();
    evenement e;
    if (searchText.isEmpty()) {
        // If search field is empty, show all events
        ui->tableView4->setModel(e.afficher());
    } else {
        // Filter events by name
        ui->tableView4->setModel(e.searchByName(searchText));
    }
}

void MainWindow::on_lineEdit_search_textChanged(const QString &text)
{
    evenement e;
    if (text.trimmed().isEmpty()) {
        // If search field is empty, show all events
        ui->tableView4->setModel(e.afficher());
    } else {
        // Filter events by name
        ui->tableView4->setModel(e.searchByName(text.trimmed()));
    }
    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);

}



void MainWindow::on_pushButton_historique_clicked()
{
    QSqlQuery query;
    query.prepare("SELECT ID_EV, FIELD_NAME, OLD_VALUE, NEW_VALUE, TO_CHAR(CHANGE_DATE, 'YYYY-MM-DD HH24:MI:SS') "
                  "FROM EVENEMENTS_HISTORY ORDER BY CHANGE_DATE DESC");

    if (query.exec()) {
        QStringList history;
        while (query.next()) {
            int id_ev = query.value(0).toInt();
            QString field = query.value(1).toString();
            QString oldVal = query.value(2).toString();
            QString newVal = query.value(3).toString();
            QString date = query.value(4).toString();

            history.append(QString("Event ID %1: Field '%2' changed from '%3' to '%4' on %5")
                               .arg(id_ev).arg(field, oldVal, newVal, date));
        }

        if (history.isEmpty()) {
            QMessageBox::information(this, "Event History", "No changes recorded.");
        } else {
            HistoryDialog *historyDialog = new HistoryDialog(this);
            historyDialog->setHistory(history);  // Pass full history
            historyDialog->exec();  // Show the modal
        }
    } else {
        QMessageBox::critical(this, "Error", query.lastError().text());
    }
    connect(ui->tableView4->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onRowSelected);

}

void MainWindow::onComboBox2IndexChanged(int index)
{
    evenement e;
    QSqlQueryModel* sortedModel = nullptr;

    if (index == 0) { // Tri par lieu (bloc A → B)
        sortedModel = e.trier(QString("lieu"), QString("ASC"));
    }
    else if (index == 1) { // Tri par sponsors (A → Z)
        sortedModel = e.trier(QString("sponsors"), QString("ASC"));
    }
    else if (index == 2) { // Tri par capacité (desc)
        sortedModel = e.trier(QString("capacite"), QString("DESC"));
    }

    if (sortedModel) {
        ui->tableView4->setModel(sortedModel);
    }
}

void MainWindow::checkUpcomingEvents()
{
    QSqlQuery query;
    query.prepare("SELECT * FROM IMENE.EVENEMENTS WHERE DATE_DEBUT >= :currentDate AND DATE_DEBUT <= :twoDaysLater");

    QDate currentDate = QDate::currentDate();
    QDate twoDaysLater = currentDate.addDays(2);

    // Debug: Print current date and two days later
    qDebug() << "Current Date: " << currentDate.toString("yyyy-MM-dd");
    qDebug() << "Two Days Later: " << twoDaysLater.toString("yyyy-MM-dd");

    query.bindValue(":currentDate", QVariant(currentDate));
    query.bindValue(":twoDaysLater", QVariant(twoDaysLater));

    if (query.exec()) {
        bool eventFound = false;
        QString allMessages;

        while (query.next()) {
            QString eventName = query.value("NOM").toString();
            QString eventDate = query.value("DATE_DEBUT").toString();
            QString message = "• L'événement '" + eventName + "' arrive le " + eventDate + " !\n";
            allMessages += message;
            eventFound = true;
        }

        if (!eventFound) {
            allMessages = "Aucun événement à venir dans les 2 prochains jours.";
        }

        showNotification(allMessages);

    } else {
        // Show detailed error message from the database query
        QString error = query.lastError().text();
        QMessageBox::critical(this, "Erreur de la requête", "Erreur lors de l'exécution de la requête: " + error);
    }
}



void MainWindow::showNotification(const QString &message)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Notification");
    dialog->setStyleSheet(R"(
        QDialog {
            background-color: #f9f9f9;
            border-radius: 15px;
        }
        QLabel {
            font-size: 14px;
            color: #3e5879;
        }
        QPushButton {
            background-color: #3e5879;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 6px 14px;
        }
        QPushButton:hover {
            background-color: #5c759a;
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QLabel *label = new QLabel("Voici les événements à venir :", dialog);
    layout->addWidget(label);

    QStringList lines = message.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QLabel *msg = new QLabel(line, dialog);
        layout->addWidget(msg);
    }

    QPushButton *okBtn = new QPushButton("OK");
    layout->addWidget(okBtn);
    connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    dialog->exec(); // Affiche le popup
}
