#include <format>
#include <cstdlib>
#include <toml.hpp>

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    try {
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    }
    catch (const toml::exception& err) {
        QMessageBox::critical(nullptr, "DeskToDo Error", 
            std::format("DeskToDo Crashed \n- TOML file read/parse error：{}", err.what()).c_str()
        );
    }
    catch (const std::exception& err) {
        QMessageBox::critical(nullptr, "DeskToDo Error", 
            std::format("DeskToDo Crashed \n- Unknown internal error：{}", err.what()).c_str()
        );
    }
    
    return EXIT_FAILURE;
}