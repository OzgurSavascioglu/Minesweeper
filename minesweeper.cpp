#include <QApplication>
#include <QInputDialog>
#include "gamewindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // set the number of rows
    bool ok;
    unsigned int N = QInputDialog::getInt(nullptr, "New Game", "# of rows:", 10, 2, 100, 1, &ok);
    if (!ok) return 0;

    // set the number of columns
    unsigned int M = QInputDialog::getInt(nullptr, "New Game", "# of columns:", 10, 2, 100, 1, &ok);
    if (!ok) return 0;

    //set the number of mines
    unsigned int mines = QInputDialog::getInt(nullptr, "New Game", "# of mines:", 4, 1, N * M, 1, &ok);
    if (!ok) return 0;

    //create and show the game window
    gameWindow minesweeper(N,M,mines);//create the main window
    minesweeper.setWindowIcon(QIcon(":/images/mine.png"));//set the frame icon
    minesweeper.setWindowTitle("Minesweeper");//set the frame title
    minesweeper.show();//show the window

    return app.exec();
}
