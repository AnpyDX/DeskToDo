#include "ListItem.h"
#include "Data.hpp"

#include <QLabel>
#include <QEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPainterPath>
#include <QStackedWidget>


static QPixmap createCirclePixMap(
    const QSize& size, 
    const QColor& borderColor,
    const QColor& fillColor = Qt::transparent)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    const int margin = 2;
    QRectF r(margin, margin, size.width() - 2 * margin, size.height() - 2 * margin);
    if (fillColor.isValid() && fillColor.alpha() > 0) {
        p.setBrush(fillColor);
    }
    p.setPen(QPen(borderColor, 2));
    p.drawEllipse(r);
    p.end();
    return pix;
}

static QPixmap createCheckPixMap(
    const QSize& size, 
    const QColor& borderColor,
    const QColor& fillColor, 
    const QColor& checkColor)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    const int margin = 2;
    QRectF r(margin, margin, size.width() - 2 * margin, size.height() - 2 * margin);
    if (fillColor.isValid() && fillColor.alpha() > 0) {
        p.setBrush(fillColor);
    }
    p.setPen(QPen(borderColor, 2));
    p.drawEllipse(r);
    p.setPen(QPen(checkColor, 2.5, Qt::SolidLine, Qt::PenCapStyle::FlatCap, Qt::RoundJoin));
    QPainterPath path;
    const qreal w = size.width(), h = size.height();
    path.moveTo(w * 0.25, h * 0.45);
    path.lineTo(w * 0.45, h * 0.65);
    path.lineTo(w * 0.70, h * 0.35);
    p.drawPath(path);
    p.end();
    return pix;
}

ListItem::ListItem(QString content, bool completed, QWidget* parent)
: QWidget(parent), mCompleted(completed) {
    auto GlobalConfig = GetConfig();

    setStyleSheet(std::format(
        "background-color: {}; border-radius: {};",
        GlobalConfig.theme.item.background,
        GlobalConfig.theme.item.border_radius
    ).c_str());
    setAutoFillBackground(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 0, 8, 4);

    stack = new QStackedWidget(this);

    /* ============= Display Page Layout ============= */
    auto* displayPage = new QWidget;
    auto* displayLayout = new QHBoxLayout(displayPage);
    displayLayout->setContentsMargins(5, 0, 5, 0);
    displayLayout->setSpacing(8);

    const QSize btnSize(18, 18);
    emptyPixMap = createCirclePixMap(
        btnSize, 
        QColor::fromString(GlobalConfig.theme.item.checker_empty_color_0)
    );
    hoverPixMap = createCheckPixMap(
        btnSize, 
        QColor::fromString(GlobalConfig.theme.item.checker_hover_color_0),
        QColor::fromString(GlobalConfig.theme.item.checker_hover_color_1),
        QColor::fromString(GlobalConfig.theme.item.checker_hover_color_2)
    );
    donePixMap  = createCheckPixMap(
        btnSize, 
        QColor::fromString(GlobalConfig.theme.item.checker_done_color_0),
        QColor::fromString(GlobalConfig.theme.item.checker_done_color_1),
        QColor::fromString(GlobalConfig.theme.item.checker_done_color_2)
    );

    completeButton = new QPushButton;
    completeButton->setFixedSize(btnSize);
    completeButton->setIconSize(btnSize);
    completeButton->setFlat(true);
    completeButton->setStyleSheet("border: none; background: transparent;");
    completeButton->installEventFilter(this);
    completeButton->setIcon(QIcon(emptyPixMap));

    connect(completeButton, &QPushButton::clicked, this, &ListItem::onCompleteClicked);

    displayLayout->addWidget(completeButton);

    label = new QLabel(content);
    label->setWordWrap(true);
    label->setStyleSheet(std::format(
        "font-size: {}; color: {};", 
        GlobalConfig.theme.item.font_size,
        GlobalConfig.theme.item.font_color
    ).c_str());
    displayLayout->addWidget(label, 1);
    stack->addWidget(displayPage);


    /* ============= Editing Page Layout ============= */
    auto* editPage = new QWidget;
    auto* editLayout = new QVBoxLayout(editPage);
    editLayout->setContentsMargins(5, 0, 5, 0);
    lineEdit = new QLineEdit(content);
    editLayout->addWidget(lineEdit);
    stack->addWidget(editPage);

    stack->setCurrentIndex(0);   // Show `this->label` by default 
    mainLayout->addWidget(stack);

    connect(lineEdit, &QLineEdit::editingFinished, this, &ListItem::finishEditing);
    lineEdit->installEventFilter(this);

    // Set item's minimum height
    const int minHeight = 50;
    setMinimumHeight(minHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Apply initial appearance
    updateCompleteAppearance();
}

bool ListItem::isCompleted() const {
    return mCompleted;
}

std::string ListItem::getContent() const {
    return label->text().toStdString();
}

QSize ListItem::sizeHint() const {
    int h = layout()->sizeHint().height();
    h = qMax(h, minimumHeight());
    return QSize(0, h);
}

void ListItem::mouseDoubleClickEvent(QMouseEvent* event) {
    QWidget::mouseDoubleClickEvent(event);
    startEditing();
}

bool ListItem::eventFilter(QObject* object, QEvent* event) {
    // 处于编辑状态时，按下 Esc 取消编辑
    if (object == lineEdit && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            lineEdit->setText(label->text());
            finishEditing();
            return true;
        }
    }

    // 根据鼠标位置更新状态按钮图标
    if (object == completeButton) {
        if (event->type() == QEvent::Enter) {
            mHovering = true;
            updateCompleteAppearance();
            return false;
        } else if (event->type() == QEvent::Leave) {
            mHovering = false;
            updateCompleteAppearance();
            return false;
        }
    }

    return QWidget::eventFilter(object, event);
}

void ListItem::startEditing() {
    lineEdit->setText(label->text());
    stack->setCurrentIndex(1);
    lineEdit->setFocus();
    lineEdit->selectAll();
}

void ListItem::finishEditing() {
    label->setText(lineEdit->text());
    stack->setCurrentIndex(0);
    updateCompleteAppearance();
    updateGeometry();

    emit statusChanged();
}

void ListItem::onCompleteClicked() {
    mCompleted = !mCompleted;
    updateCompleteAppearance();

    emit statusChanged();
}

void ListItem::updateCompleteAppearance() {
    auto GlobalConfig = GetConfig();

    label->setStyleSheet(std::format(
        "color: {};",
        mCompleted ? GlobalConfig.theme.item.font_color_completed :
                     GlobalConfig.theme.item.font_color
    ).c_str());

    if (mCompleted) {
        completeButton->setIcon(QIcon(donePixMap));
    } else if (mHovering) {
        completeButton->setIcon(QIcon(hoverPixMap));
    } else {
        completeButton->setIcon(QIcon(emptyPixMap));
    }
}