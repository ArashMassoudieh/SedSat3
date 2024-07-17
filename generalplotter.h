#pragma once

#include <QPoint>
#include "qcustomplot.h"
#include "BTCSet.h"

class QRubberBand;
class QMouseEvent;
class QWidget;
#include "vector"

//#ifdef _WINDOWS
//#include "drand.h"
//#endif

enum class AxisScale {normal, log};
enum class Axis {x, y};

using namespace std;

class GeneralPlotter : public QCustomPlot
{


public:
    GeneralPlotter(QWidget * parent = 0);
    virtual ~GeneralPlotter();
    void Clear();
    bool AddScatter(const string &name, const vector<double> &x, const vector<double> &y, const QCPScatterStyle &symbol = QCPScatterStyle::ssDisc);
    bool AddScatter(const string &name, const vector<string> &x, const vector<double> &y, const QCPScatterStyle &symbol = QCPScatterStyle::ssDisc);
    bool AddScatters(const vector<string> names, const vector<vector<double>> &x,const vector<vector<double>> &y);
    bool AddScatters(const vector<string> names, const vector<string> &x,const vector<vector<double>> &y);
    bool AddNoneUniformScatter(const map<string,vector<double>> &data, int shape_counter=0);
    bool AddTimeSeries(const string &name, const vector<double> &x, const vector<double> &y);
    bool AddTimeSeriesSet(const string &name, const vector<vector<double>> &x, const vector<vector<double>> &y);
    bool SetYAxisScaleType(AxisScale axisscale);
    void setZoomMode(bool mode);
    void ZoomExtends();
    void SetRange(const vector<double> &range, const Axis &whichaxis);
    void SetLegend(bool onoff);



private:
    vector<double> x_max_min_range;
    vector<double> y_max_min_range;
    static QVector<QCPScatterStyle::ScatterShape> shapes;

private slots:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent * event);
    void mouseReleaseEvent(QMouseEvent * event) override;
    void axisDoubleClick(QCPAxis *  axis, QCPAxis::SelectablePart  part, QMouseEvent *  event);
private:
    bool mZoomMode;
    QRubberBand * mRubberBand;
    QPoint mOrigin;
};

QVector<QCPScatterStyle::ScatterShape> Make_Shapes();

template <class T>
static inline QVector<T> QVectorfromStdVector(const std::vector<T> &vector)
{
   return QVector<T>(vector.begin(), vector.end());
}

template <class T>
static inline QList<T> QListfromStdVector(const std::vector<T> &vector)
{
   return QList<T>(vector.begin(), vector.end());
}

template <class T>
static inline QVector<T> fromStdVector(const std::vector<T> &vector);

