#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <QStringList>
#include <qjsonobject.h>
#include <qtablewidget.h>
#include "parameter.h"

class QFile; 

using namespace std;

enum class options_key {single_column_x};


struct options
{
    bool single_column_x = false;
    QString X_suffix = "x-value";
    QString Y_suffix = "y-value";

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
        if (opt == options_key::single_column_x)
            Options.single_column_x = val;
    }
    bool Option(options_key opt)
    {
        if (opt == options_key::single_column_x)
            return Options.single_column_x;

        return false;
    }
    options& GetOptions()
    {
        return Options;
    }
private:
    string Notes;
    options Options;


};

#endif // INTERFACE_H
