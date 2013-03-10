#ifndef COMMONXMLPROCESS_H
#define COMMONXMLPROCESS_H

#include <QObject>
#include "config.h"

class CommonXmlProcess : public QObject
{
    Q_OBJECT
public:
    CommonXmlProcess(QObject *parent = 0);
    virtual bool save();
    
protected:
    bool initCommon(const QString &fileName, const QString &rootTag, const QString &levelOneTag, const QVector<QString> &levelTwoTags);
    void parseXml();    
    int appendElement(const QMap<QString, QString> &values);
    int insertElement(const QMap<QString, QString> &values, const int &id);
    bool removeElement(const int &id);
    bool modifyElement(const int &id, const QMap<QString, QString> &values);
    QMap<QString, QString> getValues(const int &id);
    inline QList<int> idIndex()
    {
        return m_idIndex;
    }
    inline QString fileName()
    {
        return m_fileName;
    }
    inline QString rootTag()
    {
        return m_rootTag;
    }
    inline QString levelOneTag()
    {
        return m_levelOneTag;
    }
    inline QVector<QString> levelTwoTags()
    {
        return m_levelTwoTags;
    }
    
private:
    QString m_fileName;
    QString m_rootTag;
    QString m_levelOneTag;
    QVector<QString> m_levelTwoTags;
    bool m_isInit;
    QDomDocument m_doc;
    QMap<int, QDomElement> m_idToElement;     //Ó³Éä£ºid-ÔªËØ
    QList<int> m_idIndex;   //id¼¯ºÏ
    
    bool createFile();
    QMap<QString, QString> getElementValues(const QDomElement &element);
    
signals:

public slots:

};

#endif // COMMONXMLPROCESS_H
