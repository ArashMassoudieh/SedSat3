#ifndef SELECTSAMPLES_H
#define SELECTSAMPLES_H

#include <QWidget>
#include "sourcesinkdata.h"
#include "selectsampledelegate.h"


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
    void SetMode(mode _mode) {Mode=_mode;}
private:
    Ui::SelectSamples *ui;
    SourceSinkData *data;
    mode Mode;
public slots:
    void comboChanged();
};

#endif // SELECTSAMPLES_H
