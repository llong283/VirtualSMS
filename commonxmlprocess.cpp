/************************************************
����: CommonXmlProcess
����: ����xml�ļ��Ļ��࣬��MsgBox��PhoneBook�̳�
˵��: 1.	ͨ��Dom���ļ����в�����
     2.	Ϊÿһ��һ����ǩ����һ��Ψһ��id���ṩ���ϲ㣬ͬʱά��id-һ����ǩ���������Ա���ٶ�λ��
     3.	����Ԫ�صĲ�������Ϊ��id�����ṩ���ϲ㣻
     4.	Ϊ�ϲ��ṩͨ�õ���ӡ�ɾ�����޸ķ�����
************************************************/

#include "commonxmlprocess.h"

CommonXmlProcess::CommonXmlProcess(QObject *parent) :
    QObject(parent),
    m_isInit(false)
{
}

/************************************************
���ܣ�
    ��ʼ�����ļ��������ȡ���������򴴽�
������
    fileName: �ļ���
    rootTag: ���ڵ���
    levelOneTag: ��һ����ǩ��
    levelTwoTags: �ڶ�����ǩ����
����ֵ:
    ��ʼ���ɹ�����true, ����false
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
            //����
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
        //�������ļ�
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
���ܣ�
    ���Ԫ��
������
    values: ����Ҫ��ӵ�Ԫ�صı�ǩ�ͱ�ǩ�������ı�
����ֵ:
    ��ӳɹ�������id, ʧ���򷵻�-1.
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
���ܣ�
    ����һ��һ����ǩ��values�����������ǩֵ����Ϊ�²����
    һ����ǩ����һ��Ψһ����id
������
    values: ����Ҫ��ӵ�Ԫ�صı�ǩ�ͱ�ǩ�������ı�
    id: ��idΪ-1������뵽��ǰ��������뵽��id��ӦԪ��֮��
����ֵ:
    ����ɹ�������id, ʧ���򷵻�-1.
************************************************/
int CommonXmlProcess::insertElement(const QMap<QString, QString> &values, const int &id)
{
    if (!m_isInit) {
        qDebug() << "init first!" << __FILE__ << __LINE__;
        return -1;
    }
    QList<QString> valueKeys = values.keys();
    //��֤values�еļ�ֵ�͵ڶ�����ǩֵ��Ӧ
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
���ܣ�
    �޸�Ԫ��
������
    values: ����Ҫ�޸ĵ�Ԫ�صı�ǩ���䱻�޸ĺ���ı�
    id: Ҫ�޸ĵ�Ԫ�ض�Ӧ��id
����ֵ:
    �޸ĳɹ�����true, ����false
************************************************/
bool CommonXmlProcess::modifyElement(const int &id, const QMap<QString, QString> &values)
{
    if (!m_isInit) {
        qDebug() << "init first!" << __FILE__ << __LINE__;
        return false;
    }
    //��֤values�еļ�ֵ�����ڵڶ�����ǩ
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
���ܣ�
    �Ƴ�Ԫ��
������
    id: Ҫ�Ƴ���Ԫ�ض�Ӧ��id
����ֵ:
    �Ƴ��ɹ�����true, ����false
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
���ܣ�
    ����
������
    ��
����ֵ:
    ����ɹ�����true, ����false
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
���ܣ�
    ��ȡԪ���������ֵ
������
    element: Ҫ��ȡ��Ԫ��
����ֵ:
    ����Ԫ�ص�ֵ�ļ���
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
���ܣ�
    ��ȡָ��id��Ԫ�ص�ֵ
������
    id: Ԫ�ص�id
����ֵ:
    ����Ԫ�ص�ֵ�ļ���
************************************************/
QMap<QString, QString> CommonXmlProcess::getValues(const int &id)
{
    QDomElement element = m_idToElement.value(id);
    return getElementValues(element);
}

/************************************************
���ܣ�
    ����xml���Ԫ�أ�Ϊÿ��һ����ǩ��������id��������m_idIndex
������
    ��
����ֵ:
    ��
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
