#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QGroupBox>
#include <QTreeView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStateMachine>
#include <QTextEdit>
#include <QProcess>

#include "tableheadercaption.h"
#include "updatethread.h"

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();
    void InitStringMap();
    void InitControlsText();
    void RestoreAppState();
    void SaveAppState();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void filterRegExpChanged();
    void filterColumnChanged();
    void updateGui();
    void pauseUpdateThread();
    void onCustomContextMenu(const QPoint &point);
    void RunNetstat();
    void SetFontSize(int fontSizeIndex);
    void captureMode();
    void RunRootDatasource();

    void readProcDataStd();
    void readProcDataErr();
signals:
    void callUpdateGui();

private:
    CUpdateThread                                       m_NetData;

    QMap<CDataSource::ColumnData, TableHeaderCaption>   m_TableCaption;
    QMap<CDataSource::eNetType, QString>                m_TableProtocolName;
    QMap<CDataSource::ConnectionTcpState, QString>      m_TableConnectionTcpState;

    QSortFilterProxyModel                               m_ProxyModel;
    QGroupBox                                           m_GroupBox;
    QTreeView                                           m_TreeView;
    QLabel                                              m_filterPatternLabel;
    QLabel                                              m_filterSyntaxLabel;
    QLabel                                              m_filterColumnLabel;
    QLineEdit                                           m_filterPatternLineEdit;
    QComboBox                                           m_filterSyntaxComboBox;
    QComboBox                                           m_filterColumnComboBox;
    QVBoxLayout                                         m_connLayout;
    QWidget                                             m_connWidg;

    QPushButton         m_buttonPause;
    QStateMachine       m_qstatePause;

    QPushButton         m_buttonCaptureMode;
    QStateMachine       m_qstateCaptureMode;
    bool                m_captureEnable;

    QSlider             m_zoomSlider;

    QPushButton         m_buttonRunNetstat;
    int                 m_fontSize;
    QList<int>          m_fontSizeList;

    QPushButton         m_buttonRunRootDatasource;
    QProcess            m_rootResolver;

    void updateTreeView(CDataSource::SocketInfo *dataSource, bool disableCleanup);
    void HideDataColumn();
    void showEvent( QShowEvent* event );
    void ShowWhois(QString rowText, QString whoisText);
    void ShowInfoDialog(QString title, QString dialogText, bool readonly);
};

#endif // WINDOW_H
