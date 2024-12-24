#ifndef STIFFODETABLEMODEL_HPP
#define STIFFODETABLEMODEL_HPP

#include <QAbstractTableModel>
#include <vector>
#include <QPointF>

namespace StiffOde
{
class StiffOdeTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StiffOdeTableModel(QObject* parent = nullptr);

    // Установка данных
    void setData(const std::vector<QPointF>& numericalSeries0,
                 const std::vector<QPointF>& numericalSeries1,
                 const std::vector<QPointF>& exactSolution0,
                 const std::vector<QPointF>& exactSolution1,
                 const std::vector<QPointF>& error0,
                 const std::vector<QPointF>& error1);

    // Реализация виртуальных методов
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    std::vector<QPointF> m_numericalSeries0;
    std::vector<QPointF> m_numericalSeries1;
    std::vector<QPointF> m_exactSolution0;
    std::vector<QPointF> m_exactSolution1;
    std::vector<QPointF> m_error0;
    std::vector<QPointF> m_error1;
};
}

#endif // STIFFODETABLEMODEL_HPP
