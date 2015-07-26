// CS 488 (Spring 2015)
// FINAL PROJECT - LASER CHESS!
// Adam Atkinson
// UserID:    aratkins
// StudentID: 20416358

#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QMessageBox>
#include <QSet>
#include <vector>
#include "Game.hpp"
#include "Viewer.hpp"
#include "Sound.hpp"

class AppWindow : public QMainWindow
{
    Q_OBJECT

public:
    AppWindow();
    ~AppWindow();
    enum AppMode {
        REGULAR_MODE, 
        HIGHLIGHT_MODE, 
        SELECTED_MODE, 
        FIRE_LASER_MODE,
        SHOW_LASER_MODE,
        GAME_OVER
    };

protected:
    void keyPressEvent(QKeyEvent *event);
 	void keyReleaseEvent(QKeyEvent * event);

private slots:
    void toggleHighlight();
    void resetGame();
    void togglePerlinDisplay();
    void toggleOtherTextureDisplay();
    void toggleBVHDisplay();
    void showHelpDialog();
    void showRulesDialog();
    void flipCamera();

private:
    void createMenu();
    void showGameOverDialog();
    void updateTurnLabel();
    void updateModeLabel();
    // Each menu itself
    QMenu* m_menu_app;
    std::vector<QAction*> applicationActions;

    QMenu* m_menu_mode;
    std::vector<QAction*> modeActions;

    QMenu* m_menu_help;
    std::vector<QAction*> helpActions;

    QMessageBox* help_dialog;
    QMessageBox* rules_dialog;
    QMessageBox* gameover_dialog;

    Viewer* m_viewer;
    Game* m_game;
    SoundFX* m_soundfx;
    QLabel* status_label;
    QLabel* turn_label;
    QLabel* mode_label;
    AppMode mode;
    QSet<int> arrowKeys;
};

#endif