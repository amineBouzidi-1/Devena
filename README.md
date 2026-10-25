# Devena — Module Évaluation (Qt / C++ / Oracle)

Module réalisé d'après l'**Atelier 3 : CRUD (architecture Modèle-Vue)**.

## Architecture Modèle-Vue
| Couche | Fichiers | Rôle |
|---|---|---|
| Connexion | `connection.h / .cpp` | Singleton : une seule connexion à la BD (QODBC → Oracle) |
| **Modèle** | `evaluation.h / .cpp` | Classe purement C++ : **toutes** les requêtes SQL (préparées, `bindValue`) |
| Modèle (affichage) | `evaluationmodel.h / .cpp` | `QSqlQueryModel` mis en forme (étoiles, dates, alertes) |
| **Vue** | `mainwindow.ui` (**un seul .ui** pour tous les modules) | stackedWidget principal : une page par module |
| **Vue** | `mainwindow.h / .cpp` | Code découpé en zones `// === MODULE XXX ===` : le module Évaluation est dans sa zone, aucune requête SQL |
| Vue | `theme.h`, `chartview.h` | Charte graphique, widget promu pour les graphiques |
| Secours | `basedemo.h / .cpp` | Base SQLite en mémoire si Oracle est indisponible |

## Entité gérée : EVALUATION
`EVALUATION (ID_EVALUATION, NOTE, COMMENTAIRE, DATE_EVAL, #ID_EQUIPE, #ID_EMPLOYE)`

| CRUD | Méthode (classe Evaluation) | Requête |
|---|---|---|
| Create | `bool ajouter()` | `INSERT … VALUES (SEQ_EVALUATION.NEXTVAL, :note, …)` |
| Read | `QSqlQueryModel* afficher(recherche, tri, période)` | `SELECT … JOIN EQUIPE … JOIN EMPLOYE …` |
| Update | `bool modifier()` | `UPDATE EVALUATION SET … WHERE ID_EVALUATION = :id` |
| Delete | `bool supprimer(int id)` | `DELETE FROM EVALUATION WHERE ID_EVALUATION = :id` |

Après chaque ajout / modification / suppression réussi, la vue appelle `actualiserEvaluations()`
qui ré-exécute `afficher()` (actualisation de l'affichage, cf. atelier).

## Interface (sans fenêtres de dialogue, avec un QStackedWidget)
1. **Liste** : tableau (QTableView), recherche, tri, filtre par période, Modifier / Supprimer / Fiche PDF
2. **Ajouter / Modifier** : formulaire intégré (listes déroulantes pour l'équipe et l'évaluateur)
3. **Classement & alertes** : podium des équipes + évaluations avec note < 3

## Contrôles de saisie (C++) et contraintes (BD)
- Équipe et évaluateur obligatoires · note entre 0 et 5 · date valide et pas dans le futur
- Commentaire de 3 à 255 caractères avec au moins une lettre
- Règle métier : un employé ne peut pas évaluer deux fois la même équipe le même jour
- Côté Oracle : clés primaires / étrangères, `CHECK (NOTE BETWEEN 0 AND 5)`, `UNIQUE`…

## Fonctionnalités avancées
Recherche multicritère + tri + période · Export PDF (liste, fiche, alertes) ·
Statistiques (cartes, moyenne par équipe, répartition) · Classement des équipes · Alertes

## Mise en place
1. **SQL Developer** : partie A de `database/devena.sql` en SYSTEM (création de `amine`),
   puis tout le fichier connecté en `amine` (F5 / « Exécuter le script »).
2. **ODBC 64 bits** : source `Source_Projet2A` (pilote Oracle, TNS `XE` ou `localhost:1521/XE`,
   utilisateur `amine`) → *Test Connection*.
3. Qt Creator → `Devena.pro` → kit MinGW 64-bit → **Ctrl + R** → « Connexion réussie ».

Si la connexion échoue, l'appli démarre sur la **base de démonstration** (indicateur jaune
en haut) : tout fonctionne mais rien n'est enregistré dans Oracle.

## Intégration (guide d'intégration collaborative)
- Un seul `mainwindow.ui` : chaque module a sa page dans `stackedPages`.
- Navigation (barre latérale) dans la zone « SLOTS GÉNÉRAUX » de `mainwindow.cpp`.
- Chaque étudiant ajoute **sa classe métier** (`equipe.h/.cpp`, `projet.h/.cpp`…) et écrit
  **uniquement dans sa zone** `// === MODULE XXX ===` de `mainwindow.h` et `mainwindow.cpp`.
- Même configuration de connexion pour toute l'équipe (`connection.cpp`) : même source ODBC,
  même utilisateur, même mot de passe.

## GitHub (jamais d'« upload »)
```
git pull origin main          # toujours avant de commencer
git add evaluation.h evaluation.cpp mainwindow.h mainwindow.cpp mainwindow.ui
git commit -m "Module Evaluation : CRUD + métiers"
git pull origin main          # récupérer le travail des autres avant de pousser
git push origin main
```
