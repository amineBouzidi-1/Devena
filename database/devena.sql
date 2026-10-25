-- ===========================================================================
--  DEVENA - Base de donnees commune (Oracle XE 11g)
--  D'apres le modele Entite/Association de l'equipe.
--
--  PARTIE A (une seule fois) : connecte en SYSTEM, creer l'utilisateur :
--     CREATE USER amine IDENTIFIED BY amine123;
--     GRANT CONNECT, RESOURCE, CREATE VIEW TO amine;
--     GRANT UNLIMITED TABLESPACE TO amine;
--
--  PARTIE B : connecte en AMINE, executer TOUT ce fichier avec
--  « Executer le script » (F5 ou Fn+F5, 2e icone a droite du triangle vert).
--  Les erreurs ORA-00942 / ORA-02289 sur les DROP au debut sont NORMALES
--  (elles signifient seulement que l'objet n'existait pas encore).
-- ===========================================================================

-- ---------------------------------------------------------------------------
-- 0) Nettoyage (ordre inverse des dependances)
-- ---------------------------------------------------------------------------
DROP TABLE EVALUATION  CASCADE CONSTRAINTS;
DROP TABLE PROJET      CASCADE CONSTRAINTS;
DROP TABLE PARTICIPANT CASCADE CONSTRAINTS;
DROP TABLE EMPLOYE     CASCADE CONSTRAINTS;
DROP TABLE EQUIPE      CASCADE CONSTRAINTS;

DROP SEQUENCE SEQ_EVALUATION;
DROP SEQUENCE SEQ_PROJET;
DROP SEQUENCE SEQ_PARTICIPANT;
DROP SEQUENCE SEQ_EMPLOYE;
DROP SEQUENCE SEQ_EQUIPE;

-- ---------------------------------------------------------------------------
-- 1) EQUIPE
-- ---------------------------------------------------------------------------
CREATE TABLE EQUIPE (
    ID_EQUIPE     NUMBER         CONSTRAINT PK_EQUIPE PRIMARY KEY,
    NOM_EQUIPE    VARCHAR2(100)  NOT NULL CONSTRAINT UQ_EQUIPE_NOM UNIQUE,
    NB_MEMBRES    NUMBER(1)      DEFAULT 0 NOT NULL,
    CAPACITE_MAX  NUMBER(1)      DEFAULT 6 NOT NULL,
    SECTEUR       VARCHAR2(50),
    AGE_MOYEN     NUMBER(3),
    COMPETENCES   VARCHAR2(255),
    CONSTRAINT CK_EQUIPE_CAPACITE CHECK (CAPACITE_MAX BETWEEN 1 AND 6),
    CONSTRAINT CK_EQUIPE_MEMBRES  CHECK (NB_MEMBRES BETWEEN 0 AND CAPACITE_MAX),
    CONSTRAINT CK_EQUIPE_AGE      CHECK (AGE_MOYEN IS NULL OR AGE_MOYEN BETWEEN 15 AND 99)
);

-- ---------------------------------------------------------------------------
-- 2) PARTICIPANT   (Participant 1,1 --appartenir-- 1,6 Equipe)
-- ---------------------------------------------------------------------------
CREATE TABLE PARTICIPANT (
    ID_PARTICIPANT  NUMBER         CONSTRAINT PK_PARTICIPANT PRIMARY KEY,
    NOM             VARCHAR2(50)   NOT NULL,
    PRENOM          VARCHAR2(50)   NOT NULL,
    EMAIL           VARCHAR2(100)  NOT NULL CONSTRAINT UQ_PARTICIPANT_EMAIL UNIQUE,
    COMPETENCES     VARCHAR2(255),
    ID_EQUIPE       NUMBER         NOT NULL,
    CONSTRAINT FK_PARTICIPANT_EQUIPE FOREIGN KEY (ID_EQUIPE) REFERENCES EQUIPE (ID_EQUIPE),
    CONSTRAINT CK_PARTICIPANT_EMAIL  CHECK (EMAIL LIKE '%_@_%._%')
);

-- ---------------------------------------------------------------------------
-- 3) PROJET   (Equipe 1,1 --developper-- 1,1 Projet)
-- ---------------------------------------------------------------------------
CREATE TABLE PROJET (
    ID_PROJET     NUMBER          CONSTRAINT PK_PROJET PRIMARY KEY,
    TITRE         VARCHAR2(100)   NOT NULL,
    DESCRIPTION   VARCHAR2(500),
    THEMATIQUE    VARCHAR2(100),
    NOTE_FINALE   NUMBER(3,1),
    ID_EQUIPE     NUMBER          NOT NULL,
    CONSTRAINT FK_PROJET_EQUIPE    FOREIGN KEY (ID_EQUIPE) REFERENCES EQUIPE (ID_EQUIPE),
    CONSTRAINT UQ_PROJET_EQUIPE    UNIQUE (ID_EQUIPE),
    CONSTRAINT CK_PROJET_NOTE      CHECK (NOTE_FINALE IS NULL OR NOTE_FINALE BETWEEN 0 AND 5)
);

-- ---------------------------------------------------------------------------
-- 4) EMPLOYE
-- ---------------------------------------------------------------------------
CREATE TABLE EMPLOYE (
    ID_EMPLOYE   NUMBER         CONSTRAINT PK_EMPLOYE PRIMARY KEY,
    NOM          VARCHAR2(50)   NOT NULL,
    PRENOM       VARCHAR2(50)   NOT NULL,
    SPECIALITE   VARCHAR2(100)
);

-- ---------------------------------------------------------------------------
-- 5) EVALUATION   (Equipe 1,1 --evaluer-- 1,n Evaluation)
--                 (Employe 1,1 --gerer--  1,n Evaluation)
-- ---------------------------------------------------------------------------
CREATE TABLE EVALUATION (
    ID_EVALUATION  NUMBER         CONSTRAINT PK_EVALUATION PRIMARY KEY,
    NOTE           NUMBER(2,1)    NOT NULL,
    COMMENTAIRE    VARCHAR2(255),
    DATE_EVAL      DATE           DEFAULT SYSDATE NOT NULL,
    ID_EQUIPE      NUMBER         NOT NULL,
    ID_EMPLOYE     NUMBER         NOT NULL,
    CONSTRAINT FK_EVALUATION_EQUIPE  FOREIGN KEY (ID_EQUIPE)  REFERENCES EQUIPE (ID_EQUIPE) ON DELETE CASCADE,
    CONSTRAINT FK_EVALUATION_EMPLOYE FOREIGN KEY (ID_EMPLOYE) REFERENCES EMPLOYE (ID_EMPLOYE),
    CONSTRAINT CK_EVALUATION_NOTE    CHECK (NOTE BETWEEN 0 AND 5)
);

-- ---------------------------------------------------------------------------
-- 6) Sequences (identifiants automatiques)
-- ---------------------------------------------------------------------------
CREATE SEQUENCE SEQ_EQUIPE      START WITH 1 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE SEQ_PARTICIPANT START WITH 1 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE SEQ_PROJET      START WITH 1 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE SEQ_EMPLOYE     START WITH 1 INCREMENT BY 1 NOCACHE;
CREATE SEQUENCE SEQ_EVALUATION  START WITH 1 INCREMENT BY 1 NOCACHE;

-- ---------------------------------------------------------------------------
-- 7) Donnees de test
-- ---------------------------------------------------------------------------
INSERT INTO EQUIPE VALUES (SEQ_EQUIPE.NEXTVAL, 'Equipe Innovatrice', 4, 6, 'Web',        22, 'Qt, C++, SQL');
INSERT INTO EQUIPE VALUES (SEQ_EQUIPE.NEXTVAL, 'Equipe Digital',     3, 6, 'Education',  21, 'UI/UX, Java');
INSERT INTO EQUIPE VALUES (SEQ_EQUIPE.NEXTVAL, 'Logic Devs',         5, 6, 'IA',         23, 'Python, IA, Data');

INSERT INTO PARTICIPANT VALUES (SEQ_PARTICIPANT.NEXTVAL, 'Ben Ali',  'Amine',   'amine.benali@esprit.tn',   'C++, Qt',        1);
INSERT INTO PARTICIPANT VALUES (SEQ_PARTICIPANT.NEXTVAL, 'Trabelsi', 'Sarra',   'sarra.trabelsi@esprit.tn', 'Gestion projet', 3);
INSERT INTO PARTICIPANT VALUES (SEQ_PARTICIPANT.NEXTVAL, 'Gharbi',   'Youssef', 'youssef.gharbi@esprit.tn', 'Design UI',      2);
INSERT INTO PARTICIPANT VALUES (SEQ_PARTICIPANT.NEXTVAL, 'Hammami',  'Nour',    'nour.hammami@esprit.tn',   'Python',         3);

INSERT INTO PROJET VALUES (SEQ_PROJET.NEXTVAL, 'Plateforme Devena', 'Plateforme de gestion de challenges', 'Web Innovation', 4.4, 1);
INSERT INTO PROJET VALUES (SEQ_PROJET.NEXTVAL, 'Campus Connect',    'Application du campus connecte',      'Smart Campus',   3.9, 2);
INSERT INTO PROJET VALUES (SEQ_PROJET.NEXTVAL, 'VisionAI',          'Reconnaissance d''images par IA',     'AI Challenge',   4.8, 3);

INSERT INTO EMPLOYE VALUES (SEQ_EMPLOYE.NEXTVAL, 'Mansour', 'Leila', 'Developpement web');
INSERT INTO EMPLOYE VALUES (SEQ_EMPLOYE.NEXTVAL, 'Jaziri',  'Karim', 'Intelligence artificielle');

INSERT INTO EVALUATION VALUES (SEQ_EVALUATION.NEXTVAL, 4.6, 'Tres bonne collaboration', SYSDATE,      1, 1);
INSERT INTO EVALUATION VALUES (SEQ_EVALUATION.NEXTVAL, 4.0, 'Bon travail',              SYSDATE - 2,  2, 1);
INSERT INTO EVALUATION VALUES (SEQ_EVALUATION.NEXTVAL, 4.8, 'Excellente equipe',        SYSDATE - 1,  3, 2);
INSERT INTO EVALUATION VALUES (SEQ_EVALUATION.NEXTVAL, 2.7, 'Retard sur le planning',   SYSDATE - 30, 2, 2);

COMMIT;

-- ---------------------------------------------------------------------------
-- 8) Verification
-- ---------------------------------------------------------------------------
SELECT ev.ID_EVALUATION, eq.NOM_EQUIPE, ev.NOTE, ev.COMMENTAIRE,
       em.NOM || ' ' || em.PRENOM AS EVALUATEUR, ev.DATE_EVAL
FROM   EVALUATION ev
JOIN   EQUIPE  eq ON eq.ID_EQUIPE  = ev.ID_EQUIPE
JOIN   EMPLOYE em ON em.ID_EMPLOYE = ev.ID_EMPLOYE
ORDER BY ev.ID_EVALUATION;
