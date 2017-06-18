#ifndef TABLEHEADERCAPTION_H
#define TABLEHEADERCAPTION_H

#include <QWidget>

class TableHeaderCaption
{
public:
    TableHeaderCaption(const QString &caption, const QString &captionSample, const QString &captionToolTip)
    {
        m_caption = caption;
        m_captionSample = captionSample;
        m_captionToolTip = captionToolTip;
    }

    QString caption() const { return m_caption; }
    QString captionSample() const { return m_captionSample; }
    QString captionToolTip() const { return m_captionToolTip; }

  private:
    QString m_caption;
    QString m_captionSample;
    QString m_captionToolTip;

};

#endif // TABLEHEADERCAPTION_H
