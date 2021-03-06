#ifndef RECORDINGTABLEVIEW_H
#define RECORDINGTABLEVIEW_H

#include <QAbstractTableModel>
#include <QVariant>

class RecordingTableViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RecordingTableViewModel(QObject *parent = nullptr);

    // Header:
    //QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


    //Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QList<QString> m_list;
};

#endif // RECORDINGTABLEVIEW_H
