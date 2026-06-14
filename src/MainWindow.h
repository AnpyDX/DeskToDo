#pragma once
#include <QWidget>

class QVBoxLayout;
class QScrollArea;

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow();

    /**
     * @brief Set window's titlebar and background color.
     *
     * @note  TitleBar's color customization only works on
     *        Windows 11 Insider 22000 +.
     *        Currently theme color only affects background
     *        on macOS, Linux and other versions of Windows. 
     */
    void setThemeColor(QColor color);

private slots:
    void onAddItem();
    void onClearCompleted();

private:
    void loadHistoryItems();
    void saveHistoryItems();

    QVBoxLayout* listLayout = nullptr;
    QScrollArea* scrollArea = nullptr;
};