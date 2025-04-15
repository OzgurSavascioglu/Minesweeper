#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H
#include <QMainWindow>
#include <QPushButton>
#include "gamegrid.h"

class gameWindow : public QMainWindow
{
    Q_OBJECT
public:
    //contructor
    gameWindow(unsigned int nRows,unsigned int mColumns,unsigned int totMines, QWidget* parent = nullptr);

public slots:
    void changeScore();//update the score
    void restartGame();//restart the game
    void giveHint();//give a hint
    void hintEnd();//reveal an ixisting hint cell and end hint case
    void indirectHindEnd();//if hinted cell is revealed close the hint
    void disableHint();//disable the hint button

private:
    //create the game frame
    void  createEnvironment(unsigned int nRows,unsigned int mColumns,unsigned int totMines);

private:
    QFrame* myFrame;
    gameGrid* myGrid;//gameGrid object
    QPushButton* restart;//restart button
    QPushButton* hint;//hint button
    QLabel* scoreLabel;//score label
    unsigned int score;//score
    unsigned int hintStatus;//hintStatus
    int hintIndex;//hintIndex
    //QTimer* hintTimer;

};

#endif // GAMEWINDOW_H
