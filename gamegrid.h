#ifndef GAMEGRID_H
#define GAMEGRID_H

#include <QFrame>
#include "cell.h"

class gameGrid : public QFrame
{
    Q_OBJECT
public:
    //contructor
    explicit gameGrid(unsigned int nRows,unsigned int mColumns,unsigned int totMines, QWidget* parent = nullptr);
    //class Object that stores all the cells
    QVector<Cell *> allCells;


    //getters and setters
    unsigned int getRows() const;
    unsigned int getColumns() const;
    unsigned int getMines() const;
    unsigned int getRemaining() const;

    //function that creates the cells and print the initial state
    void printCells();
    //function that checks the cells during the recursion
    void checkCell(unsigned int index);

    //pattern checkers
    unsigned int basic1Pattern() const;
    void checkWhatUserKnows(unsigned int cellIndex, std::vector<unsigned int>& revealedCells,
                            std::vector<unsigned int>& knownMines, std::vector<unsigned int>& noInfoCells, unsigned int& revealedCount,
                            unsigned int& knownMineCount, unsigned int& noInfoCount) const;
    unsigned int pattern121() const;
    unsigned int pattern1221() const;
    unsigned int pattern_One_One() const;

public slots:
    void revealAllMines();//show all mines in case of end game
    void recursionCase();//recursion
    void loseDialog();//game lost dialog screen
    void winDialog();//game won dialog screen
    void decreaseCounter();//decrease the number of remaining cells

signals:
    void youWin();//game won
    void closeHint();//disable the hint button
    void indirectHintClose();//disable the hint button
    void scoreUpdate();//update the score

private:
    unsigned int rows;//number of rows
    unsigned int columns;//number of columns
    unsigned int mines;//number of mines
    unsigned int remaining;//number of remaining cells without mines
};

#endif // GAMEGRID_H
