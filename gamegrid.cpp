#include "gamegrid.h"
#include "recursionstack.h"

#include <QGridLayout>
#include <QSpacerItem>
#include <QSet>
#include <random>
#include <set>

//contructor
gameGrid::gameGrid(unsigned int nRows,unsigned int mColumns,unsigned int totMines, QWidget* parent)
    : QFrame(parent), rows(nRows), columns(mColumns), mines(totMines)
{
    //set the remaining cells without mines
    this->remaining=(rows*columns)-mines;

    //layout settings
    this->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto layout = new QGridLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    this->setLayout(layout);

    //connections
    connect(this, &gameGrid::youWin, this, &gameGrid::revealAllMines); //when player won the game, reveal all cells,
    connect(this, &gameGrid::youWin, this, &gameGrid::winDialog);//when player won the game, open the win dialog

    //create and print cells
    printCells();
}

//function that creates the cells and print the initial state
void gameGrid::printCells()
{
    //initialize grid with random values
    std::vector<int> myGrid;
    myGrid.resize(rows*columns);
    std::fill_n(myGrid.begin(), mines, 1);
    std::fill_n(myGrid.begin() + mines, rows*columns-mines, 0);
    std::shuffle(myGrid.begin(), myGrid.end(), std::mt19937{std::random_device{}()});

    //create the cell objects
    for (unsigned int i = 0; i < rows; i++){
        for (unsigned int j = 0; j < columns; j++){
            unsigned int topLeft=0;
            unsigned int topMid=0;
            unsigned int topRight=0;
            unsigned int botLeft=0;
            unsigned int botMid=0;
            unsigned int botRight=0;
            unsigned int sameLeft=0;
            unsigned int sameRight=0;

            //calculate the neighMines
            if(i!=0){
                topMid=myGrid[((i-1)*columns)+j];
                if(j!=0)
                    topLeft=myGrid[((i-1)*columns)+j-1];
                if(j!=columns-1)
                    topRight=myGrid[((i-1)*columns)+j+1];
            }

            if(i!=rows-1){
                botMid=myGrid[((i+1)*columns)+j];
                if(j!=0)
                    botLeft=myGrid[((i+1)*columns)+j-1];
                if(j!=columns-1)
                    botRight=myGrid[((i+1)*columns)+j+1];
            }

            if(j!=0){
                sameLeft=myGrid[(i*columns)+j-1];
            }

            if(j!=columns-1){
                sameRight=myGrid[(i*columns)+j+1];
            }

            int neighMines=0+(topLeft+topMid+topRight+botLeft+botMid+botRight+sameLeft+sameRight);
            //initialize mineInput
            unsigned int mineInput=myGrid[i*columns+j];
            //initialize index
            unsigned int index=i*columns+j;
            //add the cell to the Qvector
            allCells.push_back(new Cell(index,mineInput));
            //initialize neighMines
            allCells[i*columns+j]->setNeighMineCount(neighMines);

            //layout settings
            QIcon icon = QIcon(QPixmap(":/images/empty.png").scaled(QSize(23, 23)));
            allCells[i*columns+j]->setIcon(icon);
            allCells[i*columns+j]->setFixedSize(23, 23);
            static_cast<QGridLayout*>(this->layout())->addWidget(allCells[i*columns+j], i, j);

            //create connections
            connect(allCells[i*columns+j], &Cell::youLose, this , &gameGrid::revealAllMines); //when game is lost, reveal all mines
            connect(allCells[i*columns+j], &Cell::startRecursion, this , &gameGrid::recursionCase); //when cell has no neighnour mines, start recursion
            connect(allCells[i*columns+j], &Cell::youLose, this, &gameGrid::loseDialog); //when game lost, open you lose dialog
            connect(allCells[i*columns+j], &Cell::showNumber, this, &gameGrid::decreaseCounter); //when a cell is revealed decrease the remaining cell count
            connect(allCells[i*columns+j], &Cell::showEmpty, this, &gameGrid::decreaseCounter); //when a cell is revealed decrease the remaining cell count
            connect(allCells[i*columns+j], &Cell::indirectHint, this, &gameGrid::indirectHintClose); //when a cell is revealed, delete existing hint
        }
    }

}

//setters and getters
unsigned int gameGrid::getRows() const{
    return rows;
}
unsigned int gameGrid::getColumns() const{
    return columns;
}
unsigned int gameGrid::getMines() const {
    return mines;
}

unsigned int gameGrid::getRemaining() const {
    return remaining;
}

//PATTERN CHECKERS for HINT function

//check the B1 pattern and B2 pattern
//B1: If a number is touching the same number of cells,
//then these cells are all mines.
//B2: If a number is touching the same number of flags,
//then all adjacent cells can be opened.
unsigned int gameGrid::basic1Pattern() const{
    //check the pattern for each cell
    for(unsigned int index=0;index<(rows*columns);index++){
        unsigned int rIndex=index/columns;//calculate the row index of the cell
        unsigned int cIndex=index%columns;//calculate the column index of the cell

        //check if the cell is revealed
        if(allCells[index]->getRevealed()==1 && allCells[index]->getNeighMineCount()!=0){
            //calculate the neighbour cell indexes
            unsigned int topLeft=((rIndex-1)*columns)+cIndex-1;
            unsigned int oneAbove=((rIndex-1)*columns)+cIndex;
            unsigned int topRight=((rIndex-1)*columns)+cIndex+1;
            unsigned int oneLeft=((rIndex)*columns)+cIndex-1;
            unsigned int oneRight=((rIndex)*columns)+cIndex+1;
            unsigned int botLeft=((rIndex+1)*columns)+cIndex-1;
            unsigned int oneBelow=((rIndex+1)*columns)+cIndex;
            unsigned int botRight=((rIndex+1)*columns)+cIndex+1;

            //initialize the counts, stores the knowledge of the user as total numbers
            unsigned int revealedCount=0;   //revealed neighbours total
            unsigned int knownMineCount=0;  //known neighbour mines total
            unsigned int noInfoCount=0;   //the neighbour cells that user has no information
            unsigned int borderCount=0;  //if there is no neighbour cell in a direction increase border count

            //create the vectors that stores each type of neighbours
            std::vector<unsigned int> revealedCells;
            std::vector<unsigned int> knownMines;
            std::vector<unsigned int> noInfoCells;
            std::vector<unsigned int> borders;

            //start to check the neighbours
            //topLeft
            //check if it is border
            if(rIndex==0){
                borderCount++;//increase the border count
                borders.push_back(1);//add to vector
            }

            else if(cIndex==0){
                borderCount++;//increase the border count
                borders.push_back(1);//add to vector
            }

            else
                checkWhatUserKnows(topLeft, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount); //check the users information

            //oneAbove
            //check if it is border
            if(rIndex==0){
                borderCount++;//increase the border count
                borders.push_back(2);//add to vector
            }

            else
                checkWhatUserKnows(oneAbove, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount); //check the users information


            //topRight
            //check if it is border
            if(rIndex==0){
                borderCount++;//increase the border count
                borders.push_back(3);//add to vector
            }

            else if(cIndex==columns-1){
                borderCount++;//increase the border count
                borders.push_back(3);//add to vector
            }

            else
                checkWhatUserKnows(topRight, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount);//check the users information

            //oneLeft
            //check if it is border
            if(cIndex==0){
                borderCount++;//increase the border count
                borders.push_back(4);//add to vector
            }

            else
                checkWhatUserKnows(oneLeft, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount);//check the users information

            //oneRight
            //check if it is border
            if(cIndex==columns-1){
                borderCount++;//increase the border count
                borders.push_back(5);//add to vector
            }

            else
                checkWhatUserKnows(oneRight, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount);//check the users information

            //botLeft
            //check if it is border
            if(rIndex==rows-1){
                borderCount++;//increase the border count
                borders.push_back(6);//add to vector
            }

            else if(cIndex==0){
                borderCount++;//increase the border count
                borders.push_back(6);//add to vector
            }

            else
                checkWhatUserKnows(botLeft, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount);//check the users information

            //oneBelow
            //check if it is border
            if(rIndex==rows-1){
                borderCount++;//increase the border count
                borders.push_back(7);//add to vector
            }

            else
                checkWhatUserKnows(oneBelow, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount);//check the users information

            //botRight
            //check if it is border
            if(rIndex==rows-1){
                borderCount++;//increase the border count
                borders.push_back(8);//add to vector
            }

            else if(cIndex==columns-1){
                borderCount++;//increase the border count
                borders.push_back(8);//add to vector
            }

            else
                checkWhatUserKnows(botRight, revealedCells, knownMines, noInfoCells,revealedCount,knownMineCount,noInfoCount);//check the users information

            //calculate the remaining unknown cells
            unsigned int checkTheRemaining=8-revealedCount-borderCount-knownMineCount;
            //if there are remaining unknown cells and their number matches with unknown mines,
            //mark them as mine
            if(checkTheRemaining!=0 && checkTheRemaining==allCells[index]->getNeighMineCount()-knownMineCount){
                for(unsigned int i=0;i<checkTheRemaining;i++){
                    allCells[noInfoCells[i]]->setKnownMine(1);
                }
            }
            //if player knowns the location of all the neighbour mines,
            //and there are remaining unknown cells, return the first as hint
            if(allCells[index]->getNeighMineCount()==knownMineCount && noInfoCount!=0){
                for(unsigned int i=0;i<checkTheRemaining;i++){
                    return noInfoCells[i];
                }
            }
        }
    }
    return -1;//if there is no hint, return 0
}

//this function checks the existing knowledge of a user for a cell,
//the function gets the fields in the basic1Pattern function as parameter and populates the vectors,
//and calculate the counts acccording the players knowledge
void gameGrid::checkWhatUserKnows(unsigned int cellIndex,
                        std::vector<unsigned int>& revealedCells, std::vector<unsigned int>& knownMines, std::vector<unsigned int>& noInfoCells,
                        unsigned int& revealedCount, unsigned int& knownMineCount, unsigned int& noInfoCount)  const {
    //case: the cell is a known mine
    if(allCells[cellIndex]->getKnownMine()==1){
        knownMineCount++;//increase the relevant count
        knownMines.push_back(cellIndex);//add to relevant vector
    }
    //case: cell is not revealed
    else if(allCells[cellIndex]->getRevealed()==0){
        noInfoCount++;//increase the relevant count
        noInfoCells.push_back(cellIndex);//add to relevant vector
    }
    //case: cell is revealed
    else if(allCells[cellIndex]->getRevealed()==1){
        revealedCount++;//increase the relevant count
        revealedCells.push_back(cellIndex);//add to relevant vector
    }
}

//checks the 1-2-1 pattern, please check the documentation for the visual representation
unsigned int gameGrid::pattern121() const{
    //check the pattern for each cell
    for(unsigned int index=0;index<(rows*columns);index++){
        unsigned int rIndex=index/columns;//calculate the row index of the cell
        unsigned int cIndex=index%columns;//calculate the column index of the cell

        //check if the cell is revealed
        if(allCells[index]->getRevealed()==1){
            //check if the cell has 2 neigh mines
            if(allCells[index]->getNeighMineCount()==2){

                //CHECK THE RIGHT SIDE HINT
                if(cIndex!=columns-1){
                    //if not a top or bottom row cell
                    if(rIndex!=0 && rIndex!=rows-1){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        unsigned int oneAbove=((rIndex-1)*columns)+cIndex;//index of the cell above
                        unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                        unsigned int sameRight=((rIndex)*columns)+cIndex+1;//index of the right cell

                        //check if top-bottom known
                        if(allCells[oneAbove]->getRevealed()==0 || allCells[oneBelow]->getRevealed()==0)
                            notHint=-1;//if top-bottom not revealed this cell is not hint

                        //check if already known
                        if(allCells[sameRight]->getRevealed()==1)
                            notHint=-1;//if already known this cell is not hint

                        //check the pattern
                        if(allCells[oneAbove]->getNeighMineCount()==1 && allCells[oneBelow]->getNeighMineCount()==1 && notHint==1){
                            return sameRight;//if matches return the cell as hint
                        }
                    }
                }//END OF CHECK THE RIGHT SIDE HINT

                //CHECK THE LEFT SIDE HINT
                if(cIndex!=0){
                    if(rIndex!=0 && rIndex!=rows-1){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        unsigned int oneAbove=((rIndex-1)*columns)+cIndex;
                        unsigned int oneBelow=((rIndex+1)*columns)+cIndex;
                        unsigned int sameLeft=((rIndex)*columns)+cIndex-1;

                        //check if top-bottom known
                        if(allCells[oneAbove]->getRevealed()==0 || allCells[oneBelow]->getRevealed()==0)
                            notHint=-1;//if top-bottom not revealed this cell is not hint

                        //check if already known
                        if(allCells[sameLeft]->getRevealed()==1)
                            notHint=-1;//if already known this cell is not hint

                        //check the pattern
                        if(allCells[oneAbove]->getNeighMineCount()==1 && allCells[oneBelow]->getNeighMineCount()==1 && notHint==1){
                            return sameLeft;//if matches return the cell as hint
                        }
                    }
                }

                //CHECK THE BOTTOM SIDE HINT
                if(rIndex!=rows-1){
                    if(cIndex!=0 && cIndex!=columns-1){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        unsigned int oneLeft=((rIndex)*columns)+cIndex-1;
                        unsigned int oneRight=((rIndex)*columns)+cIndex+1;
                        unsigned int botMid=((rIndex+1)*columns)+cIndex;

                        //check if top-bottom known
                        if(allCells[oneLeft]->getRevealed()==0 || allCells[oneRight]->getRevealed()==0)
                            notHint=-1;//if top-bottom not revealed this cell is not hint

                        //check if already known
                        if(allCells[botMid]->getRevealed()==1)
                            notHint=-1;//if already known this cell is not hint

                        //check the pattern
                        if(allCells[oneLeft]->getNeighMineCount()==1 && allCells[oneRight]->getNeighMineCount()==1 && notHint==1){
                            return botMid;//if matches return the cell as hint
                        }
                    }
                }//END OF CHECK THE BOTTOM SIDE HINT

                //CHECK THE TOP SIDE HINT
                if(rIndex!=0){
                    if(cIndex!=0 && cIndex!=columns-1){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        unsigned int oneLeft=((rIndex)*columns)+cIndex-1;
                        unsigned int oneRight=((rIndex)*columns)+cIndex+1;
                        unsigned int topMid=((rIndex-1)*columns)+cIndex;

                        //check if top-bottom known
                        if(allCells[oneLeft]->getRevealed()==0 || allCells[oneRight]->getRevealed()==0)
                            notHint=-1;//if top-bottom not revealed this cell is not hint

                        //check if already known
                        if(allCells[topMid]->getRevealed()==1)
                            notHint=-1;//if already known this cell is not hint

                        //check the pattern
                        if(allCells[oneLeft]->getNeighMineCount()==1 && allCells[oneRight]->getNeighMineCount()==1 && notHint==1){
                            return topMid;//if matches return the cell as hint
                        }
                    }
                }//END OF CHECK THE TOP SIDE HINT
            }
        }
    }
    return -1;
}

//CHECK 1-2-2-1 pattern, please check the documentation for the visual representation
unsigned int gameGrid::pattern1221() const{
    //check the pattern for each cell
    for(unsigned int index=0;index<(rows*columns);index++){
        unsigned int rIndex=index/columns;//calculate the row index of the cell
        unsigned int cIndex=index%columns;//calculate the column index of the cell

        //check if the cell is revealed
        if(allCells[index]->getRevealed()==1){
            //check if the cell has 2 neigh mines
            if(allCells[index]->getNeighMineCount()==2){

                //CHECK THE RIGHT SIDE HINT
                if(cIndex!=columns-1){
                    //if not a top or bottom row cell
                    if(rIndex!=0 && rIndex!=rows-1 && rIndex!=rows-2){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        int otherCase=0;//initialize otherCase, 1 means the first possible hint already known but otherCase can still be a hint
                        unsigned int oneAbove=((rIndex-1)*columns)+cIndex;//index of the cell above
                        unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                        unsigned int twoBelow=((rIndex+2)*columns)+cIndex;//index of the cell two below
                        unsigned int topRight=((rIndex-1)*columns)+cIndex+1;//first possible hint
                        unsigned int twoBelowRight=((rIndex+2)*columns)+cIndex+1;//second possible hint(i.e. otherCase)

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneAbove]->getRevealed()==0 || allCells[oneBelow]->getRevealed()==0 || allCells[twoBelow]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topRight]->getRevealed()==1){
                            otherCase=1;//set as possible otherCase
                        }

                        //check the pattern
                        if(allCells[oneAbove]->getNeighMineCount()==1 && allCells[oneBelow]->getNeighMineCount()==2 && allCells[twoBelow]->getNeighMineCount()==1 && notHint==1){
                            //case: first possible
                            if(otherCase==0)
                                return topRight;//return the first possible cell as hint
                            //case: otherCase
                            else{
                                //check if revealed
                                if(allCells[twoBelowRight]->getRevealed()==0)
                                    return twoBelowRight;//return the otherCase cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE RIGHT SIDE HINT

                //CHECK THE LEFT SIDE HINT
                if(cIndex!=0){
                    //if not a top or bottom row cell
                    if(rIndex!=0 && rIndex!=rows-1 && rIndex!=rows-2){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        int otherCase=0;//initialize otherCase, 1 means the first possible hint already known but otherCase can still be a hint
                        unsigned int oneAbove=((rIndex-1)*columns)+cIndex;//index of the cell above
                        unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                        unsigned int twoBelow=((rIndex+2)*columns)+cIndex;//index of the cell two below
                        unsigned int topLeft=((rIndex-1)*columns)+cIndex-1;//first possible hint
                        unsigned int twoBelowLeft=((rIndex+2)*columns)+cIndex-1;//second possible hint(i.e. otherCase)

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneAbove]->getRevealed()==0 || allCells[oneBelow]->getRevealed()==0 || allCells[twoBelow]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topLeft]->getRevealed()==1){
                            otherCase=1;//set as possible otherCase
                        }

                        //check the pattern
                        if(allCells[oneAbove]->getNeighMineCount()==1 && allCells[oneBelow]->getNeighMineCount()==2 && allCells[twoBelow]->getNeighMineCount()==1 && notHint==1){
                            //case: first possible
                            if(otherCase==0)
                                return topLeft;//return the first possible cell as hint
                            //case: otherCase
                            else{
                                //check if revealed
                                if(allCells[twoBelowLeft]->getRevealed()==0)
                                    return twoBelowLeft;//return the otherCase cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE LEFT SIDE HINT

                //CHECK THE BOTTOM SIDE HINT
                if(rIndex!=rows-1){
                    if(cIndex!=0 && cIndex!=columns-1){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        int otherCase=0;//initialize otherCase, 1 means the first possible hint already known but otherCase can still be a hint
                        unsigned int oneLeft=((rIndex)*columns)+cIndex-1;
                        unsigned int oneRight=((rIndex)*columns)+cIndex+1;
                        unsigned int twoRight=((rIndex)*columns)+cIndex+2;
                        unsigned int botLeft=((rIndex+1)*columns)+cIndex-1;//first possible hint
                        unsigned int twoRightBelow=((rIndex+1)*columns)+cIndex+2;//second possible hint(i.e. otherCase)

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneLeft]->getRevealed()==0 || allCells[oneRight]->getRevealed()==0 || allCells[twoRight]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[botLeft]->getRevealed()==1){
                            otherCase=1;//set as possible otherCase
                        }

                        //check the pattern
                        if(allCells[oneLeft]->getNeighMineCount()==1 && allCells[oneRight]->getNeighMineCount()==2 && allCells[twoRight]->getNeighMineCount()==1 && notHint==1){
                            //case: first possible
                            if(otherCase==0)
                                return botLeft;//return the first possible cell as hint
                            else{
                                //check if revealed
                                if(allCells[twoRightBelow]->getRevealed()==0)
                                    return twoRightBelow;//return the otherCase cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE BOTTOM SIDE HINT

                //CHECK THE TOP SIDE HINT
                if(rIndex!=0){
                    if(cIndex!=0 && cIndex!=columns-1){
                        int notHint=1;//initialize notHint, 1 means possible hint, -1 means it can`t be an hint
                        int otherCase=0;//initialize otherCase, 1 means the first possible hint already known but otherCase can still be a hint
                        unsigned int oneLeft=((rIndex)*columns)+cIndex-1;
                        unsigned int oneRight=((rIndex)*columns)+cIndex+1;
                        unsigned int twoRight=((rIndex)*columns)+cIndex+2;
                        unsigned int topLeft=((rIndex-1)*columns)+cIndex-1;//first possible hint
                        unsigned int twoRightAbove=((rIndex-1)*columns)+cIndex+2;//second possible hint(i.e. otherCase)

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneLeft]->getRevealed()==0 || allCells[oneRight]->getRevealed()==0 || allCells[twoRight]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topLeft]->getRevealed()==1){
                            otherCase=1;//set as possible otherCase
                        }

                        //check the pattern
                        if(allCells[oneLeft]->getNeighMineCount()==1 && allCells[oneRight]->getNeighMineCount()==2 && allCells[twoRight]->getNeighMineCount()==1 && notHint==1){
                            //case: first possible
                            if(otherCase==0)
                                return topLeft;//return the first possible cell as hint
                            //case: otherCase
                            else {
                                //check if revealed
                                if(allCells[twoRightAbove]->getRevealed()==0)
                                    return twoRightAbove;//return the otherCase cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE TOP SIDE HINT
            }
        }
    }
    return -1;//in case no hint found, return -1
}

//CHECK 1-1 and 1-1+ patterns
unsigned int gameGrid::pattern_One_One() const{
    //check each cell for the pattern
    for(unsigned int index=0;index<(rows*columns);index++){
        unsigned int rIndex=index/columns;//calculate the row index
        unsigned int cIndex=index%columns;//calculate the column index

        //check if the cell is revealed
        if(allCells[index]->getRevealed()==1){
            //check if the cell has 1 neigh mines
            if(allCells[index]->getNeighMineCount()==1){

                //CHECK THE RIGHT SIDE HINT - CASES 1 and 2: HINT IN THE botRight, HINT IN THE topRight
                //ALSO THERE ARE SUB_CASES OF 1-1+: checking the oneBelow(Case 1) and oneAbove(Case 2)
                if(cIndex!=columns-1){
                    //if not a top or bottom row cell
                    if(rIndex!=0 && rIndex!=rows-1){
                        int notHint=1;//to check the possibility of hint
                        int subCase=0;//to check the possibility of sub-case

                        //CHECK CASE 1: HINT IN THE botRight
                        //check the surrounding cells of the above cells
                        unsigned int oneAbove=((rIndex-1)*columns)+cIndex;//index of the cell above
                        unsigned int botRight=((rIndex+1)*columns)+cIndex+1;//main case possible hint index
                        unsigned int twoAbove=((rIndex-2)*columns)+cIndex;
                        unsigned int twoAboveLeft=((rIndex-2)*columns)+cIndex-1;
                        unsigned int twoAboveRight=((rIndex-2)*columns)+cIndex+1;

                        //checks if this is a border cell
                        if(rIndex!=1){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoAbove]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(cIndex!=0 && allCells[twoAboveLeft]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoAboveRight]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneAbove]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[botRight]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneAbove]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return botRight;//return the main case cell as hint

                            //case: sub-case
                            else {
                                unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                                //check if revealed
                                if(allCells[oneBelow]->getRevealed()==0)
                                    return oneBelow;//return the sub-case cell as hint
                            }
                        }

                        //CHECK CASE 2: HINT IN THE topRight
                        notHint=1;
                        subCase=0;
                        unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                        unsigned int topRight=((rIndex-1)*columns)+cIndex+1;//main case possible hint index
                        unsigned int twoBelow=((rIndex+2)*columns)+cIndex;
                        unsigned int twoBelowLeft=((rIndex+2)*columns)+cIndex-1;
                        unsigned int twoBelowRight=((rIndex+2)*columns)+cIndex+1;

                        if(rIndex!=rows-2){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoBelow]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(cIndex!=0 && allCells[twoBelowLeft]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoBelowRight]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneBelow]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topRight]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneBelow]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return topRight;//return the main case cell as hint
                            //case: sub-case
                            else {
                                //check if revealed
                                if(allCells[oneAbove]->getRevealed()==0)
                                    return oneAbove;//return the sub-case cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE RIGHT SIDE HINT

                //CHECK THE LEFT SIDE HINT - CASES 3 and 4: HINT IN THE botLeft, HINT IN THE topLeft
                if(cIndex!=0){
                    //if not a top or bottom row cell
                    if(rIndex!=0 && rIndex!=rows-1){
                        int notHint=1;
                        int subCase=0;
                        //CHECK CASE 3: HINT IN THE botLeft
                        //check the surrounding cells of the above cells
                        unsigned int oneAbove=((rIndex-1)*columns)+cIndex;//index of the cell above
                        unsigned int botLeft=((rIndex+1)*columns)+cIndex-1;//main case possible hint index
                        unsigned int twoAbove=((rIndex-2)*columns)+cIndex;
                        unsigned int twoAboveLeft=((rIndex-2)*columns)+cIndex-1;
                        unsigned int twoAboveRight=((rIndex-2)*columns)+cIndex+1;

                        if(rIndex!=1){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoAbove]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoAboveLeft]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(cIndex!=columns-1 && allCells[twoAboveRight]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneAbove]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[botLeft]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneAbove]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return botLeft;//return the main case cell as hint
                            //case: sub-case
                            else {
                                unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                                //check if revealed
                                if(allCells[oneBelow]->getRevealed()==0)
                                    return oneBelow;//return the sub-case cell as hint
                            }
                        }

                        //CHECK CASE 4: HINT IN THE topLeft
                        notHint=1;
                        subCase=0;
                        unsigned int oneBelow=((rIndex+1)*columns)+cIndex;//index of the cell below
                        unsigned int topLeft=((rIndex-1)*columns)+cIndex-1;//main case possible hint index
                        unsigned int twoBelow=((rIndex+2)*columns)+cIndex;
                        unsigned int twoBelowLeft=((rIndex+2)*columns)+cIndex-1;
                        unsigned int twoBelowRight=((rIndex+2)*columns)+cIndex+1;

                        if(rIndex!=rows-2){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoBelow]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoBelowLeft]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(cIndex!=columns-1 && allCells[twoBelowRight]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneBelow]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topLeft]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneBelow]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return topLeft;//return the main case cell as hint
                            //case: sub-case
                            else {
                                //check if revealed
                                if(allCells[oneAbove]->getRevealed()==0)
                                    return oneAbove;//return the sub-case cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE LEFT SIDE HINT

                //CHECK THE BOTTOM SIDE HINT - CASES 5 and 6: HINT IN THE botRight, HINT IN THE botLeft
                if(rIndex!=rows-1){
                    //if not a top or bottom column cell
                    if(cIndex!=0 && cIndex!=columns-1){
                        int notHint=1;
                        int subCase=0;
                        //CHECK CASE 5: HINT IN THE botRight
                        //check the surrounding cells of the above cells
                        unsigned int oneLeft=((rIndex)*columns)+cIndex-1;//index of the cell left
                        unsigned int botRight=((rIndex+1)*columns)+cIndex+1;//main case possible hint index
                        unsigned int twoLeft=((rIndex)*columns)+cIndex-2;
                        unsigned int twoLeftAbove=((rIndex-1)*columns)+cIndex-2;
                        unsigned int twoLeftBelow=((rIndex+1)*columns)+cIndex-2;

                        if(cIndex!=1){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoLeft]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(rIndex!=0 && allCells[twoLeftAbove]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoLeftBelow]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneLeft]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[botRight]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the values
                        if(allCells[oneLeft]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return botRight;//return the main case cell as hint
                            //case: sub-case
                            else {
                                unsigned int oneRight=((rIndex)*columns)+cIndex+1;//index of the cell right
                                //check if revealed
                                if(allCells[oneRight]->getRevealed()==0)
                                    return oneRight;//return the sub-case cell as hint

                            }
                        }

                        //CHECK CASE 6: HINT IN THE botLeft
                        notHint=1;
                        subCase=0;
                        unsigned int oneRight=((rIndex)*columns)+cIndex+1;//index of the cell right
                        unsigned int botLeft=((rIndex+1)*columns)+cIndex-1;//main case possible hint index
                        unsigned int twoRight=((rIndex)*columns)+cIndex+2;
                        unsigned int twoRightAbove=((rIndex-1)*columns)+cIndex+2;
                        unsigned int twoRightBelow=((rIndex+1)*columns)+cIndex+2;

                        if(cIndex!=columns-2){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoRight]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(rIndex!=0 && allCells[twoRightAbove]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoRightBelow]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneRight]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[botLeft]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneRight]->getNeighMineCount()==1 && notHint==1){
                            if(subCase==0)
                                return botLeft;//return the main case cell as hint

                            else {
                                if(allCells[oneLeft]->getRevealed()==0)
                                    return oneLeft;//return the sub-case cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE BOTTOM SIDE HINT

                //CHECK THE TOP SIDE HINT - CASES 7 and 8: HINT IN THE topRight, HINT IN THE topLeft
                if(rIndex!=0){
                    //if not a top or bottom column cell
                    if(cIndex!=0 && cIndex!=columns-1){
                        int notHint=1;
                        int subCase=0;
                        //CHECK CASE 7: HINT IN THE topRight
                        //check the surrounding cells of the above cells
                        unsigned int oneLeft=((rIndex)*columns)+cIndex-1;//index of the cell left
                        unsigned int topRight=((rIndex-1)*columns)+cIndex+1;//main case possible hint index
                        unsigned int twoLeft=((rIndex)*columns)+cIndex-2;
                        unsigned int twoLeftAbove=((rIndex-1)*columns)+cIndex-2;
                        unsigned int twoLeftBelow=((rIndex+1)*columns)+cIndex-2;

                        if(cIndex!=1){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoLeft]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoLeftAbove]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(rIndex!=rows-1 && allCells[twoLeftBelow]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneLeft]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topRight]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneLeft]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return topRight; //return the main case cell as hint
                            //case: sub-case
                            else {
                                unsigned int oneRight=((rIndex)*columns)+cIndex+1;//index of the cell right
                                //check if revealed
                                if(allCells[oneRight]->getRevealed()==0)
                                    return oneRight;//return the sub-case cell as hint
                            }
                        }

                        //CHECK CASE 8: HINT IN THE topLeft
                        notHint=1;
                        subCase=0;
                        unsigned int oneRight=((rIndex)*columns)+cIndex+1;//index of the cell right
                        unsigned int topLeft=((rIndex-1)*columns)+cIndex-1;//main case possible hint index
                        unsigned int twoRight=((rIndex)*columns)+cIndex+2;
                        unsigned int twoRightAbove=((rIndex-1)*columns)+cIndex+2;
                        unsigned int twoRightBelow=((rIndex+1)*columns)+cIndex+2;

                        if(cIndex!=columns-2){
                            //check the existing information for neighbour cells needs to be revealed
                            if(allCells[twoRight]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(allCells[twoRightAbove]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                            if(rIndex!=rows-1 && allCells[twoRightBelow]->getRevealed()==0)
                                notHint=-1;//if required cells not revealed this cell is not hint
                        }

                        //check the existing information for neighbour cells needs to be revealed
                        if(allCells[oneRight]->getRevealed()==0)
                            notHint=-1;//if required cells not revealed this cell is not hint

                        //check if already known
                        if(allCells[topLeft]->getRevealed()==1){
                            subCase=1;//set as possible subCase
                        }

                        //check the pattern
                        if(allCells[oneRight]->getNeighMineCount()==1 && notHint==1){
                            //case: main case
                            if(subCase==0)
                                return topLeft;//return the main case cell as hint
                            //case: sub-case
                            else {
                                //check if revealed
                                if(allCells[oneLeft]->getRevealed()==0)
                                    return oneLeft;//return the sub-case cell as hint
                            }
                        }
                    }
                }//END OF CHECK THE BOTTOM SIDE HINT
            }
        }
    }
    return -1;
}

//SLOTS
//show all mines in case of end game
void gameGrid::revealAllMines() {
    //check all rows and cells
    for (unsigned int i=0;i<rows;i++) {
        for (unsigned int j=0;j<columns;j++) {
            allCells[i*columns+j]->setEnabled(false);//disable all cell interactions
            //case: if the cell has a mine
            if(allCells[i*columns+j]->getMine()==1){
                QIcon icon = QIcon(QPixmap(":/images/mine.png").scaled(QSize(23, 23)));//get mine icon
                allCells[i*columns+j]->setIcon(icon);//show mine icon in the cell
            }
        }
    }
}

//game lost dialog screen
void gameGrid::loseDialog(){
    QDialog youLose; //create the dialog object
    youLose.setWindowTitle("You lose!"); //set the dialog title

    QLabel messageLabel("You lose!"); //create the message label object
    QPushButton ok("OK");//create the OK buttn

    QVBoxLayout loseLayout;//create the box layout
    loseLayout.addWidget(&messageLabel);// add message label
    loseLayout.addWidget(&ok);//add ok button
    youLose.setLayout(&loseLayout);//set the layout of the dialog

    emit closeHint();// close the hint button
    QObject::connect(&ok, &QPushButton::clicked, &youLose, &QDialog::close);//after ok button is clicked, dialog closes

    youLose.exec();
}

//game won dialog screen
void gameGrid::winDialog(){
    QDialog youWin; //create the dialog object
    youWin.setWindowTitle("You win!"); //set the dialog title

    QLabel messageLabel("You win!");//create the message label object
    QPushButton ok("OK");//add ok button

    QVBoxLayout winLayout;//create the box layout
    winLayout.addWidget(&messageLabel);// add message label
    winLayout.addWidget(&ok);//add ok button
    youWin.setLayout(&winLayout);//set the layout of the dialog

    emit closeHint();// close the hint button
    QObject::connect(&ok, &QPushButton::clicked, &youWin, &QDialog::close);//after ok button is clicked, dialog closes

    youWin.exec();
}

//perform the recursion
void gameGrid::recursionCase() {
    std::set<int> checked;//this set stores the unique cell indexes to make sure not include duplicates to recursion
    checked.insert(globalStack.top());//add the top of the stck to the checked set
    while(!globalStack.isEmpty()){//while recursion stack is not empty
        unsigned int index=globalStack.pop();//pop the top item
        unsigned int rIndex=index/columns;//get the row index
        unsigned int cIndex=index%columns;//get the column index

        //case: not first row, check the cells on the top
        if(rIndex!=0){
            unsigned int topMid=((rIndex-1)*columns)+cIndex;//get the top mid index
            auto it=checked.find(topMid);//check if the cell is already checked for this recursion case
            if(it==checked.end() && allCells[topMid]->getRevealed()!=1){
                this->checkCell(topMid);//check the cell
                checked.insert(topMid);//insert the cell to the checked list
            }

            //case: not first column, check the cell on the left
            if(cIndex!=0){
                unsigned int topLeft=((rIndex-1)*columns)+cIndex-1;//get the left mid index
                auto it=checked.find(topLeft);//check if the cell is already checked for this recursion case
                if(it==checked.end() && allCells[topLeft]->getRevealed()!=1){
                    this->checkCell(topLeft);//check the cell
                    checked.insert(topLeft);//insert the cell to the checked list
                }
            }

            //case: not last column, check the cell on the right
            if(cIndex!=columns-1){
                unsigned int topRight=((rIndex-1)*columns)+cIndex+1; //get the top right index
                auto it=checked.find(topRight);//check if the cell is already checked for this recursion case
                if(it==checked.end() && allCells[topRight]->getRevealed()!=1){
                    this->checkCell(topRight);//check the cell
                    checked.insert(topRight);//insert the cell to the checked list
                }
            }
        }

        //case: not last row
        if(rIndex!=rows-1){
            unsigned int botMid=((rIndex+1)*columns)+cIndex;//get the bot mid index
            auto it=checked.find(botMid);//check if the cell is already checked for this recursion case
            if(it==checked.end() && allCells[botMid]->getRevealed()!=1){
                this->checkCell(botMid);//check the cell
                checked.insert(botMid);//insert the cell to the checked list
            }
            if(cIndex!=0){
                unsigned int botLeft=((rIndex+1)*columns)+cIndex-1;//get the bot left index
                auto it=checked.find(botLeft);//check if the cell is already checked for this recursion case
                if(it==checked.end() && allCells[botLeft]->getRevealed()!=1){
                    this->checkCell(botLeft);//check the cell
                    checked.insert(botLeft);//insert the cell to the checked list
                }
            }
            if(cIndex!=columns-1){
                unsigned int botRight=((rIndex+1)*columns)+cIndex+1;//get the bot right index
                auto it=checked.find(botRight);//check if the cell is already checked for this recursion case
                if(it==checked.end() && allCells[botRight]->getRevealed()!=1){
                    this->checkCell(botRight);//check the cell
                    checked.insert(botRight);//insert the cell to the checked list
                }
            }
        }

        if(cIndex!=0){
            unsigned int sameLeft=((rIndex)*columns)+cIndex-1;//get the same left index
            auto it=checked.find(sameLeft);//check if the cell is already checked for this recursion case
            if(it==checked.end() && allCells[sameLeft]->getRevealed()!=1){
                this->checkCell(sameLeft);//check the cell
                checked.insert(sameLeft);//insert the cell to the checked list
            }
        }

        if(cIndex!=columns-1){
            unsigned int sameRight=((rIndex)*columns)+cIndex+1;//get the same right index
            auto it=checked.find(sameRight);//check if the cell is already checked for this recursion case
            if(it==checked.end() && allCells[sameRight]->getRevealed()!=1){
                this->checkCell(sameRight);//check the cell
                checked.insert(sameRight);//insert the cell to the checked list
            }
        }
    }
}

//function that checks the cells during the recursion
void gameGrid::checkCell(unsigned int index){
    unsigned int isMine=allCells[index]->getMine();//get if the cell is mine or not
    unsigned int mineCount=allCells[index]->getNeighMineCount();// get the neighbour mine total value

    //case: the cell is mine
    if(isMine==1)
        return;//end check, not a recursion element

    //case: the cell is not mine and has no neighbour mines
    //recursion element
    else if(mineCount==0){
        globalStack.push(index);//push the cell to stack, to chekc it`s neighbours
        emit allCells[index]->showEmpty();//call the show empty function
        emit indirectHintClose();//if there is an active hint in the game, close it
    }

    //case: the cell is not mine and has neighbour mines
    else if(mineCount!=0){
        emit allCells[index]->showNumber();//call the show number function
        emit indirectHintClose();//if there is an active hint in the game, close it
    }
}

//decrease the number of remaining cells
void gameGrid::decreaseCounter(){
    this->remaining=this->remaining-1;//decrease the remaining mine number by 1

    emit scoreUpdate();// update the score
    if(this->remaining==0)//check if game won, if there is no more remainin mine
        emit youWin();//emit the youWin signal
}
