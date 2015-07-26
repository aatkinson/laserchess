// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#include <QtWidgets>
#include <QGLFormat>
#include <iostream>
#include "AppWindow.hpp"

AppWindow::AppWindow()
{
    setWindowTitle("LASER CHESS");

    QGLFormat glFormat;
    glFormat.setVersion(3,3);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    QVBoxLayout *layout = new QVBoxLayout;
    // m_menubar = new QMenuBar;

    // Add the status box
    status_label = new QLabel("<b>Welcome to LASER CHESS!</b>");
    status_label->setAlignment(Qt::AlignCenter);
    QDockWidget *dockWidget = new QDockWidget(this);
    dockWidget->setFloating(false);
    dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockWidget->setWidget(status_label);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    turn_label = new QLabel("TURN LABEL");
    turn_label->setAlignment(Qt::AlignCenter);
    QDockWidget *dockWidget2 = new QDockWidget(this);
    dockWidget2->setFloating(false);
    dockWidget2->setAllowedAreas(Qt::TopDockWidgetArea);
    dockWidget2->setWidget(turn_label);
    addDockWidget(Qt::TopDockWidgetArea, dockWidget2);

    QLabel* helpLabel = new QLabel("<b>Press 'H' for Help</b>");
    helpLabel->setAlignment(Qt::AlignCenter);
    QDockWidget *dockWidget3 = new QDockWidget(this);
    dockWidget3->setFloating(false);
    dockWidget3->setAllowedAreas(Qt::TopDockWidgetArea);
    dockWidget3->setWidget(helpLabel);
    addDockWidget(Qt::TopDockWidgetArea, dockWidget3);

    mode_label = new QLabel("MODE LABEL");
    mode_label->setAlignment(Qt::AlignCenter);
    QDockWidget *dockWidget4 = new QDockWidget(this);
    dockWidget4->setFloating(false);
    dockWidget4->setAllowedAreas(Qt::TopDockWidgetArea);
    dockWidget4->setWidget(mode_label);
    addDockWidget(Qt::TopDockWidgetArea, dockWidget4);

    // Create dialogs

    QString helpText;
    helpText.append("To see game rules, press the 'R' key.\n");
    helpText.append("\nView Controls:\n");
    helpText.append("- Move the camera left & right by clicking the left mouse button and dragging the cursor.\n");
    helpText.append("- Move the camera up & down by clicking the right mouse button and dragging the cursor.\n");
    helpText.append("- Zoom the camera in & out by clicking the middle mouse button and dragging the cursor.\n");
    helpText.append("- To reset the view to the current player, press the 'F' key.\n");
    helpText.append("\nPiece Controls and Gameplay:\n");
    helpText.append("- Move the yellow highlight square using the arrowKeys.\n");
    helpText.append("- Press 'Enter' to select a highlighted game piece, and the piece will turn green.\n");
    helpText.append("- Press an arrow key to move the piece to an adjacent square\n");
    helpText.append("- Press the 'C' key to rotate the piece clockwise\n");
    helpText.append("- Press the 'W' key to rotate the piece counter-clockwise\n");
    helpText.append("- Once your move piece has moved, press 'Enter' to fire your laser.\n");
    helpText.append("- Press 'Enter' to switch the players' turn.\n");
    helpText.append("- Press the 'N' key to start a new game.\n");
    helpText.append("\nMode Controls:\n");
    helpText.append("- Press 'P' to toggle Perlin noise textures.\n");
    helpText.append("- Press 'T' to toggle mapped textures.\n");
    helpText.append("- Press 'B' to toggle viewing the bounded volumes of the game pieces.\n");    

    help_dialog = new QMessageBox(this);
    help_dialog->setWindowTitle("Laser Chess - Help");
    help_dialog->setText(helpText);

    QString rulesText;
    rulesText.append("The goal of the game is to move your reflective pieces so you can 'zap' your opponent's king piece with a laser beam.\n");
    rulesText.append("\nPlayers take turns, during with they can move one of their pieces one square left, right, up, or down, or rotate one of their pieces clockwise or counter-clockwise by 90 degrees.\n");
    rulesText.append("After moving a piece, the player must activate their laser to finish their turn\n");
    rulesText.append("\nThe laser beam bounces off of the reflective surfaces of pieces, and eliminates a piece when it hits a non-reflective surface.\n");
    rulesText.append("\nEach piece has different abilities:\n\n");
    rulesText.append("- Pawn (tetrahedron): can reflect a laser in one direction, eliminated when hit on the backside, and can move U/D/L/R and rotate.\n\n");
    rulesText.append("- Two-Way (angled wall): can reflect a beam from any direction and can't be eliminated. Can move U/D/L/R and rotate.\n");
    rulesText.append("  The two way piece can also swap with any adjacent piece.\n\n");
    rulesText.append("- Blocker (block): blocks a laser from any direction, can't be eliminated, and can move U/D/L/R and rotate.\n\n");
    rulesText.append("- Laser (cylinder): cannot be eliminated, emits the laser beam, and can only be rotated such that it shoots a laser across the board.\n\n");
    rulesText.append("- King (sphere): cannot reflect any lasers - when it is hit, the game ends. Can move U/D/L/R and rotate.\n\n");
    rules_dialog = new QMessageBox(this);
    rules_dialog->setWindowTitle("Laser Chess - Rules");
    rules_dialog->setText(rulesText);

    gameover_dialog = new QMessageBox(this);
    gameover_dialog->setWindowTitle("Laser Chess - Game Over");

    // Create an instance of the sound fx class and load music
    m_soundfx = new SoundFX();
    m_soundfx->initSDL();
    m_soundfx->loadFile("../sounds/explosion.wav", "explosion");
    m_soundfx->loadFile("../sounds/lasershot.wav", "lasershot");
    m_soundfx->loadFile("../sounds/rockscrape.wav", "movepiece");
    m_soundfx->loadFile("../sounds/gameover.wav", "gameover");

    //Create the game instance
    m_game = new Game(m_soundfx, status_label);

    //Create the viewer
    m_viewer = new Viewer(glFormat, m_game, this);
    layout->addWidget(m_viewer);
    setCentralWidget(new QWidget);
    centralWidget()->setLayout(layout);
    m_viewer->show();

    createMenu();

    arrowKeys += Qt::Key_Up;
    arrowKeys += Qt::Key_Down;
    arrowKeys += Qt::Key_Left;
    arrowKeys += Qt::Key_Right;

    toggleHighlight();
    updateModeLabel();
    updateTurnLabel();
}

AppWindow::~AppWindow() {
    delete m_game;
    delete m_soundfx;
}

void AppWindow::keyPressEvent(QKeyEvent *event) {
    int key = event->key();

    switch (key) {
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;
        case Qt::Key_P:
            modeActions[0]->trigger();
            //togglePerlinDisplay();
            break;
        case Qt::Key_T:
            modeActions[1]->trigger();
            //toggleOtherTextureDisplay();
            break;
        case Qt::Key_B:
            modeActions[2]->trigger();
            //toggleBVHDisplay();
            break;
        case Qt::Key_H:
            helpActions[0]->trigger();
            //showHelpDialog();
            break;
        case Qt::Key_R:
            helpActions[1]->trigger();
            //showRulesDialog();
            break;
        case Qt::Key_F:
            flipCamera();
            break;
        case Qt::Key_N:
            applicationActions[1]->trigger();
            // resetGame();
            break;
        case Qt::Key_Enter:
            if (mode == AppMode::HIGHLIGHT_MODE){
                bool success = m_game->selectHighlightedPiece();
                if (success) {
                    mode = AppMode::SELECTED_MODE;
                    m_viewer->mode = Viewer::SELECTED_MODE;
                    //std::cerr << "selected a piece\n";
                }
            }
            else if (mode == AppMode::FIRE_LASER_MODE){
                m_game->fireLaser(m_game->currentPlayer);
                m_viewer->updateLaserData((m_game->currentPlayer%2)+1);
                mode = AppMode::SHOW_LASER_MODE;
                m_viewer->mode = Viewer::SHOW_LASER_MODE;
            }
            else if (mode == AppMode::SHOW_LASER_MODE){
                mode = AppMode::HIGHLIGHT_MODE;
                m_viewer->mode = Viewer::HIGHLIGHT_MODE;
                m_viewer->updateBVHData();
                updateTurnLabel();
                if (m_game->gameover){
                    mode = AppMode::GAME_OVER;
                    m_viewer->mode = Viewer::REGULAR_MODE;
                    showGameOverDialog();
                }
            }
            break;
        case Qt::Key_Return:
            if (mode == AppMode::HIGHLIGHT_MODE){
                bool success = m_game->selectHighlightedPiece();
                if (success) {
                    mode = AppMode::SELECTED_MODE;
                    m_viewer->mode = Viewer::SELECTED_MODE;
                    //std::cerr << "selected a piece\n";
                }
            }
            else if (mode == AppMode::FIRE_LASER_MODE){
                m_game->fireLaser(m_game->currentPlayer);
                m_viewer->updateLaserData((m_game->currentPlayer%2)+1);
                mode = AppMode::SHOW_LASER_MODE;
                m_viewer->mode = Viewer::SHOW_LASER_MODE;
            }
            else if (mode == AppMode::SHOW_LASER_MODE){
                mode = AppMode::HIGHLIGHT_MODE;
                m_viewer->mode = Viewer::HIGHLIGHT_MODE;
                m_viewer->updateBVHData();
                updateTurnLabel();
                if (m_game->gameover){
                    mode = AppMode::GAME_OVER;
                    m_viewer->mode = Viewer::REGULAR_MODE;
                    showGameOverDialog();
                }
            }
            break;

        // Arrow Keys
        case Qt::Key_Left:
            if (mode == AppMode::HIGHLIGHT_MODE){
                if (m_game->currentPlayer == 1)  m_game->moveHighlight(1,0);
                else m_game->moveHighlight(-1,0);
            }
            /*else if (mode == AppMode::SELECTED_MODE && !arrowKeysPressed.contains(key)){
                arrowKeysPressed += key;
            }*/
            break;

        case Qt::Key_Right:
            if (mode == AppMode::HIGHLIGHT_MODE){
                if (m_game->currentPlayer == 1) m_game->moveHighlight(-1,0);
                else m_game->moveHighlight(1,0);
            }
            /*else if (mode == AppMode::SELECTED_MODE && !arrowKeysPressed.contains(key)){
                arrowKeysPressed += key;
            }*/
            break;

        case Qt::Key_Up:
            if (mode == AppMode::HIGHLIGHT_MODE){
                if (m_game->currentPlayer == 1) m_game->moveHighlight(0,1);
                else m_game->moveHighlight(0,-1);
            }
            /*else if(mode == AppMode::SELECTED_MODE && !arrowKeysPressed.contains(key)){
                arrowKeysPressed += key;
            }*/
            break;

        case Qt::Key_Down:
            if (mode == AppMode::HIGHLIGHT_MODE){
                if (m_game->currentPlayer == 1) m_game->moveHighlight(0,-1);
                else m_game->moveHighlight(0,1);
            }
            /*else if (mode == AppMode::SELECTED_MODE && !arrowKeysPressed.contains(key)){
                arrowKeysPressed += key;
            }*/
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void AppWindow::keyReleaseEvent(QKeyEvent * event) {

    int key = event->key();

    // Process multi arrow keys
    if ((mode == AppMode::SELECTED_MODE) && arrowKeys.contains(key)){ 
        //&& arrowKeysPressed.contains(key) && arrowKeysPressed.size() <= 2){

        int dx = 0;
        int dz = 0;
        /*
        if (arrowKeysPressed.contains(Qt::Key_Up))    dz += -1;
        if (arrowKeysPressed.contains(Qt::Key_Down))  dz += 1;
        if (arrowKeysPressed.contains(Qt::Key_Left))  dx += -1;
        if (arrowKeysPressed.contains(Qt::Key_Right)) dz += 1;
        */
        switch (key){
            case Qt::Key_Up:
                dz += -1;
                break;
            case Qt::Key_Down:
                dz += 1;
                break;
            case Qt::Key_Left:
                dx += -1;
                break;
            case Qt::Key_Right:
                dx += 1;
                break;
        }

        // Flip if it's the other player
        if (m_game->currentPlayer == 1){
            dx *= -1;
            dz *= -1;
        }

        if(m_game->moveSelectedPiece(dx, dz)){
            mode = AppWindow::FIRE_LASER_MODE;
        } else {
            mode = AppMode::HIGHLIGHT_MODE;
            m_viewer->mode = Viewer::HIGHLIGHT_MODE;
        }

        return;
    } 

    switch (key) {
        case Qt::Key_C:
            if (mode == AppMode::SELECTED_MODE){
                if(m_game->rotateSelectedPiece(1)){
                    mode = AppWindow::FIRE_LASER_MODE;
                } else {
                    mode = AppMode::HIGHLIGHT_MODE;
                    m_viewer->mode = Viewer::HIGHLIGHT_MODE;
                }
            }
            break;
        case Qt::Key_W:
            if (mode == AppMode::SELECTED_MODE){
                if(m_game->rotateSelectedPiece(-1)){
                    mode = AppWindow::FIRE_LASER_MODE;
                } else {
                    mode = AppMode::HIGHLIGHT_MODE;
                    m_viewer->mode = Viewer::HIGHLIGHT_MODE;
                }
            }
            break;
        default:
            QWidget::keyReleaseEvent(event);
            break;
    }
}

void AppWindow::createMenu() {
    // Application menu
    m_menu_app = menuBar()->addMenu(tr("&Application"));

    QAction* quitAct = new QAction(tr("&Quit"), this);
    applicationActions.push_back(quitAct);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Exits the file"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));
    m_menu_app->addAction(quitAct);

    QAction* newGameAct = new QAction(tr("&New Game"), this);
    applicationActions.push_back(newGameAct);
    newGameAct->setShortcut(Qt::Key_N);
    newGameAct->setStatusTip(tr("Resets the game"));
    connect(newGameAct, SIGNAL(triggered()), this, SLOT(resetGame()));
    m_menu_app->addAction(newGameAct);

    // Mode menu
    m_menu_mode = menuBar()->addMenu(tr("&Mode"));

    QAction* perlinAct = new QAction(tr("&Show Perlin Textures"), this);
    modeActions.push_back(perlinAct);
    perlinAct->setShortcut(Qt::Key_P);
    perlinAct->setStatusTip(tr("Shows Perlin textures on game and tile pieces"));
    perlinAct->setCheckable(true);
    connect(perlinAct, SIGNAL(triggered()), this, SLOT(togglePerlinDisplay()));

    QAction* texturesAct = new QAction(tr("&Show Other Textures"), this);
    modeActions.push_back(texturesAct);
    texturesAct->setShortcut(Qt::Key_T);
    texturesAct->setStatusTip(tr("Shows other textures on game and tile pieces"));
    texturesAct->setCheckable(true);
    connect(texturesAct, SIGNAL(triggered()), this, SLOT(toggleOtherTextureDisplay()));

    QAction* bvhAct = new QAction(tr("Show Bounding Volumes"), this);
    modeActions.push_back(bvhAct);
    bvhAct->setShortcut(Qt::Key_B);
    bvhAct->setStatusTip(tr("Shows bounding volumes for currently highlighted piece"));
    bvhAct->setCheckable(true);
    connect(bvhAct, SIGNAL(triggered()), this, SLOT(toggleBVHDisplay()));

    QActionGroup* modeActionGroup = new QActionGroup(this);
    modeActionGroup->addAction(perlinAct);
    modeActionGroup->addAction(texturesAct);
    modeActionGroup->addAction(bvhAct);

    for (auto& action : modeActions) {
        m_menu_mode->addAction(action);
    }

    // Help Menu
    m_menu_help = menuBar()->addMenu(tr("&Help"));

    QAction* controlsAct = new QAction(tr("&Controls"), this);
    helpActions.push_back(controlsAct);
    controlsAct->setShortcut(Qt::Key_H);
    controlsAct->setStatusTip(tr("Shows game controls"));
    connect(controlsAct, SIGNAL(triggered()), this, SLOT(showHelpDialog()));
    m_menu_help->addAction(controlsAct);

    QAction* rulesAct = new QAction(tr("&Rules"), this);
    helpActions.push_back(rulesAct);
    rulesAct->setShortcut(Qt::Key_R);
    rulesAct->setStatusTip(tr("Shows game rules"));
    connect(rulesAct, SIGNAL(triggered()), this, SLOT(showRulesDialog()));
    m_menu_help->addAction(rulesAct);
}

void AppWindow::showGameOverDialog(){
    QString gameOverText;
    gameOverText.append("<b>GAME OVER</b><br><br>");
    if (m_game->winner == 1){
        gameOverText.append("<b>Player 1 (Grey) is the winner!</b>");
    } else {
        gameOverText.append("<b>Player 2 (Red) is the winner!</b>");
    }
    gameover_dialog->setText(gameOverText);
    gameover_dialog->show();
    m_soundfx->playSound("gameover");
    resetGame();
}

void AppWindow::updateTurnLabel(){
    if (m_game->currentPlayer == 1){
        turn_label->setText("<b>TURN: Player 1 <FONT COLOR='#2e2e2c'>(Grey)</b>");
    }
    else {
        turn_label->setText("<b>TURN: Player 2 <FONT COLOR='#ff0000'>(Red)</b>");
    }
    m_viewer->flipCameraPlayer(m_game->currentPlayer);
}

void AppWindow::updateModeLabel(){
    if (m_viewer->showPerlinTextures){
        mode_label->setText("<b>MODE: Perlin</b>");
    }
    else if (m_viewer->showOtherTextures){
        mode_label->setText("<b>MODE: Textures</b>");
    }
    else if (m_viewer->showBoundingVolumes){
        mode_label->setText("<b>MODE: BVH</b>");
    }
    else {
        mode_label->setText("<b>MODE: Normal</b>");
    }
}

// SLOTS
void AppWindow::toggleHighlight(){
    if (mode == AppMode::HIGHLIGHT_MODE){
        mode = AppMode::REGULAR_MODE;
        m_viewer->mode = Viewer::REGULAR_MODE;
        m_game->resetHighlight();
    }
    else {
        mode = AppMode::HIGHLIGHT_MODE;
        m_viewer->mode = Viewer::HIGHLIGHT_MODE;
    }
}

void AppWindow::resetGame(){
    mode = AppMode::HIGHLIGHT_MODE;
    m_viewer->reset();
    m_viewer->mode = Viewer::HIGHLIGHT_MODE;
    m_game->reset();
    updateModeLabel();
    updateTurnLabel();
}

void AppWindow::togglePerlinDisplay(){
    m_viewer->togglePerlinDisplay();
    updateModeLabel();
}

void AppWindow::toggleOtherTextureDisplay(){
    m_viewer->toggleOtherTextureDisplay();
    updateModeLabel();
}

void AppWindow::toggleBVHDisplay(){
    m_viewer->toggleBVHDisplay();
    updateModeLabel();
}

void AppWindow::showHelpDialog(){
    help_dialog->show();
}

void AppWindow::showRulesDialog(){
    rules_dialog->show();
}

void AppWindow::flipCamera(){
    m_viewer->flipCameraPlayer(m_game->currentPlayer);
}
