/****************************************************************************
****************************************************************************/
#ifndef RECORDSPAGE_H
#define RECORDSPAGE_H

#include <QDialog>
#include <QStandardItemModel>

class QFile;
class QLabel;
class QLineEdit;

namespace Ui {
    class RecordsPage;
}

class RecordsPage : public QDialog
{
    Q_OBJECT

  public:
    explicit RecordsPage(QWidget *parent);
    ~RecordsPage();


  private Q_SLOTS:

  private:
    Ui::RecordsPage *ui;
    QStandardItemModel *csvModel;
    QFile *file;
};

#endif // RECORDSPAGE_H
