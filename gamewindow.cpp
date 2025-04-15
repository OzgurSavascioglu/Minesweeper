#include "gamewindow.h"
#include "gamegrid.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QInputDialog>
#include <QPalette>

//constructor
gameWindow::gameWindow(unsigned int nRows,unsigned int mColumns,unsigned int totMines, QWidget* parent)
    : QMainWindow(parent)
    , myFrame(nullptr)
{
    //create the game environment
    createEnvironment(nRows,mColumns,totMines);
}

//create the game environment
void gameWindow::createEnvironment(unsigned int nRows,unsigned int mColumns,unsigned int totMines)
{
    QFrame* newMainFrame=new QFrame(this);
    myGrid = new gameGrid (nRows, mColumns, totMines, newMainFrame); //call the gameGrid constuctor
    score=0; //initialize the score
    hintStatus=0; //initialize the hintStatus, means no active hint
    hintIndex=-1; //initialize the hintIndex, means no active hintIndex
    scoreLabel = new QLabel("Score: " + QString::number(score),newMainFrame); //create the score label

    restart = new QPushButton(newMainFrame);//create the restart button
    restart->setText("Restart"); //set the restart button text

    hint = new QPushButton(newMainFrame); //create the hint button
    hint->setText("Hint"); //set the hint button text

    QHBoxLayout *topLayout = new QHBoxLayout(); //create the layout of the top part(score, restart and hint buttons)
    topLayout->addWidget(scoreLabel); //add the score label to layout
    restart->setMinimumSize(30, 30); //set the minimum size for the restart button
    topLayout->addWidget(restart); //add the restart button to layout
    hint->setMinimumSize(30, 30); //set the minimum size for the hint button
    topLayout->addWidget(hint); //add the restart button to layout

    QVBoxLayout *layout = new QVBoxLayout(newMainFrame); //create the mainframe layout
    layout->addLayout(topLayout); //add the toplayout to the mainframe layout
    layout->addWidget(myGrid); //add the grid to the mainframe layout
    newMainFrame->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); //set the size policy, horizontal and vertical minimum
    this->setCentralWidget(newMainFrame); //make the newMain Frame the central widget

    //create connections
    connect(myGrid, &gameGrid::scoreUpdate, this, &gameWindow::changeScore); //when score is updated change the score label
    connect(restart, &QPushButton::clicked, this, &gameWindow::restartGame); //when restart is clicked restart the game
    connect(hint, &QPushButton::clicked, this, &gameWindow::giveHint); //when hint is clicked give hint/or reveal existing hint
    connect(myGrid, &gameGrid::closeHint, this, &gameWindow::disableHint); //at the end of the game, disables hint buttons
    connect(myGrid, &gameGrid::indirectHintClose, this, &gameWindow::indirectHindEnd); //if another cell is revealed, delete the existing hint
}

//SLOTS
//update the score
void gameWindow::changeScore(){
    score++;
    scoreLabel->setText("Score: " + QString::number(score));
}

//restart the game
void gameWindow::restartGame(){
    bool ok;
    unsigned int N = QInputDialog::getInt(nullptr, "New Game", "# of rows:", 10, 2, 100, 1, &ok);
    if (!ok){
        close();
        return;
    }
    // set the number of columns
    unsigned int M = QInputDialog::getInt(nullptr, "New Game", "# of columns:", 10, 2, 100, 1, &ok);
    if (!ok){
        close();
        return;
    }
    //set the number of mines
    unsigned int mines = QInputDialog::getInt(nullptr, "New Game", "# of mines:", 4, 1, N * M, 1, &ok);
    if (!ok){
        close();
        return;
    }
    close();

    createEnvironment(N,M,mines);
    show();
}

void gameWindow::giveHint(){
    //check the hint status,
    //if there is no active hint(i.e. hintStatus==0) search the cells
    if(hintStatus==0){
        //try the basic pattern
        //if not found, hintIndex=-1, if found hintIndex is the relevant cell index
        hintIndex=myGrid->basic1Pattern();
        //in case not found re-try it, since it is possible to find with a hint with the information available from the first run
        if(hintIndex==-1)
            hintIndex=myGrid->basic1Pattern();

        //in case not found. try 1-2-2-1 pattern
        if(hintIndex==-1)
            hintIndex=myGrid->pattern1221();

        //in case not found. try 1-2-1 pattern
        if(hintIndex==-1)
            hintIndex=myGrid->pattern121();
        //in case not found. try 1-1-1 pattern
        if(hintIndex==-1)
            hintIndex=myGrid->pattern_One_One();

        //if hint found, highlight the cell
        if(hintIndex!=-1){
            QPalette hintPalette =myGrid->allCells[hintIndex]->palette();
            hintPalette.setColor(QPalette::Button, QColor(Qt::darkGreen));
            myGrid->allCells[hintIndex]->setPalette(hintPalette);
            QIcon icon = QIcon(QPixmap(":/images/hint.png").scaled(QSize(23, 23)));//get the empty icon
            myGrid->allCells[hintIndex]->setIcon(icon);//set the icon
            hintStatus=1;//change the hint status to active(i.e. hintStatus==1)
        }

    }

    //if there is an active hint(i.e. hintStatus==1) reveal the cell
    else {
        hintStatus=0; //change the hint status to inactive(i.e. hintStatus==0)
        emit myGrid->allCells[hintIndex]->leftClick();//leftClick signal will trigger the reveal process
        hintEnd();// call the hint end function
        hintIndex=-1;//reset the hintIndex
    }
}

//this method checks if there an active hint during a reveal attemp by a leftClick
//in case a cell is revealed by player during an active hint, close the hint
void gameWindow::indirectHindEnd(){
    //check if there is an active hint(i.e. hintStatus==1)
    if(hintStatus==1){
        hintStatus=0;//change the hint status to inactive(i.e. hintStatus==0)
        if(myGrid->allCells[hintIndex]->getRevealed()==0)
            emit myGrid->allCells[hintIndex]->unclick();
        hintEnd();//call the hint end function
        hintIndex=-1;//reset the hintIndex
    }
}

//reveal an existing hint cell and end hint case
void gameWindow::hintEnd(){
    //reset the palette color to white
    QPalette hintPalette =myGrid->allCells[hintIndex]->palette();
    hintPalette.setColor(QPalette::Button, QColor(Qt::white));
    myGrid->allCells[hintIndex]->setPalette(hintPalette);//change the color of the hint cell
}

//create the game environment
void gameWindow::disableHint(){
    hint->setEnabled(false);
}
