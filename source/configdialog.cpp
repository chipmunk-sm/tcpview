/* This file is part of "TcpView For Linux" - network connections viewer for Linux
 * Copyright (C) 2021 chipmunk-sm <dannico@linuxmail.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "configdialog.h"
#include "ccfontsize.h"

#include <QColorDialog>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QToolTip>
#include <QWheelEvent>
#include <QScrollArea>
#include <QGroupBox>
#include <QCoreApplication>
#include <QGestureEvent>
#include <QScrollBar>
#include <QScroller>
#include <QAbstractItemView>

#define PROP_CLRFOREGROUND "clrForeground"
#define PROP_CLRBACKGROUND "clrBackground"
#define PROP_CLRLABEL "clrLabel"

ConfigDialog::ConfigDialog(const std::function<void()> &callbackUpdate, const std::function<void()> &callbackClose, QWidget *parent)
    : QDialog(parent) {

    m_callbackUpdate = std::move(callbackUpdate);
    m_callbackClose = std::move(callbackClose);

    CCFontSize::changeFontSize(0.0, this);

    setWindowTitle(tr("Settings"));
    setSizeGripEnabled(true);

    auto verticalLayout = new QVBoxLayout(this);

    // *** LABEL ABOUT
    {
        auto label = new QLabel(this);
        label->setOpenExternalLinks(true);
        label->setText(tr("<html>\n"
                          "<head/>\n"
                          "<body>\n"
                          "Copyright (C) chipmunk-sm<br>\n"
                          "<a href=\"https://github.com/chipmunk-sm/\">https://github.com/chipmunk-sm/</a>\n"
                          "</body>\n"
                          "</html>"));
        verticalLayout->addWidget(label);
    }

    // *** QScrollArea for Color Group Box
    {
        m_scrollArea_color = new QScrollArea(this);
        m_scrollArea_color->setWidgetResizable(true);
        m_scrollArea_color->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);

        verticalLayout->addWidget(m_scrollArea_color);

        auto verticalLayout_2 = new QVBoxLayout(m_scrollArea_color);
        auto groupBox = new QGroupBox(m_scrollArea_color);
        groupBox->setTitle(tr("Color"));

        auto gridLayout_color = new QGridLayout(groupBox);
        verticalLayout_2->addWidget(groupBox);

        m_scrollArea_color->setWidget(groupBox);
        m_layout_color = gridLayout_color;
    }

    // *** Color Group Box Items
    int ind = 0;
    for (const auto &item : m_ConnectionStateHelper.getArray()) {

        if (item.first == 0)
            continue;

        auto colorLabel = new QLabel(item.second.name);
        colorLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        m_layout_color->addWidget(colorLabel, ind, 0);
        colorLabel->setProperty(PROP_CLRLABEL, QVariant(item.first));
        setLabelColor(item.second.foreground, item.second.background, colorLabel);

        auto colorFButton = new QPushButton(tr("Foreground"));
        m_layout_color->addWidget(colorFButton, ind, 1);
        connect(colorFButton, &QPushButton::clicked, this, &ConfigDialog::onClick);
        colorFButton->setProperty(PROP_CLRFOREGROUND, QVariant(item.first));

        auto colorBButton = new QPushButton(tr("Background"));
        m_layout_color->addWidget(colorBButton, ind, 2);
        connect(colorBButton, &QPushButton::clicked, this, &ConfigDialog::onClick);
        colorBButton->setProperty(PROP_CLRBACKGROUND, QVariant(item.first));

        ind++;
    }

    // Reset
    {
        auto resetButton = new QPushButton(tr("Reset color"));
        m_layout_color->addWidget(resetButton, ind, 0, 1, 3);
        connect(resetButton, &QPushButton::clicked, this, &ConfigDialog::onReset);
    }
    // separator
    {
        ind++;
        auto line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        m_layout_color->addWidget(line, ind, 0, 1, 3);
    }
    // Color BW
    {
        ind++;
        auto wbColorButton = new QPushButton(tr("Color - B && W"));
        m_layout_color->addWidget(wbColorButton, ind, 0, 1, 3);
        connect(wbColorButton, &QPushButton::clicked, this, &ConfigDialog::onWbColor);
    }
    // Color WB
    {
        ind++;
        auto bwColorButton = new QPushButton(tr("Color - W && B"));
        m_layout_color->addWidget(bwColorButton, ind, 0, 1, 3);
        connect(bwColorButton, &QPushButton::clicked, this, &ConfigDialog::onBwColor);
    }
    // separator
    {
        ind++;
        auto line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        m_layout_color->addWidget(line, ind, 0, 1, 3);
    }
    // Color viewport background
    {
        ind++;
        auto backgroundButton = new QPushButton(tr("Table - set background color"));
        m_layout_color->addWidget(backgroundButton, ind, 0, 1, 3);
        connect(backgroundButton, &QPushButton::clicked, this, &ConfigDialog::onBackgroundColor);
    }
    // Color Reset viewport background
    {
        ind++;
        auto backgroundButton = new QPushButton(tr("Table - set default background"));
        m_layout_color->addWidget(backgroundButton, ind, 0, 1, 3);
        connect(backgroundButton, &QPushButton::clicked, this, &ConfigDialog::onDefaultBackgroundColor);
    }
    // separator
    {
        ind++;
        auto line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        m_layout_color->addWidget(line, ind, 0, 1, 3);
    }


    auto pObj1 = this;
    pObj1->grabGesture(Qt::PinchGesture);
    //pObj1->grabGesture(Qt::PanGesture);
    //pObj1->grabGesture(Qt::SwipeGesture);

    auto pObj = m_scrollArea_color;
    //setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    //setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QScroller::grabGesture(pObj, QScroller::LeftMouseButtonGesture);

    QScrollerProperties properties = QScroller::scroller(pObj)->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,
                               QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff));
    properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,
                               QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff));
    QScroller::scroller(pObj)->setScrollerProperties(properties);

#if ((QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 9) || QT_VERSION_MAJOR >= 6)

#endif

    readGeometry();
}

ConfigDialog::~ConfigDialog() {
    m_callbackClose();
   // delete m_ui;
}

void ConfigDialog::wheelEvent(QWheelEvent *event) {
    auto bControl = (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier;
    if (bControl) {
        const auto numPixels = event->pixelDelta();
        const auto numDegrees = event->angleDelta();

        if (!numPixels.isNull()) {
            auto fontSize = CCFontSize::changeFontSize((numPixels.y() < 0.0) ? -1 : 1, this);
            tooltipText(QString("<center><b>Font %1</b></center>").arg(static_cast<int>(fontSize)));
        } else if (!numDegrees.isNull()) {
            auto fontSize = CCFontSize::changeFontSize((numDegrees.y() < 0.0) ? -1 : 1, this);
            tooltipText(QString("<center><b>Font %1</b></center>").arg(static_cast<int>(fontSize)));
        }
        m_callbackUpdate();
        event->accept();
        return;
    }
    QDialog::wheelEvent(event);
}

bool ConfigDialog::event(QEvent *event) {
    if (event->type() == QEvent::Gesture) {
        auto pGestureEvent = dynamic_cast<QGestureEvent *>(event);
        if (pGestureEvent) {
            auto pPinchGesture = pGestureEvent->gesture(Qt::PinchGesture);
            if (pPinchGesture) {
                return gestureEventPinch(static_cast<QPinchGesture *>(pPinchGesture));
            }
        }
    }
    return QWidget::event(event);
}

bool ConfigDialog::gestureEventPinch(QPinchGesture *gesture) {

    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if ((changeFlags & QPinchGesture::ScaleFactorChanged) == QPinchGesture::ScaleFactorChanged) {

        double scale = 0.0;
        double scaleFactor = gesture->scaleFactor() - 1.0;

        if (scaleFactor >= 0.01)
            scale = 1;
        else if (scaleFactor <= -0.01)
            scale = -1;

        if (std::abs(scale) > 0.01) {
            auto fontSize = CCFontSize::changeFontSize(scale, this);
            tooltipText(QString("<center><b>Font %1</b></center>").arg(static_cast<int>(fontSize)));
        }
    }
    return true;
}

void ConfigDialog::tooltipText(const QString &text) {
    if (width() < 1 || height() < 1)
        return;

    const auto toolTipText = QString("<center><b>%1</b></center>").arg(text);
    auto fm = fontMetrics();
    auto boundingRect = fm.boundingRect(QRect(0, 0, width(), height()), Qt::TextWordWrap, toolTipText);
    boundingRect.setWidth(boundingRect.width());
    boundingRect.setHeight(boundingRect.height());

    auto xPos = width() - boundingRect.width() / 2;
    auto yPos = height() - boundingRect.height() / 2;

    if (xPos < 1 || yPos < 1)
        return;

    xPos /= 2;
    yPos /= 2;

    QToolTip::showText(this->mapToGlobal(QPoint(xPos, yPos)), toolTipText, this);
}

void ConfigDialog::reloadColor()
{
    QLabel *pLabel = nullptr;
    for (int idx = 0; idx < m_layout_color->count(); idx++) {
        auto item = m_layout_color->itemAt(idx)->widget();
        auto propLab = item->property(PROP_CLRLABEL);
        if (propLab.isValid()) {
            pLabel = qobject_cast<QLabel *>(item);
            if (pLabel) {
                auto id = static_cast<eConnectionTcpState>(propLab.toInt());
                setLabelColor(m_ConnectionStateHelper.getStateColor(id, true), m_ConnectionStateHelper.getStateColor(id, false), pLabel);
            }
        }
    }
}

void ConfigDialog::onClick() {
    auto button = qobject_cast<QPushButton *>(QObject::sender());
    if (!button)
        return;

    auto propFr = button->property(PROP_CLRFOREGROUND);
    auto propBk = button->property(PROP_CLRBACKGROUND);
    auto propVal = propFr.isValid() ? propFr : propBk;

    if (!propVal.isValid())
        return;

    QLabel *pLabel = nullptr;
    for (int idx = 0; idx < m_layout_color->count(); idx++) {
        auto item = m_layout_color->itemAt(idx)->widget();
        auto propLab = item->property(PROP_CLRLABEL);
        if (propLab.isValid()) {
            if (propLab == propVal) {
                pLabel = qobject_cast<QLabel *>(item);
                if (pLabel)
                    break;
            }
        }
    }

    if (!pLabel)
        return;

    auto color = QColorDialog::getColor(m_ConnectionStateHelper.getStateColor(static_cast<eConnectionTcpState>(propVal.toUInt()), propFr.isValid()), this);
    if (color.isValid()) {
        auto id = static_cast<eConnectionTcpState>(propVal.toUInt());
        m_ConnectionStateHelper.saveColor(id, color, propFr.isValid());
        setLabelColor(m_ConnectionStateHelper.getStateColor(id, true), m_ConnectionStateHelper.getStateColor(id, false), pLabel);
        m_callbackUpdate();
    }
}

void ConfigDialog::onReset() {
    m_ConnectionStateHelper.updatetColor(true);
    reloadColor();
    //m_callbackUpdate();
    onDefaultBackgroundColor();
}

void ConfigDialog::onBwColor()
{
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("tree/color", "#000000");
    }
    m_ConnectionStateHelper.setBwColor();
    reloadColor();
    m_callbackUpdate();
}

void ConfigDialog::onWbColor()
{
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("tree/color", "#FFFFFF");
    }
    m_ConnectionStateHelper.setWbColor();
    reloadColor();
    m_callbackUpdate();
}

void ConfigDialog::onBackgroundColor() {
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        auto color = QColorDialog::getColor(QColor(settings.value("tree/color").toString()), this);
        if (color.isValid()) {
            settings.setValue("tree/color", color.name());
        }
    }
    m_callbackUpdate();
}

void ConfigDialog::onDefaultBackgroundColor()
{
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue("tree/color", "");
    }
    m_callbackUpdate();
}

void ConfigDialog::setLabelColor(const QColor &frg, const QColor &bkg, QLabel *pLabel) {
    const QString clrStyle("QLabel { color: rgb(%1, %2, %3); background-color: rgb(%4, %5, %6); }");
    pLabel->setStyleSheet(clrStyle
                              .arg(frg.red())
                              .arg(frg.green())
                              .arg(frg.blue())
                              .arg(bkg.red())
                              .arg(bkg.green())
                              .arg(bkg.blue()));
}

void ConfigDialog::readGeometry()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    restoreGeometry(settings.value("settings/geometry").toByteArray());
}

void ConfigDialog::closeEvent(QCloseEvent *)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("settings/geometry", saveGeometry());
}

void ConfigDialog::keyPressEvent(QKeyEvent *event)
{
    if (m_scrollArea_color->verticalScrollBar()->isEnabled() && (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
    {
        m_scrollArea_color->verticalScrollBar()->setEnabled(false);
        toolTip();
    }
    QWidget::keyPressEvent(event);
}

void ConfigDialog::keyReleaseEvent(QKeyEvent *event)
{
    if(!m_scrollArea_color->verticalScrollBar()->isEnabled() && (event->modifiers() & Qt::ControlModifier) != Qt::ControlModifier)
    {
        m_scrollArea_color->verticalScrollBar()->setEnabled(true);
        QToolTip::hideText();
    }
    QWidget::keyReleaseEvent(event);
}
