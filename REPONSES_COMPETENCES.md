# A7. Implémentation des interactions avec le SI

## C15 - Interfaces textuelles & Interfaces accessibles

**La solution présentée comporte diverses interfaces (graphiques et/ou textuelles) réfléchies afin d'optimiser l'expérience utilisateur**

La solution propose une distinction claire entre deux types d'interfaces adaptées à leur usage :
*   **Le Client Jeu (Graphique)** : Construit avec **SFML**, il offre une interface immersive pour les joueurs (menus, in-game overlay).
*   **Le Serveur (Administration)** : Dispose de l’**AdministratorPanel**, une interface graphique dédiée aux administrateurs. Conçue pour l'efficacité, elle sépare l'écran en deux zones (70% pour la liste des joueurs avec actions rapides, 30% pour les logs), permettant une modération fluide sans ligne de commande complexe.

**Les interfaces implémentées intègrent les exigences techniques et ergonomiques en matière d'accessibilité numérique**

L'accessibilité est prise en compte via plusieurs leviers techniques :
*   **Personnalisation des contrôles** : Le système `KeybindManager` permet le remappage complet des touches, adaptant le jeu aux périphériques spécifiques ou aux dispositions de clavier alternatives.
*   **Contraste Visuel** : L'interface d'administration utilise un thème à fort contraste (fond sombre et texte blanc), garantissant une lisibilité optimale conformes aux bonnes pratiques d'ergonomie.
*   **Jouabilité Manette** : Une prise en charge native des manettes de console est implémentée (`sf::Joystick` dans `client/RType.cpp`), incluant gestion des zones mortes et mapping intuitif, pour une expérience de jeu inclusive et ergonomique.

---

## C16 & C17 - Intégrité des données, stabilité & Normes

**La solution s'appuie sur un ensemble de normes et de composants tiers existants et reconnus comme robustes**

Le projet intègre des librairies industrielles éprouvées :
*   **SFML 2.6.1** : Standard pour le multimédia et le fenêtrage en C++.
*   **SQLite3** : Base de données légère, robuste et atomique pour la persistance.
*   **OpenSSL** : Norme de facto pour la sécurisation des échanges réseaux.
*   **PortAudio** : Librairie de référence pour l'audio à faible latence.

**Le candidat est capable de justifier de la pertinence de sa sélection de composants tiers au vue de l'environnement technologique**

Les choix technologiques sont cohérents avec les besoins du projet :
*   **SFML** : Préférée aux moteurs lourds (Unity/Unreal) pour sa légèreté et sa facilité d'intégration.
*   **SQLite** : Idéal pour un serveur de jeu nécessitant intégrité sans la lourdeur administrative d'un SGBD client-serveur.
*   **Zlib** : Justifiée pour l'optimisation de la bande passante via la compression des paquets réseaux.

---

# A8. Mise en place de fonctionnalités complexes avec traitement autonome

## C18 - Code opérationnel & Conventions de formatage et de nommage

**Le dossier du candidat contient un code opérationnel répondant aux exigences fonctionnelles**

Le code démontre une maturité technique :
*   Utilisation du standard **C++20** moderne.
*   Support **Multi-plateforme** (Windows/Linux) via directives préprocesseur.
*   Architecture découplée Client/Serveur avec bibliothèque réseau commune.

**Le code de la solution respecte les bonnes pratiques en matière de conventions de formatage et de nommage**

Les standards de développement sont respectés :
*   **Nommage** : CamelCase pour les fonctions et PascalCase pour les classes et fichiers.
*   **Structure** : Utilisation systématique de *header guards* et gestion propre des inclusions.
*   **Qualité** : Indentation constante et code propre, assurant une bonne maintenabilité.

---

## C19 - Composants tiers - implémentation & gestion des erreurs

**Le code de la solution implémente les appels nécessaires à l'intégration des composants tiers sélectionnés**

L'intégration des bibliothèques est conforme aux documentations officielles :
*   **SQLite** : Utilisation sécurisée des *prepared statements* pour prévenir les injections.
*   **SFML** : Implémentation correcte de la boucle d'événements (`pollEvent`).
*   **Réseau** : Gestion bas niveau des sockets compatible avec les standards OS.

**Le code de la solution traite de manière adéquate les cas d'erreur déclarés par ces composants tiers**

Une gestion d'erreur défensive est en place :
*   **Base de données** : Vérification des codes de retour SQL et levée d'exceptions explicites en cas d'échec.
*   **Réseau** : Gestion des erreurs non bloquantes (`EAGAIN`) pour assurer la stabilité du serveur sous charge.
*   **Ressources** : Logs d'erreurs clairs (`std::cerr`) en cas de fichiers manquants (ex: polices), empêchant les crashs silencieux.
