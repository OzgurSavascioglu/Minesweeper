#ifndef CELL_H
#define CELL_H
#include <QLabel>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <QMap>
#include <QMessageBox>
#include <QMouseEvent>
#include <QIcon>
#include <QPixmap>

class Cell: public QPushButton
{
    Q_OBJECT

public:
    //constructor
    explicit Cell(unsigned int index=0, unsigned int mineInput=0, QWidget* parent = nullptr);

    //setters and getters
    unsigned int getPositionIndex() const;
    void setPositionIndex(unsigned int index);
    unsigned int getMine() const;
    void setMine(unsigned int mine);
    unsigned int getNeighMineCount() const;
    void setNeighMineCount(unsigned int count);
    unsigned int getRevealed() const;
    void setRevealed (unsigned int status);
    unsigned int getFlagged() const;
    void setFlagged (unsigned int flag);
    unsigned int getKnownMine() const;
    void setKnownMine(unsigned int known);

    //mouseClickFunction
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

public slots:
    void updateButtonText();//show the number of neigbor mines
    void changeToEmpty();//show the empty cell visual
    void startMineCheck();//check if there is a mine in the cell
    void insertFlag();//insert a flag
    void closeExistingHint();//insert a flag

signals:
    void youLose();//game is lost
    void showNumber();//show the number of neigh mines
    void showEmpty();//show the empty character '-'
    void startRecursion();//start the recursion function in case cell is empty
    void leftClick();//left mouse click
    void unclick();//to close the hint
    void rightClick();//right mouse click
    void indirectHint();//in case of indirect hint close

private:
    unsigned int positionIndex;//cell position
    unsigned int mine;//if mine exist
    unsigned int neighMineCount;//count of neihgboring mines
    unsigned int revealed;//if the cell revealed
    unsigned int flagged;//if the cell flagged
    unsigned int knownMine;//if the cell flagged

};

#endif // CELL_H
