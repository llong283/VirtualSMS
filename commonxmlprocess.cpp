/************************************************
类名: CommonXmlProcess
功能: 操作xml文件的基类，被MsgBox、PhoneBook继承
说明: 1.	通过Dom对文件进行操作；
     2.	为每一个一级标签虚拟一个唯一的id，提供给上层，同时维护id-一级标签的索引，以便快速定位；
     3.	将对元素的操作抽象为对id操作提供给上层；
     4.	为上层提供通用的添加、删除、修改方法。
************************************************/

#include "commonxmlprocess.h"

CommonXmlProcess::CommonXmlProcess(QObject *parent) :
    QObject(parent),
    m_isInit(false)
{
}

/************************************************
功能：
    初始化，文件存在则读取，不存在则创建
参数：
    fileName: 文件名
    rootTag: 根节点名
    levelOneTag: 第一级标签名
    levelTwoTags: 第二级标签集合
返回值:
    初始化成功返回true, 否则false
************************************************/
bool CommonXmlProcess::initCommon(const QString &fileName, 
                                  const QString &rootTag, 
                                  const QString &levelOneTag, 
                                  const QVector<QString> &levelTwoTags)
{    
    m_fileName = fileName;
    m_rootTag = rootTag;
    m_levelOneTag = levelOneTag;
    m_levelTwoTags = levelTwoTags;
    QFileInfo fileInfo(m_fileName);
    QDir dir(fileInfo.absoluteDir());
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }
    bool isSuccess = false;
    QString baseName = fileInfo.completeBaseName();
    if (fileInfo.exists() && fileInfo.size()!=0) {
        QString errorStr;
        int errorLine;
        int errorColumn;
        
        QFile file(m_fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qDebug() << file.errorString() << __FILE__ << __LINE__;
            return false;
        }
        isSuccess = m_doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn);
        if (isSuccess) {
            QDomElement root = m_doc.documentElement();
            if (root.tagName() != m_rootTag) {
                qDebug() << QString("Error: Not a %1 file. FileName:%2").arg(m_rootTag).arg(m_fileName)
                         << __FILE__ << __LINE__;
                QMessageBox::information(NULL, tr("msg"), 
                                         tr("Error: Not a %1 file. FileName:%2")
                                         .arg(m_rootTag).arg(m_fileName));
                isSuccess = false;
            }            
        } else {
            QMessageBox::information(NULL, tr("msg"), tr("Error: Parse error at line %1, column %2.\nError info:%3")
                                     .arg(errorLine).arg(errorColumn).arg(errorStr));
        }
        file.close();
        if (!isSuccess) {
            //备份
            QString dir = fileInfo.absoluteDir().absolutePath();
            QString suffix = fileInfo.suffix();
            QString newFileName;    
            int i = 0;
            baseName += '_';
            while (true) {
                newFileName = QString("%1/%2.%3").arg(dir).arg(baseName + QString::number(i)).arg(suffix);
                if (!QFile::exists(newFileName)) {
                    break;
                }
                ++i;
            }
            baseName += QString::number(i);            
            qDebug() << QFile::copy(m_fileName, newFileName) << newFileName << __FILE__ << __LINE__;
        }
    }
    if (!isSuccess) {
        //创建新文件
        if (!createFile()) {
            QMessageBox::information(NULL, tr("msg"), tr("Create file failed. Please check if you have permission"));
            return false;
        }
        if (fileInfo.exists()) {
            QMessageBox::information(NULL, tr("msg"), tr("Create new file. Original file is backed up:\n%1/%2.xml")
                                     .arg(fileInfo.absoluteDir().absolutePath()).arg(baseName));
        }
    }
    parseXml();
    m_isInit = true;
    return true;    
}

bool CommonXmlProcess::createFile()
{
    QFile file(m_fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
        return false;
    }
    m_doc.clear();
    QDomNode newNode;
    QDomElement newElement;
    newNode = m_doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    m_doc.appendChild(newNode);
    newElement = m_doc.createElement(m_rootTag);              
    newNode = m_doc.appendChild(newElement);
    QTextStream out(&file);
    m_doc.save(out, 4);
    file.close();
    return true;
}

/************************************************
功能：
    添加元素
参数：
    values: 包含要添加的元素的标签和标签包含的文本
返回值:
    添加成功返回新id, 失败则返回-1.
************************************************/
int CommonXmlProcess::appendElement(const QMap<QString, QString> &values)
{
    if (m_idIndex.isEmpty()) {
        return insertElement(values, -1);
    } else {
        QList<int>::iterator iter = m_idIndex.end();
        iter--;
        return insertElement(values, *iter);
    }
}

/************************************************
功能：
    插入一个一级标签，values里包含二级标签值，并为新插入的
    一级标签生成一个唯一的新id
参数：
    values: 包含要添加的元素的标签和标签包含的文本
    id: 如id为-1，则插入到最前；否则插入到该id对应元素之后
返回值:
    插入成功返回新id, 失败则返回-1.
************************************************/
int CommonXmlProcess::insertElement(const QMap<QString, QString> &values, const int &id)
{
    if (!m_isInit) {
        qDebug() << "init first!" << __FILE__ << __LINE__;
        return -1;
    }
    QList<QString> valueKeys = values.keys();
    //保证values中的键值和第二级标签值对应
    if (valueKeys.size() != m_levelTwoTags.size()) {
        qDebug() << "The keys of values don't correspond with levelTwoTags" << __FILE__ << __LINE__;
        qDebug() << "keys:" << valueKeys << "levelTwoTags:" << m_levelTwoTags << __FILE__ << __LINE__;
        return -1;
    } else {
        foreach (QString key, valueKeys) {
            if (m_levelTwoTags.indexOf(key) == -1) {
                qDebug() << "The keys of values don't correspond with levelTwoTags" << __FILE__ << __LINE__;
                qDebug() << "keys:" << valueKeys << "levelTwoTags:" << m_levelTwoTags << __FILE__ << __LINE__;
                return -1;
            }
        }
    }
    
    QDomElement root = m_doc.documentElement();
    int newId;
    if (m_idToElement.isEmpty()) {
        newId = 1;
    } else {
        QMap<int, QDomElement>::iterator iter = m_idToElement.end();
        iter--;
        newId = iter.key() + 1;
    }
    QDomElement newFirstElement = m_doc.createElement(m_levelOneTag);
    QDomElement newSecondElement;
    QDomText newDomText;
    foreach (QString ltTag, m_levelTwoTags) {
        newSecondElement = m_doc.createElement(ltTag);
        newDomText = m_doc.createTextNode(values.value(ltTag));
        newSecondElement.appendChild(newDomText);
        newFirstElement.appendChild(newSecondElement);                
    }
    if (id == -1) {
        QDomElement element = root.firstChildElement(m_levelOneTag);
        root.insertBefore(newFirstElement, element);
        m_idIndex.prepend(newId);
    } else {
        QDomElement element = m_idToElement.value(id);
        if (element.isNull()) {
            qDebug() << "this is a bug" << id << __FILE__ << __LINE__;
        }
        root.insertAfter(newFirstElement, element);
        if (id == m_idIndex.last()) {
            m_idIndex.append(newId);
        } else {
            m_idIndex.insert(m_idIndex.indexOf(id) + 1, newId);
        }
    }
    if (m_idToElement.contains(newId)) {
        qDebug() << "this is a bug!" << __FILE__ << __LINE__;        
    }
    m_idToElement.insert(newId, newFirstElement);
    return newId;
}

/************************************************
功能：
    修改元素
参数：
    values: 包含要修改的元素的标签及其被修改后的文本
    id: 要修改的元素对应的id
返回值:
    修改成功返回true, 否则false
************************************************/
bool CommonXmlProcess::modifyElement(const int &id, const QMap<QString, QString> &values)
{
    if (!m_isInit) {
        qDebug() << "init first!" << __FILE__ << __LINE__;
        return false;
    }
    //保证values中的键值都属于第二级标签
    QList<QString> valueKeys = values.keys();
    foreach (QString key, valueKeys) {
        if (m_levelTwoTags.indexOf(key) == -1) {
            qDebug() << "The key doesn't belong to levelTwoTags. key:" << key  << __FILE__ << __LINE__;
            qDebug() << "keys:" << valueKeys << "levelTwoTags:" << m_levelTwoTags << __FILE__ << __LINE__;
            return false;
        }
    }
    
    QDomElement element = m_idToElement.value(id);
    if (element.isNull()) {
        qDebug() << "no such element exist.id:" << id << __FILE__ << __LINE__;
        return false;
    } else {
        QDomElement newElement;
        QDomText newDomText;
        QDomElement child = element.firstChildElement();
        while (!child.isNull()) {
            if (!values.value(child.tagName()).isEmpty() 
                    && child.text() != values.value(child.tagName())) {
                newElement = m_doc.createElement(child.tagName());
                newDomText = m_doc.createTextNode(values.value(child.tagName()));
                newElement.appendChild(newDomText);
                element.replaceChild(newElement, child);
                child = newElement;
            }
            child = child.nextSiblingElement();
        }
        return true;
    }
}

/************************************************
功能：
    移除元素
参数：
    id: 要移除的元素对应的id
返回值:
    移除成功返回true, 否则false
************************************************/
bool CommonXmlProcess::removeElement(const int &id)
{
    if (!m_isInit) {
        qDebug() << "init first!" << __FILE__ << __LINE__;
        return false;
    }
    QDomElement root = m_doc.documentElement();
    QDomElement element = m_idToElement.value(id);
    if (element.isNull()) {
        qDebug() << "no such element. id:" << id << __FILE__ << __LINE__;
        return false;
    }
    if (root.removeChild(element).isNull()) {
        qDebug() << "remove element failed" << __FILE__ << __LINE__;
        return false;
    }
    m_idToElement.remove(id);
    m_idIndex.removeAt(m_idIndex.indexOf(id));
    return true;    
}

/************************************************
功能：
    保存
参数：
    无
返回值:
    保存成功返回true, 否则false
************************************************/
bool CommonXmlProcess::save()
{
    if (!m_isInit) {
        qDebug() << "init first!" << __FILE__ << __LINE__;
        return false;
    }
    QFile file(m_fileName);
    QFileInfo fileInfo(m_fileName);
    QDir dir(fileInfo.absoluteDir());
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
        return false;
    }
    QTextStream out(&file);
    m_doc.save(out, 4);
    file.close();
    return true;    
}

/************************************************
功能：
    获取元素里包含的值
参数：
    element: 要获取的元素
返回值:
    返回元素的值的集合
************************************************/
QMap<QString, QString> CommonXmlProcess::getElementValues(const QDomElement &element)
{
    QDomElement child = element.firstChildElement();
    QMap<QString, QString> values;
    while (!child.isNull()) {
        QString strTagName = child.tagName();
        if (m_levelTwoTags.indexOf(strTagName) == -1) {
            qDebug() << "unknown levelTwoTag:" << strTagName << __FILE__ << __LINE__;
        } else {
            values.insert(child.tagName(), child.text());
        }
        child = child.nextSiblingElement();
    }
    return values;    
}

/************************************************
功能：
    获取指定id的元素的值
参数：
    id: 元素的id
返回值:
    返回元素的值的集合
************************************************/
QMap<QString, QString> CommonXmlProcess::getValues(const int &id)
{
    QDomElement element = m_idToElement.value(id);
    return getElementValues(element);
}

/************************************************
功能：
    解析xml里的元素，为每个一级标签生成虚拟id，并存入m_idIndex
参数：
    无
返回值:
    无
************************************************/
void CommonXmlProcess::parseXml()
{
    QDomElement child = m_doc.documentElement().firstChildElement();
    int id = 1;
    while (!child.isNull()) {
        if (child.tagName() == m_levelOneTag) {
            m_idToElement.insert(id, child);
            m_idIndex.append(id);
            id++;
        } else {
            qDebug() << "Unknown levelOneTag:" << child.tagName() << __FILE__ << __LINE__;
        }
        child = child.nextSiblingElement();
    }
}
