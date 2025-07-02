// HistoryDialog.cpp
#include "HistoryDialog.h"
#include <QPushButton>

HistoryDialog::HistoryDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Event History");
    setFixedSize(400, 300);

    layout = new QVBoxLayout(this);
    titleLabel = new QLabel("Event History Details:", this);
    historyListWidget = new QListWidget(this);

    layout->addWidget(titleLabel);
    layout->addWidget(historyListWidget);

    QPushButton *closeButton = new QPushButton("Close", this);
    layout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &HistoryDialog::accept);
}

void HistoryDialog::setHistory(const QStringList &history)
{
    historyListWidget->clear();
    historyListWidget->addItems(history);
}
