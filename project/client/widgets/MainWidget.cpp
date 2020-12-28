#include "MainWidget.h"

#include <QtWidgets>
#include <iostream>

MainWidget::MainWidget(std::string &name, std::string &device, std::string &sync_folder, QWidget *parent) :
        m_name(name),
        m_device(device),
        m_sync_folder(sync_folder),
        QWidget(parent) {
    m_main_layout = new QGridLayout;
    m_button = new QPushButton(tr("Sync"));
    m_button_dialog = new QPushButton(tr("Chose directory"));

    m_line_edits.resize(3);
    for (auto &m_line_edit : m_line_edits) {
        m_line_edit = new QLineEdit();
    }

    m_labels.push_back(new QLabel("Username:"));
    m_labels.push_back(new QLabel("Device name:"));
    m_labels.push_back(new QLabel("Sync folder:"));

    m_main_layout->addWidget(m_button, 4, 1);
    m_main_layout->addWidget(m_button_dialog, 3, 1);
    for (size_t i = 0; i < m_line_edits.size(); ++i) {
        m_main_layout->addWidget(m_line_edits[i], i, 1);
    }
    for (size_t i = 0; i < m_labels.size(); ++i) {
        m_main_layout->addWidget(m_labels[i], i, 0);
    }
    setLayout(m_main_layout);
    setWindowTitle(tr("SkyData"));
    connect(m_button, SIGNAL(released()), this, SLOT(onButtonReleased()));
    connect(m_button_dialog, SIGNAL(released()), this, SLOT(onDialogButton()));
}

MainWidget::~MainWidget() {
    delete m_button;
    delete m_main_layout;
    for (auto line : m_line_edits) {
        delete line;
    }
    for (auto label : m_labels) {
        delete label;
    }
}

void MainWidget::onButtonReleased() {
    m_name = m_line_edits[0]->text().toStdString();
    m_device = m_line_edits[1]->text().toStdString();
    m_sync_folder = m_line_edits[2]->text().toStdString();
    close();
}

void MainWidget::onDialogButton() {
    m_line_edits[2]->setText(QFileDialog::getExistingDirectory());
}
