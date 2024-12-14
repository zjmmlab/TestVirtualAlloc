#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <iostream>

const size_t bufferSize = 1.5 * 1024 * 1024 * 1024;  // Define the size of the buffer to be allocated (e.g. 1.5 GB)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buffer = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_AllocButton_clicked()
{
#ifdef Q_OS_WIN

    // Allocating memory using VirtualAlloc
    buffer = VirtualAlloc(
        nullptr,                   // System selects memory address
        bufferSize,                // The size of the allocation in bytes
        MEM_COMMIT | MEM_RESERVE,  // Allocation Type
        PAGE_READWRITE             // Read and write permissions
        );

    if (buffer == nullptr) {
        std::cerr << "Failed to allocate memory! Error code: " << GetLastError() << std::endl;
        return ;
    }

    std::cout << "Successfully allocated " << bufferSize / (1024 * 1024) << "MB" << " bytes of memory at address: " << buffer << std::endl;

    // Use memory here
    memset(buffer, 0, bufferSize);
    std::cout << "Data buffer has been initialized." << std::endl;

    // Locking memory pages
    if (VirtualLock(buffer, bufferSize)) {
        std::cout << "Memory locked successfully." << std::endl;
        MessageBox(nullptr, L"Memory locked successfully.", L"Platform-Specific", MB_OK);

        // Simulate storing binary data
        char* data = static_cast<char*>(buffer);
        for (size_t i = 0; i < bufferSize; ++i) {
            data[i] = static_cast<char>(i % 256); // Sample data filling
        }

    } else {
        std::cerr << "VirtualLock failed. Error: " << GetLastError() << std::endl;
        MessageBox(nullptr, L"VirtualLock failed.", L"Platform-Specific", MB_OK);
    }

#endif
}


void MainWindow::on_FreeButton_clicked()
{
#ifdef Q_OS_WIN

    // Unlock memory page
    if (VirtualUnlock(buffer, bufferSize)) {
        std::cout << "Memory unlocked successfully." << std::endl;
    } else {
        std::cerr << "VirtualUnlock failed. Error: " << GetLastError() << std::endl;
    }

    // Release memory after use
    if (VirtualFree(buffer, 0, MEM_RELEASE)) {
        std::cout << "Memory has been successfully freed." << std::endl;
    } else {
        std::cerr << "Failed to free memory! Error code: " << GetLastError() << std::endl;
    }

#endif
}

