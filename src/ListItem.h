#pragma once

#include <string>
#include <QWidget>
#include <QPixmap>

class QLabel;
class QLineEdit;
class QStackedWidget;
class QPushButton;

class ListItem : public QWidget {
    Q_OBJECT
public:
    explicit ListItem(QString content, bool completed = false, QWidget* parent = nullptr);

    bool isCompleted() const;
    std::string getContent() const;

signals:
    /**
     * @brief Changes on item's content or `completed` 
     *        status will trigger this signal.
     */
    void statusChanged();

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    QSize sizeHint() const override;

private slots:
    void startEditing();
    void finishEditing();
    void onCompleteClicked();

private:
    /**
     * @brief Update label's text color and complete button's icon
     *        depending on `mCompleted` and `mHovering`.
     */
    void updateCompleteAppearance();

    QLabel* label;
    QLineEdit* lineEdit;
    QStackedWidget* stack;
    QPushButton* completeButton = nullptr;

    QPixmap donePixMap;
    QPixmap hoverPixMap;
    QPixmap emptyPixMap;

    bool mCompleted = false;
    bool mHovering = false;
};