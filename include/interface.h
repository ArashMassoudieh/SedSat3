#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <QStringList>
#include <qjsonobject.h>
#include <qtablewidget.h>

class QFile; 

using namespace std;

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
private:
    string Notes;

};

#endif // INTERFACE_H
