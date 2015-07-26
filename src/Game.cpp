// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#include "Game.hpp"

Game::Game(SoundFX* sfx, QLabel* label)
: soundfx(sfx)
, status_label(label)
, gameover(false)
, highlightedPiece(NULL)
, highlightx(-1)
, highlightz(-1)
, currentPlayer(2)
, selectedx(-1)
, selectedz(-1)
, selectedPiece(NULL)
, loser(0)
, winner(0)
{

    boardTiles = new std::vector<Piece*>();
    gamePieces = new std::vector<GamePiece*>();

    // Create the tiles
    for (int row=0; row<8; row++){
    	for (int col=0; col<10; col++){
    		Piece* t = new Piece("TILE", col, row);
            tilesGrid[col][row] = t;
    		boardTiles->push_back(t);
    	}
    }

    GamePiece* p;
    // Create blockers
    p = new GamePiece("BLOCKER", 4, 0, 0, 1);
    gamePieces->push_back(p);
    p = new GamePiece("BLOCKER", 6, 0, 0, 1);
    gamePieces->push_back(p);
    p = new GamePiece("BLOCKER", 3, 7, 0, 2);
    gamePieces->push_back(p);
    p = new GamePiece("BLOCKER", 5, 7, 0, 2);
    gamePieces->push_back(p);

    // Create TWOWAYs
    p = new GamePiece("TWOWAY", 4, 3, 0, 1);
    gamePieces->push_back(p);
    p = new GamePiece("TWOWAY", 5, 3, 90, 1);
    gamePieces->push_back(p);
    p = new GamePiece("TWOWAY", 4, 4, 270, 2);
    gamePieces->push_back(p);
    p = new GamePiece("TWOWAY", 5, 4, 180, 2);
    gamePieces->push_back(p);

    // Create pawns
    p = new GamePiece("PAWN", 7, 0, 270, 1);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 2, 1, 180, 1);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 0, 3, 0, 1);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 7, 3, 270, 1);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 0, 4, 270, 1);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 7, 4, 0, 1);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 6, 5, 270, 1);
    gamePieces->push_back(p);

    p = new GamePiece("PAWN", 2, 7, 90, 2);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 7, 6, 0, 2);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 2, 4, 90, 2);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 9, 4, 180, 2);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 2, 3, 180, 2);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 9, 3, 90, 2);
    gamePieces->push_back(p);
    p = new GamePiece("PAWN", 3, 2, 90, 2);
    gamePieces->push_back(p);

    // Create kings
    p = new GamePiece("KING", 5, 0, 0, 1);
    gamePieces->push_back(p);
    p = new GamePiece("KING", 4, 7, 0, 2);
    gamePieces->push_back(p);

    // Create lasers
    p = new GamePiece("LASERGUN", 0, 0, 180, 1);
    gamePieces->push_back(p);
    p = new GamePiece("LASERGUN", 9, 7, 0, 2);
    gamePieces->push_back(p);

    // Init the pieces grid
    initPiecesGrid();
    resetHighlight();

    // Init the BVH tree
    bvhtree = new BVHTree();
    bvhtree->fillPieces(gamePieces);
}

Game::~Game(){
    for (int i=0; i<boardTiles->size(); i++){
        delete (*boardTiles)[i];
    }
    delete boardTiles;
    for (int i=0; i<gamePieces->size(); i++){
        delete (*gamePieces)[i];
    }
    delete gamePieces;
    delete bvhtree;
}

void Game::initPiecesGrid(){
    for (int i=0; i<10; i++){
        for (int j=0; j<8; j++){
            piecesGrid[i][j] = NULL;
        }
    }

    for (int i=0; i<gamePieces->size(); i++){
        GamePiece* p = (*gamePieces)[i];
        piecesGrid[p->boardx][p->boardz] = p; 
    }
}

void Game::reset(){
    gameover = false;
    currentPlayer = 2;
	for (int i=0; i<gamePieces->size(); i++){
		(*gamePieces)[i]->reset();
	}
    initPiecesGrid();
    resetHighlight();
    deselectPiece();
    loser = 0;
    winner = 0;
    laserPath1.clear();
    laserPath2.clear();
    bvhtree->clearPieces();
    bvhtree->fillPieces(gamePieces);
    std::cerr << "Reset the game\n";
}

void Game::switchTurn(){
    currentPlayer = (currentPlayer%2)+1;
    std::string playerStr;
    if (currentPlayer == 1){
        playerStr = " (Grey) ";
    } else {
        playerStr = " (Red) ";
    }
    std::stringstream ss;
    ss << "Player " << currentPlayer << playerStr << "'s turn";
    QString qstr = QString::fromStdString(ss.str());
    //status_label->setText(qstr);
}

void Game::resetHighlight(){
    highlightPiece(4,3);
}

void Game::moveHighlight(int dx, int dz){
    int new_hx = highlightx+dx;
    int new_hz = highlightz+dz;
    if ((0<=new_hx) && (new_hx<10) && (0<=new_hz) && (new_hz<8)){
        highlightPiece(new_hx, new_hz);
    }
}

void Game::highlightPiece(int x, int z){
    if (highlightedPiece != NULL){
        highlightedPiece->isHighlighted = false;
    }

    if (piecesGrid[x][z] != NULL && piecesGrid[x][z]->isVisible){
        highlightedPiece = piecesGrid[x][z];
    }
    else {
        highlightedPiece = tilesGrid[x][z];
    }

    highlightedPiece->isHighlighted = true;
    highlightx = x;
    highlightz = z;
    highlightCentreWorld = highlightedPiece->getCentreWorld();
}

bool Game::selectHighlightedPiece(){
    GamePiece* highlightedPiece = piecesGrid[highlightx][highlightz];
    if (currentPlayer != highlightedPiece->player){
        std::cerr << "not your piece to select\n";
        status_label->setText("Invalid selection: not your piece!");
        return false;
    }

    selectedx = highlightx;
    selectedz = highlightz;
    resetHighlight();
    selectedPiece = piecesGrid[selectedx][selectedz];
    selectedPiece->isSelected = true;
    return true;
}

void Game::deselectPiece(){
    if (selectedPiece != NULL) {
        selectedPiece->isSelected = false;
        selectedPiece = NULL;
    }
    selectedx = -1;
    selectedz = -1;
}

bool Game::rotateSelectedPiece(int dir, bool animate){
    selectedPiece->saveInitialPosition();

    std::stringstream ss;

    float curRot = selectedPiece->rotation;
    float theta, newRot;
    std::string dirStr;

    if (dir >= 0){ // +ve -> CW
        theta = -90;
        dirStr = " clockwise";
    } else {
        theta = 90; // -ve -> CCW
        dirStr = " counter-clockwise";
    }
    newRot = curRot+theta;

    // Restrict the rotation of the laser gun
    // 0<->90, or 180<->270
    if ((selectedPiece->type == "LASERGUN")
            && !((curRot == 0 && newRot == 90)
                || (curRot == 90 && newRot == 0)
                || (curRot == 180 && newRot == 270)
                || (curRot == 270 && newRot == 180))) {
        status_label->setText("Invalid rotation: laser gun can't rotate that way!");
        highlightPiece(selectedx, selectedz);
        deselectPiece();
        return false;
    }

    selectedPiece->modelRotateXZ(theta);

    soundfx->playSound("movepiece");

    if (animate){
        selectedPiece->startAnimation(QVector3D(0,0,0), theta, 50);
    }
    ss << "Rotated " << selectedPiece->type << dirStr;
    QString qstr = QString::fromStdString(ss.str());
    status_label->setText(qstr);
    deselectPiece();
    return true;
}

bool Game::moveSelectedPiece(int dx, int dz, bool animate){
    selectedPiece->saveInitialPosition();

    std::stringstream ss;
    GamePiece* swapped = NULL;

    int destx = selectedx + dx;
    int destz = selectedz + dz;

    if ((destx<0)||(destx>=10)||(destz<0)||(destz>=8)){
        status_label->setText("Invalid move: off of board!");
        highlightPiece(selectedx, selectedz);
        deselectPiece();
        return false;
    }

    // PAWN
    if (selectedPiece->type == "PAWN"){

        if (piecesGrid[destx][destz] != NULL){
            status_label->setText("Invalid move: off of board!");
            highlightPiece(selectedx, selectedz);
            deselectPiece();
            return false;
        }
        selectedPiece->moveUnitXZ(dx, dz);

    }
    // TWOWAY
    else if(selectedPiece->type == "TWOWAY"){

        GamePiece* neighbour = piecesGrid[destx][destz];
        // If not empty, switch
        // Animate the other piece too
        if (neighbour != NULL){
            neighbour->saveInitialPosition();
            neighbour->moveUnitXZ(-1*dx, -1*dz);
            if (animate){
                neighbour->startAnimation(QVector3D(-1*dx, 0, -1*dz), 0, 50);
            }
            swapped = neighbour;
        }
        selectedPiece->moveUnitXZ(dx, dz);

    }
    // BLOCKER
    else if(selectedPiece->type == "BLOCKER"){

        if (piecesGrid[destx][destz] != NULL){
            status_label->setText("Invalid move: off of board!");
            highlightPiece(selectedx, selectedz);
            deselectPiece();
            return false;
        }
        selectedPiece->moveUnitXZ(dx, dz);

    }
    // LASER GUN
    else if(selectedPiece->type == "LASERGUN"){

        status_label->setText("Invalid move: laser gun can't move!");
        highlightPiece(selectedx, selectedz);
        deselectPiece();
        return false;

    }
    // KING
    else if(selectedPiece->type == "KING"){

        if (piecesGrid[destx][destz] != NULL){
            status_label->setText("Invalid move: off of board!");
            highlightPiece(selectedx, selectedz);
            deselectPiece();
            return false;
        }
        selectedPiece->moveUnitXZ(dx, dz);

    }

    soundfx->playSound("movepiece");

    if (animate){
        selectedPiece->startAnimation(QVector3D(dx,0,dz), 0.0, 50);
    }

    piecesGrid[destx][destz] = selectedPiece;
    piecesGrid[selectedx][selectedz] = swapped;
    bvhtree->removePiece(selectedPiece);
    if (swapped != NULL) bvhtree->removePiece(swapped);
    bvhtree->insertPiece(selectedPiece);
    if (swapped != NULL) bvhtree->insertPiece(swapped);

    ss << "Moved " << selectedPiece->type << " to tile (" << destx+1 << "," << destz+1 << ")";
    QString qstr = QString::fromStdString(ss.str());
    status_label->setText(qstr);
    deselectPiece();
    //std::cerr << "Moved piece\n";
    return true;
}

void Game::fireLaser(int player){
    soundfx->playSound("lasershot");
    if (player == 1) {
        fireLaserBase(player, laserPath1);
    } else if (player == 2){
        fireLaserBase(player, laserPath2);
    }
    switchTurn();
}

void Game::fireLaserBase(int player, std::vector<std::pair<int,int>>& path){

    //Erase old path first
    path.clear();

    int curx, curz, diff;
    GamePiece* currentPiece;
    if (player == 1) {
        curx = 0;
        curz = 0;
    } else if (player == 2){
        curx = 9;
        curz = 7;
    }

    std::pair<int,int> ray;
    bool hit = false;
    bool offTheEdge = false;

    // init reflected array
    currentPiece = piecesGrid[curx][curz];
    path.push_back(std::make_pair(curx, curz));
    ray = currentPiece->getReflectedDirection(std::make_pair(0,0), hit);
    //std::cerr << "FIRST RAY " << ray.first << " " << ray.second << std::endl;

    do {
        //std::cerr << "FIRING\n";
        int edge, step, bx, bz;
        //std::cerr << "curx " << curx << " curz " << curz << std::endl;
        //std::cerr << "testing ray " << ray.first << " " << ray.second << std::endl;

        if (ray.first == 0){
            //std::cerr << "testing z direction\n";
            step = ray.second;
            edge = (step < 0) ? -1 : 8;

            for(bz=curz+step; bz!=edge; bz+=step){

                currentPiece = piecesGrid[curx][bz];
                //std::cerr << " x " << curx << " z " << bz << std::endl;

                if (currentPiece != NULL && currentPiece->isVisible){
                    //std::cerr << "breaking z\n";
                    ray = currentPiece->getReflectedDirection(ray, hit);
                    path.push_back(std::make_pair(curx, bz));
                    curz = bz;
                    break;
                }
                // If we made it this far we ran off an edge
                if ((bz+step) == edge){
                    path.push_back(std::make_pair(curx,edge));
                    offTheEdge = true;
                }
            }

        } else {
            //std::cerr << "testing x direction\n";
            step = ray.first;
            edge = (step < 0) ? -1 :10;

            for(bx=curx+step; bx!=edge; bx+=step){

                currentPiece = piecesGrid[bx][curz];
                //std::cerr << " x " << bx << " z " << curz << std::endl;

                if (currentPiece != NULL && currentPiece->isVisible){
                    //std::cerr << "breaking x\n";
                    ray = currentPiece->getReflectedDirection(ray, hit);
                    path.push_back(std::make_pair(bx, curz));
                    curx = bx;
                    break;
                }
                // If we made it this far we ran off an edge
                if ((bx+step)==edge){
                    path.push_back(std::make_pair(edge, curz));
                    offTheEdge = true;
                }
            }
        }

        //printLaserPathBase(player, path);

        //if (hit) std::cerr << "Beam hit a piece!\n";
        //if (offTheEdge) std::cerr << "Beam went off the edge!\n";

    } while (!hit && !offTheEdge);
    //printLaserPathBase(player, path);

    // Eliminate a piece and determine if game is still in play
    if (hit && currentPiece->type == "BLOCKER"){
        status_label->setText("Laser beam blocked!");
    }
    else if (hit && currentPiece->type != "BLOCKER"){

        status_label->setText("Piece eliminated!");
        currentPiece->isVisible = false;
        currentPiece->isHighlighted = false;
        soundfx->playSound("explosion");

        if (currentPiece->type == "KING"){
            std::cerr << "THE KING WAS HIT!!\n";
            gameover = true;
            loser = currentPiece->player;
            winner = (loser%2)+1;

            std::string loserStr, winnerStr;
            if (winner == 1){
                winnerStr = " (Grey) ";
                loserStr = " (Red) ";
            } else {
                loserStr = " (Grey) ";
                winnerStr = " (Red) ";
            }

            std::stringstream ss;
            ss << "Player " << winner << winnerStr << " won! ";
            ss << "Player " << loser << loserStr << " lost!";
            QString qstr = QString::fromStdString(ss.str());
            status_label->setText(qstr);
        }
    }

}

void Game::printLaserPath(int player){
    if (player == 1) {
        printLaserPathBase(player, laserPath1);
    } else if (player == 2){
        printLaserPathBase(player, laserPath2);
    }
}

void Game::printLaserPathBase(int player, std::vector<std::pair<int,int>>& path ){
    std::cerr << "Laser path " << player << std::endl;
    for (int i=0; i<path.size(); i++){
        std::cerr << " i=" << i+1 << ", x=" << path[i].first << " z=" << path[i].second << std::endl;
    }
    std::cerr << std::endl;
}

QVector<QVector3D> Game::getWorldCoordsLaserPath(int player){
    std::vector< std::pair<int,int> > path;
    if (player == 1){
        path = laserPath1;
    } else if (player == 2){
        path = laserPath2;
    }

    QVector<QVector3D> pathWorld;
    int boardx, boardz;

    for (int i=0; i<path.size(); i++){
        boardx = (path[i]).first;
        boardz = (path[i]).second;
        QVector3D result;
        result = Piece::boardToWorldCoordinates(boardx, boardz) + QVector3D(0.5,0,0.5);//QVector3D(boardx, 0.5, boardz) - QVector3D(8.5, 0, 6.5);
        pathWorld << result;
    }
    return pathWorld;
}
