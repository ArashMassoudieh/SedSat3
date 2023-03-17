#ifndef SELECTSAMPLES_H
#define SELECTSAMPLES_H

#include <QWidget>
#include "sourcesinkdata.h"

namespace Ui {
class SelectSamples;
}

class SelectSamples : public QWidget
{
    Q_OBJECT

public:
    explicit SelectSamples(QWidget *parent = nullptr);
    void SetData(SourceSinkData *_data);
    ~SelectSamples();

private:
    Ui::SelectSamples *ui;
    SourceSinkData *data;

public slots:
    void comboChanged();
};

#endif // SELECTSAMPLES_H
