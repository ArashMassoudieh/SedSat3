#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <QStringList>
#include <qjsonobject.h>
#include <qtablewidget.h>
#include "parameter.h"

class QFile; 

using namespace std;

enum class options_key {x_column_mode};


struct options
{
    bool single_column_x = false;
};

class Interface
{
public:
    Interface();
    Interface(const Interface &intf);
    Interface& operator=(const Interface &intf);
    virtual string ToString();
    virtual QJsonObject toJsonObject();
    virtual bool ReadFromJsonObject(const QJsonObject &jsonobject);
    virtual bool writetofile(QFile* file);
    virtual bool Read(const QStringList &strlist);
    virtual QTableWidget *ToTable();
    string GetNotes() const {
        return Notes;
    }
    void SetNotes(const string &note) {Notes = note;}
    void AppendtoNotes(const string &note)
    {
        if (Notes == "")
            Notes += note;
        else
            Notes += "; " + note;
    }
    void ClearNotes() {Notes = "";}
    double lowlimit,highlimit;
    bool highlightoutsideoflimit = false;
    void SetLimit(_range lowhigh, const double &value)
    {
        if (lowhigh == _range::high)
            highlimit = value;
        else
            lowlimit = value;
        highlightoutsideoflimit = true;
    }
    void SetOption(options_key opt, bool val)
    {
        if (opt == options_key::x_column_mode)
            Options.single_column_x = val;
    }
private:
    string Notes;
    options Options;


};

#endif // INTERFACE_H
