#include "MainWindow.h"
#include "ListItem.h"
#include "Data.hpp"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>

#ifdef _WIN32
#include <Windows.h>
#include <dwmapi.h>
#endif

MainWindow::MainWindow() : QWidget() {
    auto GlobalConfig = GetConfig();

    setThemeColor(QColor::fromString(GlobalConfig.theme.window.background));

    // Make window always on top, no close and resize button on titlebar.
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);

    /* ============= Buttons Area ============= */
    auto* toolbar = new QWidget;
    auto* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(8, 8, 8, 4);

    auto* addBtn = new QPushButton("➕ Add");
    auto* clearBtn = new QPushButton("🗑️ Clear");
    toolbarLayout->addWidget(addBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(clearBtn);

    QString btnStyle = std::format(
        R"(
        QPushButton {{
            color: {};
            border: none;
            border-radius: {};
            padding: 4px 12px;
            font-size: {};
            background-color: {};
        }}
        QPushButton:hover {{ background-color: {}; }}
        QPushButton:pressed {{ background-color: {}; }}
        )", 
        GlobalConfig.theme.button.font_color,
        GlobalConfig.theme.button.border_radius,
        GlobalConfig.theme.button.font_size,
        GlobalConfig.theme.button.background,
        GlobalConfig.theme.button.background_hover,
        GlobalConfig.theme.button.background_pressed
    ).c_str();

    addBtn  ->setStyleSheet(btnStyle);
    clearBtn->setStyleSheet(btnStyle);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddItem);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearCompleted);

    /* ============= TODO List Area ============= */
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    scrollArea->setStyleSheet(std::format(
        R"(
        /* Scrollbar */
        QScrollBar:vertical {{
            background: {};
            width: {};
            margin: 0px;
        }}
        /* Scrollbar Handle */
        QScrollBar::handle:vertical {{
            background: {};
            min-height: 30px;
            border-radius: {};
        }}
        QScrollBar::handle:vertical:hover {{
            background: {};
        }}
        QScrollBar::handle:vertical:pressed {{
            background: {};
        }}
        /* Hide top and bottom arrows */
        QScrollBar::sub-line:vertical,
        QScrollBar::add-line:vertical {{
            height: 0px;
            border: none;
        }}
        )",
        GlobalConfig.theme.window.background,
        GlobalConfig.theme.scrollbar.width,
        GlobalConfig.theme.scrollbar.background,
        GlobalConfig.theme.scrollbar.border_radius,
        GlobalConfig.theme.scrollbar.background_hover,
        GlobalConfig.theme.scrollbar.background_pressed
    ).c_str());

    auto* container = new QWidget;
    auto* verticalLayout = new QVBoxLayout(container);
    listLayout = verticalLayout;
    verticalLayout->setContentsMargins(0, 4, 0, 4);
    verticalLayout->setSpacing(3);

    loadHistoryItems();

    verticalLayout->addStretch(1);
    scrollArea->setWidget(container);

    // Apply layouts to window
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(scrollArea, 1);
}

void MainWindow::setThemeColor(QColor color) {

#ifdef _WIN32  // Trying to set titlebar's color only on Windows
    HWND hwnd = (HWND)winId();
    
    BOOL fnSupported = FALSE;
    if (SUCCEEDED(::DwmSetWindowAttribute(
        hwnd, DWMWA_CAPTION_COLOR,
        &fnSupported, sizeof(fnSupported)))
    ){
        // NOTE: `DWMWA_CAPTION_COLOR` supported from Windows 11 Insider 22000
        COLORREF titleBarColor = RGB(color.red(), color.green(), color.blue());
        ::DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &titleBarColor, sizeof(titleBarColor));
    }
#endif

    setStyleSheet(std::format(
        "background-color: rgb({}, {}, {})", 
        color.red(), color.green(), color.blue()).c_str()
    );
}

void MainWindow::onAddItem() {
    if (!listLayout) return;

    auto GlobalConfig = GetConfig();
    auto* newItem = new ListItem(GlobalConfig.settings.default_content.c_str());

    int insertIndex = listLayout->count() - 1;
    listLayout->insertWidget(insertIndex, newItem);
    scrollArea->ensureWidgetVisible(newItem);

    saveHistoryItems();
}

void MainWindow::onClearCompleted() {
    if (!listLayout) return;

    for (int i = listLayout->count() - 1; i >= 0; --i) {
        QLayoutItem* item = listLayout->itemAt(i);
        if (!item) continue;

        QWidget* w = item->widget();
        auto* listItem = qobject_cast<ListItem*>(w);
        if (listItem && listItem->isCompleted()) {
            listLayout->removeWidget(w);
            w->deleteLater();
        }
    }

    saveHistoryItems();
}

void MainWindow::loadHistoryItems() {
    auto records = GetHistory();

    for (auto& task : records) {
        auto* item = new ListItem(task.content.c_str(), task.completed);
        listLayout->addWidget(item);
        connect(item, &ListItem::statusChanged, this, &MainWindow::saveHistoryItems);
    }
}

void MainWindow::saveHistoryItems() {
    if (!listLayout) return;
    
    std::vector<TaskRecord> records;

    const size_t len = listLayout->count();
    for (size_t idx = 0; idx < len; idx += 1) {
        QLayoutItem* item = listLayout->itemAt(idx);
        if (item == nullptr) continue;

        QWidget* w = item->widget();
        if (w == nullptr) continue;

        auto* listItem = qobject_cast<ListItem*>(w);
        records.emplace_back(listItem->isCompleted(), listItem->getContent());
    }

    SaveHistory(records);
}