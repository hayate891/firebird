#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *main_window;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    emu_thread = &emu;

    emu.start();

    ui->setupUi(this);

    connect(&refresh_timer, SIGNAL(timeout()), this, SLOT(refresh()));
    connect(&emu, SIGNAL(putchar(char)), this, SLOT(putchar(char)), Qt::QueuedConnection);
    connect(ui->actionEnter_Debugger, SIGNAL(triggered()), &emu, SLOT(enterDebugger()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(debugCommand()));

    refresh_timer.setInterval(1000 / 60); //60 fps
    refresh_timer.start();

    ui->graphicsView->setScene(&lcd_scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}

extern "C"
{
    #include "lcd.h"
}

void MainWindow::refresh()
{
    lcd_scene.clear();

    if(emulate_cx)
    {
        QByteArray framebuffer(320 * 240 * 2, 0);
        uint32_t useless[3];
        lcd_cx_draw_frame(reinterpret_cast<u16*>(framebuffer.data()), useless);
        QImage image(reinterpret_cast<const uchar*>(framebuffer.data()), 320, 240, 320 * 2, QImage::Format_RGB16);

        lcd_scene.addPixmap(QPixmap::fromImage(image));
    }
}

void MainWindow::putchar(char c)
{
    if(c == '\r')
        return;

    QString s(c);

    ui->plainTextEdit->moveCursor(QTextCursor::End);
    ui->plainTextEdit->insertPlainText(s);
}

void MainWindow::debugCommand()
{
    debug_command = ui->lineEdit->text().toLatin1();
    emit debuggerCommand();
}
