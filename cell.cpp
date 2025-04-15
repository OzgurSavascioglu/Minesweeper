#include "cell.h"
#include "recursionstack.h"
#include <QInputDialog>
#include <QString>
#include <QMouseEvent>
#include <QPushButton>
#include <QPainter>

//construct the cell
Cell::Cell(unsigned int index, unsigned int mineInput, QWidget* parent) : QPushButton(parent), positionIndex(index), mine(mineInput)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->revealed=0;//initialize as unrevealed
    this->knownMine=0;//initialize as not a known mine

    //create connections
    connect(this, &Cell::leftClick, this, &Cell::startMineCheck);//if left click, start the mine check for the cell
    connect(this, &Cell::rightClick, this, &Cell::insertFlag);//if right click, insert a flag
    connect(this, &Cell::unclick, this, &Cell::closeExistingHint);//make a hinted cell empty
    connect(this, &Cell::showNumber, this, &Cell::updateButtonText);//if number is revealed, update the button text
    connect(this, &Cell::showEmpty, this, &Cell::changeToEmpty);//if cell is empty, show the empty icon
}

//mouse click function
void Cell::mousePressEvent(QMouseEvent* e)
{
    //case: left click
    if (e->buttons() == Qt::LeftButton)
        emit leftClick();//send the left click signal

    //case: right click
    else if (e->buttons() == Qt::RightButton)
        emit rightClick();//send the right click signal
}

//override the paint event
//to show RGB icons after disable
void Cell::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QIcon icon = this->icon();//get the existing icon

    QPainter painter(this);//create the painter object
    QRect rect = QRect(0, 0, width(), height());//set the size

    //case: button is disabled
    if (!isEnabled()) {
        if(this->mine==1)
            painter.setOpacity(1); // mines look solid
        else if(this->neighMineCount==0)
            painter.setOpacity(0.9); // 0 neigh mine cells are slightly transparent
        else if(this->neighMineCount!=0)
            painter.setOpacity(1); // cells with neighMines looks solid
    }

    icon.paint(&painter, rect, Qt::AlignCenter);// Paint the icon
}


//SLOTS
//prints the neighMineCount
void Cell::updateButtonText() {
    this->revealed=1;//set the cell as revealed
    QString numberToShow = QString::number(this->neighMineCount);//get the neighMIneCount
    QString imagePath = ":/images/" + numberToShow + ".png";//create the image path
    QIcon icon = QIcon(QPixmap(imagePath).scaled(QSize(23, 23)));//get the number icon
    this->setIcon(icon);//set the icon
    this->setEnabled(false);//disable the cell
}

//prints the empty sign
void Cell::changeToEmpty() {
    this->revealed=1;//set the cell as revealed
    QIcon icon = QIcon(QPixmap(":/images/0.png").scaled(QSize(23, 23)));//get the zero icon
    this->setIcon(icon);//set the icon
    this->setEnabled(false);//disable the cell
}

//checks the cell after left click
void Cell::startMineCheck() {
    //if flagged remove the flag
    if(this->flagged==1){
        this->flagged=0;
        QIcon icon = QIcon(QPixmap(":/images/empty.png").scaled(QSize(23, 23)));//get the empty icon
        this->setIcon(icon);//set the icon
    }

    //if mine end the game
    if(this->mine==1){
        QIcon icon = QIcon(QPixmap(":/images/mine.png").scaled(QSize(23, 23)));//get the mine icon
        this->setIcon(icon);//set the icon
        emit youLose();//send the game lost signal
        this->setEnabled(false);//disable the cell
    }

    //if there is at lest one neighMine show the number
    else if(this->neighMineCount!=0){
        emit showNumber();//send the show number signal
        emit indirectHint();//if there is an active hint in the game, close it
    }

    //if there is no neighMine start recursion
    else if(this->neighMineCount==0){
        globalStack.push(this->getPositionIndex());//push the cell index to the recursion stack

        //put the empty sign to the cell
        emit showEmpty();
        emit indirectHint();//if there is an active hint in the game, close it
        emit startRecursion();//start recursion

    }
}

//insert a flag after right click
void Cell::insertFlag() {
    //if flagged: remove the flag
    if(this->flagged==1){
        this->flagged=0;
        QIcon icon = QIcon(QPixmap(":/images/empty.png").scaled(QSize(23, 23)));//get the empty icon
        this->setIcon(icon);//set the icon
    }

    //else insert the flag
    else{
        this->flagged=1;
        QIcon icon = QIcon(QPixmap(":/images/flag.png").scaled(QSize(23, 23)));//get the flag icon
        this->setIcon(icon);//set the icon
    }
}

//make a hinted cell empty
void Cell::closeExistingHint() {
    QIcon icon = QIcon(QPixmap(":/images/empty.png").scaled(QSize(23, 23)));//get the empty icon
    this->setIcon(icon);//set the icon
}

//setters and getters
unsigned int Cell::getPositionIndex() const
{
    return positionIndex;
}

void Cell::setPositionIndex(unsigned int index)
{
    positionIndex = index;
}

unsigned int Cell::getMine() const
{
    return mine;
}

void Cell::setMine(unsigned int mine)
{
    this->mine = mine;
}

unsigned int Cell::getNeighMineCount() const
{
    return neighMineCount;
}

void Cell::setNeighMineCount(unsigned int count)
{
    neighMineCount = count;
}

unsigned int Cell::getRevealed() const
{
    return revealed;
}

void Cell::setRevealed(unsigned int status)
{
    revealed = status;
}

unsigned int Cell::getFlagged() const
{
    return flagged;
}

void Cell::setFlagged(unsigned int flag)
{
    flagged = flag;
}

unsigned int Cell::getKnownMine() const
{
    return knownMine;
}

void Cell::setKnownMine(unsigned int known)
{
    knownMine = known;
}

