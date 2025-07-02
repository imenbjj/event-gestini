// HistoryDialog.h
#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    void setHistory(const QStringList &history);

private:
    QVBoxLayout *layout;
    QLabel *titleLabel;
    QListWidget *historyListWidget;
};

#endif // HISTORYDIALOG_H
