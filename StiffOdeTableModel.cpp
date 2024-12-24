#include "StiffOdeTableModel.hpp"

StiffOde::StiffOdeTableModel::StiffOdeTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void StiffOde::StiffOdeTableModel::setData(const std::vector<QPointF>& numericalSeries0,
                                           const std::vector<QPointF>& numericalSeries1,
                                           const std::vector<QPointF>& exactSolution0,
                                           const std::vector<QPointF>& exactSolution1,
                                           const std::vector<QPointF>& error0,
                                           const std::vector<QPointF>& error1)
{
    beginResetModel();
    m_numericalSeries0 = numericalSeries0;
    m_numericalSeries1 = numericalSeries1;
    m_exactSolution0 = exactSolution0;
    m_exactSolution1 = exactSolution1;
    m_error0 = error0;
    m_error1 = error1;
    endResetModel();
}

int StiffOde::StiffOdeTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_numericalSeries0.size());
}

int StiffOde::StiffOdeTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    // "n", "x_n", "u1_exact", "u2_exact", "u1_numerical", "u2_numerical", "E1", "E2"
    return 8;
}

QVariant StiffOde::StiffOdeTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.column() >= columnCount())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        int row = index.row();
        int col = index.column();

        switch (col)
        {
        case 0: // n
            return row;
        case 1: // x_n
            return m_numericalSeries0[row].x();
        case 2: // u1_exact
            return m_exactSolution0[row].y();
        case 3: // u2_exact
            return m_exactSolution1[row].y();
        case 4: // u1_numerical
            return m_numericalSeries0[row].y();
        case 5: // u2_numerical
            return m_numericalSeries1[row].y();
        case 6: // E1
            return m_error0[row].y();
        case 7: // E2
            return m_error1[row].y();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant StiffOde::StiffOdeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return QString("n");
        case 1:
            return QString("x_n");
        case 2:
            return QString("u(1) (точное)");
        case 3:
            return QString("u(2) (точное)");
        case 4:
            return QString("u(1) (численное)");
        case 5:
            return QString("u(2) (численное)");
        case 6:
            return QString("E (погрешность 1)");
        case 7:
            return QString("E (погрешность 2)");
        default:
            return QVariant();
        }
    }

    return QVariant();
}
