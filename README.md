# Mr Robot — portage Gamebuino AKA

Jeu MicroPython d'esquive de fusées. Il nécessite le module de compatibilité upygame, encore en cours de développement : **non testé sur la console réelle**.

## Jeu d'origine

- **Jeu :** Mr Robot
- **Auteur :** Hannu Viitala (haviital)
- **Plateforme d'origine :** Pokitto (MicroPython)
- **Source :** <https://github.com/haviital/MrRobot>
- **Licence :** MIT (voir [LICENSE](LICENSE), qui reprend le texte de la licence d'origine)

Tout le mérite du jeu (conception, graphismes, musique, code) revient à son auteur. Ce dépôt n'est qu'un portage.

## Ce portage

Portage sur la console **Gamebuino AKA** (ESP32-S3, écran 320×240) réalisé par Jicehel avec l'outil AKA Port Studio : firmware ESP-IDF 5.5.1 (`idf.py set-target esp32s3 && idf.py build`). Les fichiers du dossier `sdcard_files/` se copient à la racine de la carte SD de la console.

## Licence

Ce portage est distribué sous la même licence que le jeu d'origine : **MIT** (copyright du jeu : © 2020 Hannu Viitala ; copyright du portage : © 2026 AKA Port Studio). Voir aussi [LICENSE.micropython](LICENSE.micropython). Le texte complet, avec le copyright d'origine, est dans le fichier [LICENSE](LICENSE).
