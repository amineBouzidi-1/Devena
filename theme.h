#ifndef THEME_H
#define THEME_H

#include <QString>

// ---------------------------------------------------------------------------
//  Charte graphique Devena (thème sombre bleu-canard) — NE PAS MODIFIER
//
//  Les widgets des fichiers .ui portent une propriété dynamique « role »
//  (Qt Designer : bouton « + » de l'éditeur de propriétés > Chaîne > role).
//  Exemple : un QFrame avec role = card prend automatiquement le style carte.
// ---------------------------------------------------------------------------
namespace Theme {

inline QString feuilleDeStyle()
{
    return QString::fromUtf8(R"(
        * {
            font-family: "Segoe UI", "Segoe UI Emoji", Arial;
            font-size: 13px;
            color: #e6f1f3;
        }

        QMainWindow, QWidget#central { background: #0a1c23; }
        QDialog, QMessageBox         { background: #10303a; }
        QToolTip { background: #10303a; color: #e6f1f3; border: 1px solid #1d4954; padding: 4px; }

        /* ---------- Barre latérale ---------- */
        QFrame[role="sidebar"] { background: #0e2730; border-right: 1px solid #1b3e48; }
        QLabel[role="logo"]    { font-size: 28px; font-weight: 700; color: #2ec4c0; }
        QLabel[role="slogan"]  { color: #8fb3bb; font-size: 12px; }

        QPushButton[role="nav"] {
            text-align: left; padding: 11px 14px; border: none; border-radius: 8px;
            font-size: 14px; color: #d5e6ea; background: transparent;
        }
        QPushButton[role="nav"]:hover   { background: #14363f; }
        QPushButton[role="nav"]:checked { background: #1a4a54; color: #ffffff; font-weight: 600; }

        /* ---------- Barre du haut ---------- */
        QFrame[role="topbar"] { background: #0a1c23; border-bottom: 1px solid #1b3e48; }
        QPushButton[role="iconBtn"] { background: transparent; border: none; font-size: 18px; }
        QPushButton[role="iconBtn"]:hover { background: #14363f; border-radius: 8px; }
        QLabel[role="avatar"] {
            background: #2ec4c0; color: #0a1c23; border-radius: 20px;
            font-weight: 700; font-size: 16px;
        }

        /* ---------- Cartes et textes ---------- */
        QFrame[role="card"] { background: #10303a; border: 1px solid #1d4954; border-radius: 10px; }
        QLabel[role="pageTitle"]    { font-size: 26px; font-weight: 700; }
        QLabel[role="sectionTitle"] { font-size: 15px; font-weight: 600; }
        QLabel[role="muted"]        { color: #8fb3bb; font-size: 12px; }
        QLabel[role="statValue"]    { font-size: 24px; font-weight: 700; }
        QLabel[role="statLabel"]    { color: #cfe1e5; font-size: 13px; }
        QLabel[role="statTrend"]    { color: #3ddc97; font-size: 12px; }
        QLabel[role="stars"]        { color: #f5c242; font-size: 17px; }

        /* ---------- Champs ---------- */
        QLineEdit, QComboBox, QDoubleSpinBox, QDateEdit, QPlainTextEdit {
            background: #123540; border: 1px solid #1f4c57; border-radius: 8px;
            padding: 7px 12px; color: #e6f1f3;
        }
        QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QPlainTextEdit:focus {
            border: 1px solid #2ec4c0;
        }
        QLineEdit[erreur="true"], QComboBox[erreur="true"], QDateEdit[erreur="true"], QPlainTextEdit[erreur="true"] { border: 1px solid #e2574c; }
        QLabel[role="erreur"] { color: #ff8a8a; font-size: 12px; }
        QLabel[role="champ"]  { color: #cfe1e5; font-weight: 600; }

        /* ---------- Boutons d'action ---------- */
        QPushButton[role="primary"] {
            background: #2ec4c0; color: #0a1c23; border: none; border-radius: 8px;
            padding: 8px 16px; font-weight: 600;
        }
        QPushButton[role="primary"]:hover    { background: #4fd6d2; }
        QPushButton[role="primary"]:disabled { background: #1d4954; color: #8fb3bb; }
        QPushButton[role="secondary"] {
            background: #1a4a54; color: #e6f1f3; border: 1px solid #22606c; border-radius: 8px;
            padding: 8px 14px;
        }
        QPushButton[role="secondary"]:hover { background: #22606c; }
        QPushButton[role="warning"] {
            background: #4d4220; color: #f5c242; border: 1px solid #64552a; border-radius: 8px;
            padding: 8px 14px; font-weight: 600;
        }
        QPushButton[role="warning"]:hover { background: #64552a; }
        QPushButton[role="danger"] {
            background: #6e2730; color: #ffb3b3; border: 1px solid #8a2f3a; border-radius: 8px;
            padding: 8px 14px; font-weight: 600;
        }
        QPushButton[role="danger"]:hover { background: #8a2f3a; }
        QPushButton[role="secondary"]:disabled, QPushButton[role="danger"]:disabled {
            background: #123540; color: #5f7c83; border: 1px solid #1d4954;
        }
        QComboBox::drop-down { border: none; width: 22px; }
        QComboBox QAbstractItemView {
            background: #10303a; border: 1px solid #1d4954;
            selection-background-color: #1a4a54; outline: none;
        }
        QComboBox[role="accent"] { background: #1a5a62; border: 1px solid #2ec4c0; }

        /* ---------- Onglets ---------- */
        QFrame[role="tabBar"] { background: #14414b; border-radius: 10px; }
        QPushButton[role="tab"] {
            background: #123540; border: none; border-radius: 8px;
            padding: 9px 22px; color: #d5e6ea;
        }
        QPushButton[role="tab"]:hover   { background: #1a4a54; }
        QPushButton[role="tab"]:checked { background: #7cc9c9; color: #0a1c23; font-weight: 600; }

        /* ---------- Tableaux ---------- */
        QTableView {
            background: #0f2b34; border: 1px solid #1d4954; border-radius: 8px;
            gridline-color: #1d4954; selection-background-color: #1a4a54;
        }
        QTableView::item { padding-left: 8px; border-bottom: 1px solid #1d4954; }
        QTableView::item:selected { background: #1a4a54; }
        QHeaderView::section {
            background: #12343e; color: #9fc3ca; border: none;
            border-right: 1px solid #1d4954; border-bottom: 1px solid #1d4954;
            padding: 9px 8px; font-weight: 600; font-size: 12px;
        }

        QPushButton[role="action"], QPushButton[role="actionDelete"], QPushButton[role="actionStar"] {
            border: none; border-radius: 5px; font-size: 13px;
        }
        QPushButton[role="action"]       { background: #1a4a54; }
        QPushButton[role="action"]:hover { background: #22606c; }
        QPushButton[role="actionDelete"]       { background: #6e2730; color: #ff8a8a; }
        QPushButton[role="actionDelete"]:hover { background: #8a2f3a; }
        QPushButton[role="actionStar"]         { background: #4d4220; color: #f5c242; }
        QPushButton[role="actionStar"]:hover   { background: #64552a; }

        QMessageBox QPushButton {
            background: #1a4a54; border: none; border-radius: 6px; padding: 6px 16px; min-width: 70px;
        }

        /* ---------- Divers ---------- */
        QProgressBar { background: #1d4954; border: none; border-radius: 4px; }
        QProgressBar::chunk { background: #2ec4c0; border-radius: 4px; }

        QScrollArea, QScrollArea > QWidget > QWidget { background: #0a1c23; border: none; }
        QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
        QScrollBar::handle:vertical { background: #1d4954; border-radius: 5px; min-height: 30px; }
        QScrollBar::add-line, QScrollBar::sub-line { height: 0; }
    )");
}

} // namespace Theme

#endif // THEME_H
