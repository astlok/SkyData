#include <QtWidgets>
#include <iostream>

#include "ClientsConnection.h"
#include "MessageUpdater.h"
#include "Observer.h"
#include "ClientSender.h"
#include "MainWidget.h"

std::queue<std::pair<std::shared_ptr<Message>, bool>> MessageUpdater::processed_messages = std::queue<std::pair<std::shared_ptr<Message>, bool>> ();
std::mutex MessageUpdater::mtx_stat;

int main(int argc, char** argv) {
    std::string name;
    std::string device;
    std::string sync_folder;

    std::ifstream fin;
    fin.open("config.txt");
    if (fin.is_open()) {
        fin >> name >> device >> sync_folder;
    }

    if (!fin.is_open() || name.empty() || device.empty() || sync_folder.empty()) {
        QApplication a(argc, argv);
        MainWidget w(name, device, sync_folder);
        w.setFixedSize(500, 300);
        w.show();

        while (a.exec()) {}

        std::ofstream fout("config.txt");
        fout << name << std::endl << device << std::endl << sync_folder;
        fout.close();
    }
    fin.close();

    Observer observer(name, device, sync_folder);
    observer.update_client("127.0.0.1", "7777", "127.0.0.1", "7779");

    return EXIT_SUCCESS;
}
